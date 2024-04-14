/*
 * Copyright (c) 2000, 2001, 2002, 2003, 2004, 2005, 2008, 2009
 *	The President and Fellows of Harvard College.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE UNIVERSITY AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE UNIVERSITY OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <types.h>
#include <kern/errno.h>
#include <kern/syscall.h>
#include <lib.h>
#include <mips/trapframe.h>
#include <thread.h>
#include <current.h>
#include <syscall.h>

#include <limits.h>
#include <uio.h>
#include <vfs.h>
#include <copyinout.h>
#include <synch.h>
#include <vnode.h>
#include <kern/unistd.h>
#include <proc.h>

/* Copied from `kern/include/limits.h` */
#include <kern/limits.h>



/* Copied from `kern/include/types.h` */

typedef struct __userptr *userptr_t;
typedef const struct __userptr *const_userptr_t;
typedef __i32 int32_t;
typedef _Bool bool;



/* Based on parts of `kern/include/types.h` */

#ifndef true
#define true 1
#endif /* true */

#ifndef false
#define false 0
#endif /* false */



/* Based on parts of `kern/include/limits.h` */

#ifndef NAME_MAX
#define NAME_MAX __NAME_MAX
#endif /* NAME_MAX */

#ifndef PATH_MAX
#define PATH_MAX __PATH_MAX
#endif /* PATH_MAX */



/* Based on parts of `kern/include/limits.h` */

#ifdef SYS__exit
#ifdef SYS_close
#if SYS__exit == SYS_close

#undef SYS__exit
#define SYS__exit 3
#undef SYS_close
#define SYS_close 49

#endif /* SYS__exit == SYS_close */
#endif /* SYS_close */
#endif /* SYS__exit */




/* One plus the maximum of STDIN_FILENO, STDOUT_FILENO, and STDERR_FILENO */
#define __FD_MIN 3

/* Equals 127 so that, if need be, we can cast a given fd to a `char`. */
#define __FD_MAX 127

#define FD_MIN __FD_MIN
#define FD_MAX __FD_MAX

static int sys___getcwd(userptr_t userbuf, size_t userbuflen, int32_t *retval);
static int sys_open(const_userptr_t usrfilename, int flags, mode_t mode, int32_t *retval);
static int sys_close(int fd);
static int sys_write(int fd, const_userptr_t userbuf, size_t userbuflen, int32_t *retval);
static int sys_write_out(const_userptr_t userbuf, size_t userbuflen, int32_t *retval);
static int sys_write_err(const_userptr_t userbuf, size_t userbuflen, int32_t *retval);
static int sys_write_outerr(const_userptr_t userbuf, size_t userbuflen, int32_t *retval);
static int sys_write_fdnode(int fd, const_userptr_t userbuf, size_t userbuflen, int32_t *retval);
static int sys_read(int fd, userptr_t userbuf, size_t userbuflen, int32_t *retval);
static int sys_read_in(userptr_t userbuf, size_t userbuflen, int32_t *retval);
static int sys_read_fdnode(int fd, userptr_t userbuf, size_t userbuflen, int32_t *retval);
static int sys_getpid(int32_t *retval);
__DEAD static void sys__exit(int exitcode);




struct fdnode {
	int fd;
	struct vnode *v;
	struct fdnode *next;
};

static struct fdnode *first_fdnode = NULL;
static struct lock *fdnode_lock = NULL;

static void fdnode_ensure_lock_not_null()
{
	// Implementation based on `kprintf_bootstrap()`

	if (fdnode_lock != NULL) {
		return;
	}

	// This line comes after the `if (fdnode_lock != NULL)`
	// block above so that this line is only executed once,
	// that one time being when `fdnode_lock` needs to be
	// created.
	KASSERT(FD_MIN <= FD_MAX);

	fdnode_lock = lock_create("fdnode_lock");
	if (fdnode_lock == NULL) {
		panic("Could not create fdnode_lock\n");
	}
}

static
bool
fd_is_being_used(int fd_in_question)
{
	if (fd_in_question < FD_MIN || fd_in_question > FD_MAX) {
		return false;
	}

	KASSERT(lock_do_i_hold(fdnode_lock));
	for (struct fdnode *fdn = first_fdnode; fdn != NULL; fdn = fdn->next) {
		if (fdn->fd == fd_in_question) {
			return true;
		}
	}
	return false;
}

