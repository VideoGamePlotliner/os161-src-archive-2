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

/*
 * Device test code.
 */
#include <types.h>
#include <lib.h>
#include <test.h>
#include <uio.h>
#include <kern/fcntl.h>
#include <vfs.h>
#include <vnode.h>

#define DEVTEST1_IOCTL_OPERATION_CODE 54816
#define DEVTEST1_INPUT_BUFFER_SIZE 128
#define DEVTEST1_OUTPUT_BUFFER_SIZE 64
#define DEVTEST1_DEVICE_PATH "nullprint:"



/* Copied from `kern/include/types.h` */

typedef struct __userptr *userptr_t;



int
devtest1(int nargs, char **args)
{
    // Implementation based on `printfile()`

	(void)nargs;
	(void)args;

	struct vnode *vn;
	struct iovec iov;
	struct uio ku;
	char inbuf[DEVTEST1_INPUT_BUFFER_SIZE];
    char outbuf[DEVTEST1_OUTPUT_BUFFER_SIZE];
	char path[16];
	int result;

    KASSERT(sizeof(path) > strlen(DEVTEST1_DEVICE_PATH));

	/* vfs_open destroys the string it's passed; make a copy */
	strcpy(path, DEVTEST1_DEVICE_PATH);

    /* null terminate */
    path[sizeof(path) - 1] = 0;

	result = vfs_open(path, O_RDWR, 0664, &vn);
	if (result) {
		kprintf("devtest1: vfs_open: %s\n", strerror(result));
		return result;
	}

    uio_kinit(&iov, &ku, inbuf, sizeof(inbuf), 0, UIO_READ);
    result = VOP_READ(vn, &ku);
    if (result) {
		kprintf("%s: VOP_READ: %s\n", __func__, strerror(result));
		vfs_close(vn);
		return result;
    }

    if (ku.uio_resid > 0) {
        kprintf("%s: Warning: short read\n", __func__);
    }

    uio_kinit(&iov, &ku, outbuf, sizeof(outbuf), 0, UIO_WRITE);
    result = VOP_WRITE(vn, &ku);
    if (result) {
		kprintf("%s: VOP_WRITE: %s\n", __func__, strerror(result));
		vfs_close(vn);
		return result;
    }

    if (ku.uio_resid > 0) {
        kprintf("%s: Warning: short write\n", __func__);
    }

    result = VOP_IOCTL(vn, DEVTEST1_IOCTL_OPERATION_CODE, (userptr_t)0);
	if (result) {
		kprintf("%s: VOP_IOCTL: %s\n", __func__, strerror(result));
		vfs_close(vn);
		return result;
	}

	vfs_close(vn);

	return 0;

}