// This function returns 0 for success or an error number for failure.
static
int
fd_lookup(struct vnode **vn, int fd_in_question)
{
	if (fd_in_question < FD_MIN || fd_in_question > FD_MAX) {
		return EBADF;
	}

	KASSERT(lock_do_i_hold(fdnode_lock));
	KASSERT(vn != NULL);
	for (struct fdnode *fdn = first_fdnode; fdn != NULL; fdn = fdn->next) {
		if (fdn->fd == fd_in_question) {
			*vn = fdn->v;
			return 0;
		}
	}
	return EBADF;
}

// This function is only to be called from `sys_open()`.
// Do not use `vfs_close(v)` in this function.
// This function returns 0 for success or an error number for failure.
static
int
fd_allocate(struct vnode *v, int *fd)
{
	fdnode_ensure_lock_not_null();

	struct fdnode *new_first_fdnode;

	new_first_fdnode = kmalloc(sizeof(*new_first_fdnode));
	if (new_first_fdnode == NULL) {
		return ENOMEM;
	}

	new_first_fdnode->fd = 0;
	new_first_fdnode->v = NULL;
	new_first_fdnode->next = NULL;

	int available_fd = FD_MIN;
	bool fd_is_available = false;

	lock_acquire(fdnode_lock);
	for (int i = FD_MIN; i <= FD_MAX; i++) {
		if (!fd_is_being_used(i)) {
			available_fd = i;
			fd_is_available = true;
			break;
		}
	}

	if (!fd_is_available) {
		lock_release(fdnode_lock);
		kfree(new_first_fdnode);
		return ENFILE;
	}

	new_first_fdnode->fd = available_fd;
	new_first_fdnode->v = v;
	new_first_fdnode->next = first_fdnode;

	first_fdnode = new_first_fdnode;
	lock_release(fdnode_lock);

	*fd = available_fd;
	return 0;
}

// This function is only to be called from `sys_close()`.
// This function returns 0 for success or an error number for failure.
static
int
fd_deallocate(struct vnode **v, int fd)
{
	if (fd < FD_MIN || fd > FD_MAX) {
		return EBADF;
	}

	fdnode_ensure_lock_not_null();

	struct fdnode *fdnode_removed = NULL;
	struct vnode *vnode_removed = NULL;
	bool fd_found = false;

	lock_acquire(fdnode_lock);
	if (first_fdnode == NULL) {
		// No open files
	}
	else if (first_fdnode->fd == fd) {
		fdnode_removed = first_fdnode;
		vnode_removed = fdnode_removed->v;
		first_fdnode = fdnode_removed->next;
		fd_found = true;
	}
	else {
		for (struct fdnode *fdn = first_fdnode; fdn->next != NULL; fdn = fdn->next) {
			if (fdn->next->fd == fd) {
				fdnode_removed = fdn->next;
				vnode_removed = fdnode_removed->v;
				fdn->next = fdnode_removed->next;
				fd_found = true;
				break;
			}
		}
	}
	lock_release(fdnode_lock);

	if (!fd_found) {
		return EBADF;
	}

	kfree(fdnode_removed);

	*v = vnode_removed;

	return 0;
}

// This function returns 0 for success or an error number for failure.
// This function is only to be called from `sys_write_outerr()` and `sys_read_in()`.
/*
static
int
find_console_vnode(struct vnode **vn)
{
	char path[5];
	path[0] = 'c';
	path[1] = 'o';
	path[2] = 'n';
	path[3] = ':';
	path[4] = 0;
	return vfs_lookup(path, vn);
}
*/




/*
 * System call dispatcher.
 *
 * A pointer to the trapframe created during exception entry (in
 * exception-*.S) is passed in.
 *
 * The calling conventions for syscalls are as follows: Like ordinary
 * function calls, the first 4 32-bit arguments are passed in the 4
 * argument registers a0-a3. 64-bit arguments are passed in *aligned*
 * pairs of registers, that is, either a0/a1 or a2/a3. This means that
 * if the first argument is 32-bit and the second is 64-bit, a1 is
 * unused.
 *
 * This much is the same as the calling conventions for ordinary
 * function calls. In addition, the system call number is passed in
 * the v0 register.
 *
 * On successful return, the return value is passed back in the v0
 * register, or v0 and v1 if 64-bit. This is also like an ordinary
 * function call, and additionally the a3 register is also set to 0 to
 * indicate success.
 *
 * On an error return, the error code is passed back in the v0
 * register, and the a3 register is set to 1 to indicate failure.
 * (Userlevel code takes care of storing the error code in errno and
 * returning the value -1 from the actual userlevel syscall function.
 * See src/user/lib/libc/arch/mips/syscalls-mips.S and related files.)
 *
 * Upon syscall return the program counter stored in the trapframe
 * must be incremented by one instruction; otherwise the exception
 * return code will restart the "syscall" instruction and the system
 * call will repeat forever.
 *
 * If you run out of registers (which happens quickly with 64-bit
 * values) further arguments must be fetched from the user-level
 * stack, starting at sp+16 to skip over the slots for the
 * registerized values, with copyin().
 */
void
syscall(struct trapframe *tf)
{
	int callno;
	int32_t retval;
	int err;

	KASSERT(curthread != NULL);
	KASSERT(curthread->t_curspl == 0);
	KASSERT(curthread->t_iplhigh_count == 0);

	callno = tf->tf_v0;

	/*
	 * Initialize retval to 0. Many of the system calls don't
	 * really return a value, just 0 for success and -1 on
	 * error. Since retval is the value returned on success,
	 * initialize it to 0 by default; thus it's not necessary to
	 * deal with it except for calls that return other values,
	 * like write.
	 */

	retval = 0;

	switch (callno) {
	    case SYS_reboot:
		err = sys_reboot(tf->tf_a0);
		break;

	    case SYS___time:
		err = sys___time((userptr_t)tf->tf_a0,
				 (userptr_t)tf->tf_a1);
		break;

	    /* Add stuff here */
	    case SYS___getcwd:
		err = sys___getcwd((userptr_t)tf->tf_a0, (size_t)tf->tf_a1, &retval);
		break;
	    case SYS_open:
		err = sys_open((const_userptr_t)tf->tf_a0, (int)tf->tf_a1, (mode_t)tf->tf_a2, &retval);
		break;
	    case SYS_close:
		err = sys_close((int)tf->tf_a0);
		break;
		case SYS_write:
		err = sys_write((int)tf->tf_a0, (const_userptr_t)tf->tf_a1, (size_t)tf->tf_a2, &retval);
		break;
		case SYS_read:
		err = sys_read((int)tf->tf_a0, (userptr_t)tf->tf_a1, (size_t)tf->tf_a2, &retval);
		break;
		case SYS_getpid:
		err = sys_getpid(&retval);
		break;
		case SYS__exit:
		sys__exit((int)tf->tf_a0);
		break;

	    default:
		kprintf("Unknown syscall %d\n", callno);
		err = ENOSYS;
		break;
	}


	if (err) {
		/*
		 * Return the error code. This gets converted at
		 * userlevel to a return value of -1 and the error
		 * code in errno.
		 */
		tf->tf_v0 = err;
		tf->tf_a3 = 1;      /* signal an error */
	}
	else {
		/* Success. */
		tf->tf_v0 = retval;
		tf->tf_a3 = 0;      /* signal no error */
	}

	/*
	 * Now, advance the program counter, to avoid restarting
	 * the syscall over and over again.
	 */

	tf->tf_epc += 4;

	/* Make sure the syscall code didn't forget to lower spl */
	KASSERT(curthread->t_curspl == 0);
	/* ...or leak any spinlocks */
	KASSERT(curthread->t_iplhigh_count == 0);
}

/*
 * Enter user mode for a newly forked process.
 *
 * This function is provided as a reminder. You need to write
 * both it and the code that calls it.
 *
 * Thus, you can trash it and do things another way if you prefer.
 */
void
enter_forked_process(struct trapframe *tf)
{
	(void)tf;
}

// This function is only to be called from `syscall()`.
// `userbuf` represents a `char *` passed from user space.
// This function returns 0 for success or an error number for failure.
static
int
sys___getcwd(userptr_t userbuf, size_t userbuflen, int32_t *retval)
{
	// Implementation based on `cmd_pwd()`

	char kernbuf[PATH_MAX + 1];
	int result;
	struct iovec iov;
	struct uio ku;

	/* null terminate */
	kernbuf[sizeof(kernbuf) - 1] = 0;

	uio_kinit(&iov, &ku, kernbuf, sizeof(kernbuf) - 1, 0, UIO_READ);
	result = vfs_getcwd(&ku);
	if (result) {
		return result;
	}

	/* null terminate */
	kernbuf[sizeof(kernbuf) - 1] = 0;

	/* null terminate */
	if (sizeof(kernbuf) - 1 >= ku.uio_resid) {
		kernbuf[sizeof(kernbuf) - 1 - ku.uio_resid] = 0;
	}

	size_t kernbuflen = strlen(kernbuf);

	size_t numbytescopied;
	if (kernbuflen < userbuflen) {
		numbytescopied = kernbuflen;
	} else {
		numbytescopied = userbuflen;
	}

	result = copyout(kernbuf, userbuf, numbytescopied);
	if (result) {
		return result;
	}

	*retval = (int32_t)numbytescopied;

	return 0;
}

// This function is only to be called from `syscall()`.
// `usrfilename` represents a `const char *` passed from user space.
// This function returns 0 for success or an error number for failure.
//
// According to userland/include/unistd.h, we can technically ignore the third argument.
static
int
sys_open(const_userptr_t usrfilename, int flags, mode_t mode, int32_t *retval)
{
	// Implementation based on part of `runprogram()`

	char kernfilename[NAME_MAX + 1];
	int result;
	size_t got;
	struct vnode *v;

	/* null terminate */
	kernfilename[sizeof(kernfilename) - 1] = 0;

	result = copyinstr(usrfilename, kernfilename, sizeof(kernfilename), &got);
	if (result) {
		return result;
	}

	/* null terminate */
	kernfilename[sizeof(kernfilename) - 1] = 0;

	if (got < sizeof(kernfilename)) {
		kernfilename[got] = 0;
	}

	/* Open the file. */
	result = vfs_open(kernfilename, flags, mode, &v);
	if (result) {
		return result;
	}

	int fd;
	result = fd_allocate(v, &fd);
	if (result) {
		vfs_close(v);
		return result;
	}
	
	KASSERT(fd >= FD_MIN);
	KASSERT(fd <= FD_MAX);

	*retval = (int32_t)fd;

	return 0;
}

// This function is only to be called from `syscall()`.
// This function returns 0 for success or an error number for failure.
static
int
sys_close(int fd)
{
	// Implementation based on part of `runprogram()`

	if (fd < FD_MIN || fd > FD_MAX) {
		return EBADF;
	}

	struct vnode *v; // the `vnode` previously linked to `fd` by `fd_allocate()`
	int result;

	result = fd_deallocate(&v, fd);
	if (result) {
		return result;
	}

	/* Done with the file now. */
	vfs_close(v);

	return 0;
}

// This function is only to be called from `syscall()`.
// `userbuf` represents a `const void *` passed from user space.
// This function returns 0 for success or an error number for failure.
static
int
sys_write(int fd, const_userptr_t userbuf, size_t userbuflen, int32_t *retval)
{
	if (fd == STDIN_FILENO) {
		return EBADF;
	}
	if (fd == STDOUT_FILENO) {
		return sys_write_out(userbuf, userbuflen, retval);
	}
	if (fd == STDERR_FILENO) {
		return sys_write_err(userbuf, userbuflen, retval);
	}
	if (fd < FD_MIN || fd > FD_MAX) {
		return EBADF;
	}
	return sys_write_fdnode(fd, userbuf, userbuflen, retval);
}

// This function is only to be called from `sys_write_out()` and `sys_write_err()`.
// `userbuf` represents a `const void *` passed from user space.
// This function returns 0 for success or an error number for failure.
static
int
sys_write_outerr(const_userptr_t userbuf, size_t userbuflen, int32_t *retval)
{
	if (userbuflen == 0) {
		*retval = (int32_t)0;
		return 0;
	}

	// Implementation based on `fstest_write()` and `con_io()` and at least one of the `sys_*()` functions above

	void *kernbuf;
	int result;
	size_t numbyteswritten;
	// struct vnode *vn; // corresponds to whatever console-output-mechanism is shared by `stdout` and `stderr`
	struct iovec iov;
	struct uio ku;

	kernbuf = kmalloc(userbuflen);
	if (kernbuf == NULL) {
		return ENOMEM;
	}

	// result = find_console_vnode(&vn);
	// if (result) {
	// 	kfree(kernbuf);
	// 	return result;
	// }

	result = copyin(userbuf, kernbuf, userbuflen);
	if (result) {
		kfree(kernbuf);
		return result;
	}

	uio_kinit(&iov, &ku, kernbuf, userbuflen, 0, UIO_WRITE);

	// result = VOP_WRITE(vn, &ku); // calls `dev_write()`, which calls `con_io`

	// This part is based on `con_io()`:
	char ch;
	while (ku.uio_resid > 0) {
		result = uiomove(&ch, 1, &ku);
		if (result) {
			kfree(kernbuf);
			return result;
		}
		putch(ch);
	}

	if (result) {
		kfree(kernbuf);
		return result;
	}

	numbyteswritten = (size_t)ku.uio_offset;

	kfree(kernbuf);
	*retval = (int32_t)numbyteswritten;

	return 0;
}

// This function is only to be called from `sys_write()`.
// `userbuf` represents a `const void *` passed from user space.
// This function returns 0 for success or an error number for failure.
static
int
sys_write_out(const_userptr_t userbuf, size_t userbuflen, int32_t *retval)
{
	return sys_write_outerr(userbuf, userbuflen, retval);
}

// This function is only to be called from `sys_write()`.
// `userbuf` represents a `const void *` passed from user space.
// This function returns 0 for success or an error number for failure.
static
int
sys_write_err(const_userptr_t userbuf, size_t userbuflen, int32_t *retval)
{
	return sys_write_outerr(userbuf, userbuflen, retval);
}

// This function is only to be called from `sys_write()`.
// `userbuf` represents a `const void *` passed from user space.
// This function returns 0 for success or an error number for failure.
static
int
sys_write_fdnode(int fd, const_userptr_t userbuf, size_t userbuflen, int32_t *retval)
{
	if (fd < FD_MIN || fd > FD_MAX) {
		return EBADF;
	}

	if (userbuflen == 0) {
		*retval = (int32_t)0;
		return 0;
	}

	void *kernbuf;
	int result;
	size_t numbyteswritten;
	struct vnode *vn; // the `vnode` previously linked to `fd` by `fd_allocate()`
	struct iovec iov;
	struct uio ku;

	kernbuf = kmalloc(userbuflen);
	if (kernbuf == NULL) {
		return ENOMEM;
	}

	result = copyin(userbuf, kernbuf, userbuflen);
	if (result) {
		kfree(kernbuf);
		return result;
	}

	fdnode_ensure_lock_not_null();

	lock_acquire(fdnode_lock);
	result = fd_lookup(&vn, fd);
	if (result) {
		lock_release(fdnode_lock);
		kfree(kernbuf);
		return result;
	}

	uio_kinit(&iov, &ku, kernbuf, userbuflen, 0, UIO_WRITE);
	result = VOP_WRITE(vn, &ku);
	if (result) {
		lock_release(fdnode_lock);
		kfree(kernbuf);
		return result;
	}

	if (ku.uio_resid > 0) {
		// kprintf("Short write: %lu bytes left over\n",
		// 	(unsigned long) ku.uio_resid);
	}

	numbyteswritten = (size_t)ku.uio_offset;

	if (numbyteswritten != userbuflen) {
		// kprintf("%lu bytes written, should have been %lu!\n",
		// 	(unsigned long) numbyteswritten,
		// 	(unsigned long) userbuflen);
	}

	lock_release(fdnode_lock);
	kfree(kernbuf);

	// kprintf("%lu bytes written\n", (unsigned long) numbyteswritten);
	*retval = (int32_t)numbyteswritten;

	return 0;
}

// This function is only to be called from `syscall()`.
// `userbuf` represents a `void *` passed from user space.
// This function returns 0 for success or an error number for failure.
static
int
sys_read(int fd, userptr_t userbuf, size_t userbuflen, int32_t *retval)
{
	if (fd == STDIN_FILENO) {
		return sys_read_in(userbuf, userbuflen, retval);
	}
	if (fd == STDOUT_FILENO) {
		return EBADF;
	}
	if (fd == STDERR_FILENO) {
		return EBADF;
	}
	if (fd < FD_MIN || fd > FD_MAX) {
		return EBADF;
	}
    return sys_read_fdnode(fd, userbuf, userbuflen, retval);
}

// This function is only to be called from `sys_read()`.
// `userbuf` represents a `void *` passed from user space.
// This function returns 0 for success or an error number for failure.
static
int
sys_read_in(userptr_t userbuf, size_t userbuflen, int32_t *retval)
{
	if (userbuflen == 0) {
		*retval = (int32_t)0;
		return 0;
	}

	// Implementation based on `sys___getcwd()` and `sys_write_fdnode()` and `fstest_read()` and `con_io()`

	void *kernbuf;
	int result = 0;
	size_t numbytesread;
	// struct vnode *vn; // corresponds to `stdin`
	struct iovec iov;
	struct uio ku;

	kernbuf = kmalloc(userbuflen);
	if (kernbuf == NULL) {
		return ENOMEM;
	}

	// result = find_console_vnode(&vn);
	// if (result) {
	// 	kfree(kernbuf);
	// 	return result;
	// }

	uio_kinit(&iov, &ku, kernbuf, userbuflen, 0, UIO_READ);

	// result = VOP_READ(vn, &ku); // calls `dev_read()`, which calls `con_io()`

	// This part is based on `con_io()`:
	char ch;
	while (ku.uio_resid > 0) {
		ch = getch();
		result = uiomove(&ch, 1, &ku);
		if (result) {
			kfree(kernbuf);
			return result;
		}
	}

	if (result) {
		kfree(kernbuf);
		return result;
	}

	numbytesread = (size_t)ku.uio_offset;

	result = copyout((const void *)kernbuf, userbuf, numbytesread);
	if (result) {
		kfree(kernbuf);
		return result;
	}

	kfree(kernbuf);
	*retval = (int32_t)numbytesread;

	return 0;
}

// This function is only to be called from `sys_read()`.
// `userbuf` represents a `void *` passed from user space.
// This function returns 0 for success or an error number for failure.
static
int
sys_read_fdnode(int fd, userptr_t userbuf, size_t userbuflen, int32_t *retval)
{
	if (fd < FD_MIN || fd > FD_MAX) {
		return EBADF;
	}

	if (userbuflen == 0) {
		*retval = (int32_t)0;
		return 0;
	}

	void *kernbuf;
	int result;
	size_t numbytesread;
	struct vnode *vn; // the `vnode` previously linked to `fd` by `fd_allocate()`
	struct iovec iov;
	struct uio ku;

	kernbuf = kmalloc(userbuflen);
	if (kernbuf == NULL) {
		return ENOMEM;
	}

	fdnode_ensure_lock_not_null();

	lock_acquire(fdnode_lock);
	result = fd_lookup(&vn, fd);
	if (result) {
		lock_release(fdnode_lock);
		kfree(kernbuf);
		return result;
	}

	uio_kinit(&iov, &ku, kernbuf, userbuflen, 0, UIO_READ);

	result = VOP_READ(vn, &ku);
	if (result) {
		lock_release(fdnode_lock);
		kfree(kernbuf);
		return result;
	}

	if (ku.uio_resid > 0) {
		// kprintf("Short read: %lu bytes left over\n",
		// 	(unsigned long) ku.uio_resid);
	}

	numbytesread = (size_t)ku.uio_offset;

	if (numbytesread != userbuflen) {
		// kprintf("%lu bytes read, should have been %lu!\n",
		// 	(unsigned long) numbytesread,
		// 	(unsigned long) userbuflen);
	}

	lock_release(fdnode_lock);

	result = copyout((const void *)kernbuf, userbuf, numbytesread);
	if (result) {
		kfree(kernbuf);
		return result;
	}

	kfree(kernbuf);
	*retval = (int32_t)numbytesread;

    return 0;
}

static
int
sys_getpid(int32_t *retval)
{
	int result;
	pid_t pid;

	result = curproc_getpid(&pid);
	if (result) {
		return result;
	}

	*retval = (int32_t)pid;
    return 0;
}

static
void
sys__exit(int exitcode)
{
	struct proc *proc = curproc;
	KASSERT(proc != kproc);

	char *p_name;
	unsigned p_numthreads;

	spinlock_acquire(&proc->p_lock);
	p_numthreads = proc->p_numthreads;
	KASSERT(p_numthreads == 1); // proc.h says, "And, unless you implement multithreaded user processes, this number will not exceed 1 except in kproc."
	p_name = proc->p_name;
	kprintf("%s(%d): curproc has p_name of \"%s\" and p_numthreads of %u\n", __func__, exitcode, p_name, p_numthreads);
	spinlock_release(&proc->p_lock);

	// This is the only function so far that:
	// (1) is kernel-space
	// (2) is labeled __DEAD
	// (3) is intended for exiting, not entering
	thread_exit();
}
