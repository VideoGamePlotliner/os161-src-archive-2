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
 * Implementation of the null device, "null:", which generates an
 * immediate EOF on read and throws away anything written to it.
 */
#include <types.h>
#include <kern/errno.h>
#include <lib.h>
#include <uio.h>
#include <vfs.h>
#include <device.h>

#include <kern/fcntl.h>
#include <uio.h>

/* For open() */
static
int
nullopen(struct device *dev, int openflags)
{
	(void)dev;
	(void)openflags;

	return 0;
}

/* For d_io() */
static
int
nullio(struct device *dev, struct uio *uio)
{
	/*
	 * On write, discard everything without looking at it.
	 * (Notice that you can write to the null device from invalid
	 * buffer pointers and it will still succeed. This behavior is
	 * traditional.)
	 *
	 * On read, do nothing, generating an immediate EOF.
	 */

	(void)dev; // unused

	if (uio->uio_rw == UIO_WRITE) {
		uio->uio_resid = 0;
	}

	return 0;
}

/* For ioctl() */
static
int
nullioctl(struct device *dev, int op, userptr_t data)
{
	/*
	 * No ioctls.
	 */

	(void)dev;
	(void)op;
	(void)data;

	return EINVAL;
}

static const struct device_ops null_devops = {
	.devop_eachopen = nullopen,
	.devop_io = nullio,
	.devop_ioctl = nullioctl,
};

/*
 * Function to create and attach null:
 */
void
devnull_create(void)
{
	int result;
	struct device *dev;

	dev = kmalloc(sizeof(*dev));
	if (dev==NULL) {
		panic("Could not add null device: out of memory\n");
	}

	dev->d_ops = &null_devops;

	dev->d_blocks = 0;
	dev->d_blocksize = 1;

	dev->d_devnumber = 0; /* assigned by vfs_adddev */

	dev->d_data = NULL;

	result = vfs_adddev("null", dev, 0);
	if (result) {
		panic("Could not add null device: %s\n", strerror(result));
	}
}












/* For open() */
static
int
nullprintopen(struct device *dev, int openflags)
{
	// Implementation based on `nullopen()` and `vfs_open()` and `vfs_listdev()`

	kprintf("%s():\n", __func__);

	if (dev == NULL) {
		kprintf("    Block count of device: (n/a)\n");
		kprintf("     Block size of device: (n/a)\n");
		kprintf("  Serial number of device: (n/a)\n");
	}
	else {
		blkcnt_t d_blocks     = dev->d_blocks;
		blksize_t d_blocksize = dev->d_blocksize;
		dev_t d_devnumber     = dev->d_devnumber;

		kprintf("    Block count of device: %u block%s\n", d_blocks,
			(d_blocks == 1) ? "" : "s");
		kprintf("     Block size of device: %u byte%s\n", d_blocksize,
			(d_blocksize == 1) ? "" : "s");
		kprintf("  Serial number of device: %u\n", d_devnumber);
	}

	int how = openflags & O_ACCMODE;
	kprintf("                Read only: %s\n", (how == O_RDONLY) ? "yes" : "no");
	kprintf("               Write only: %s\n", (how == O_WRONLY) ? "yes" : "no");
	kprintf("           Read and write: %s\n", (how == O_RDWR) ? "yes" : "no");
	kprintf("                 Creating: %s\n", (openflags & O_CREAT) ? "yes" : "no");
	kprintf("   Don't create if exists: %s\n", (openflags & O_EXCL) ? "yes" : "no");
	kprintf("               Truncating: %s\n", (openflags & O_TRUNC) ? "yes" : "no");
	kprintf("                Appending: %s\n", (openflags & O_APPEND) ? "yes" : "no");

	return 0;
}

/* For d_io() */
static
int
nullprintio(struct device *dev, struct uio *uio)
{
	// Implementation based on `nullio()` and `nullprintopen()`

	/*
	 * On write, discard everything without looking at it.
	 * (Notice that you can write to the null device from invalid
	 * buffer pointers and it will still succeed. This behavior is
	 * traditional.)
	 *
	 * On read, do nothing, generating an immediate EOF.
	 */

	kprintf("%s():\n", __func__);

	if (dev == NULL) {
		kprintf("    Block count of device: (n/a)\n");
		kprintf("     Block size of device: (n/a)\n");
		kprintf("  Serial number of device: (n/a)\n");
	}
	else {
		blkcnt_t d_blocks     = dev->d_blocks;
		blksize_t d_blocksize = dev->d_blocksize;
		dev_t d_devnumber     = dev->d_devnumber;

		kprintf("    Block count of device: %u block%s\n", d_blocks,
			(d_blocks == 1) ? "" : "s");
		kprintf("     Block size of device: %u byte%s\n", d_blocksize,
			(d_blocksize == 1) ? "" : "s");
		kprintf("  Serial number of device: %u\n", d_devnumber);
	}

	if (uio == NULL) {
		kprintf("                   Offset: (n/a)\n");
		kprintf("           Remaining data: (n/a)\n");
		kprintf("                Operation: (n/a)\n");
	}
	else {
		off_t        uio_offset = uio->uio_offset; /* Desired offset into object */
		size_t       uio_resid  = uio->uio_resid;  /* Remaining amt of data to xfer */
		enum uio_rw  uio_rw     = uio->uio_rw;     /* Whether op is a read or write */

		kprintf("                   Offset: %lld\n", uio_offset);
		kprintf("           Remaining data: %u byte%s\n", uio_resid,
			(uio_resid == 1) ? "" : "s");
		kprintf("                Operation: %s\n",
			(uio_rw == UIO_READ) ? "Reading" : ((uio_rw == UIO_WRITE) ? "Writing" : "(unknown)"));

		if (uio->uio_rw == UIO_WRITE) {
			uio->uio_resid = 0;

			uio_resid = uio->uio_resid; // Update `uio_resid` with the new value

			kprintf("   Remaining data post-op: %u byte%s\n", uio_resid,
					(uio_resid == 1) ? "" : "s");
		}
	}

	return 0;
}

/* For ioctl() */
static
int
nullprintioctl(struct device *dev, int op, userptr_t data)
{
	// Implementation based on `nullioctl()` and `nullprintopen()`

	/*
	 * No ioctls.
	 */

	(void)data;

	kprintf("%s():\n", __func__);

	if (dev == NULL) {
		kprintf("    Block count of device: (n/a)\n");
		kprintf("     Block size of device: (n/a)\n");
		kprintf("  Serial number of device: (n/a)\n");
	}
	else {
		blkcnt_t d_blocks     = dev->d_blocks;
		blksize_t d_blocksize = dev->d_blocksize;
		dev_t d_devnumber     = dev->d_devnumber;

		kprintf("    Block count of device: %u block%s\n", d_blocks,
			(d_blocks == 1) ? "" : "s");
		kprintf("     Block size of device: %u byte%s\n", d_blocksize,
			(d_blocksize == 1) ? "" : "s");
		kprintf("  Serial number of device: %u\n", d_devnumber);
	}

	kprintf("     IOCTL operation code: %d\n", op);

	return 0;
}

// Based on `null_devops`
static const struct device_ops nullprint_devops = {
	.devop_eachopen = nullprintopen,
	.devop_io = nullprintio,
	.devop_ioctl = nullprintioctl,
};

/*
 * Function to create and attach nullprint:
 */
void
devnullprint_create(void)
{
	// Implementation based on `devnull_create()`

	int result;
	struct device *dev;

	dev = kmalloc(sizeof(*dev));

	if (dev == NULL) {
		panic("Could not add nullprint device: out of memory\n");
	}

	dev->d_ops = &nullprint_devops;

	dev->d_blocks = 0;
	dev->d_blocksize = 1;

	dev->d_devnumber = 0; /* assigned by vfs_adddev */

	dev->d_data = NULL;

	// Should `dev` be mountable?
	// Should we have a mountable nullprint device AND a non-mountable nullprint device?
	//
	// Answer to each question: no, because the comment for `vfs_doadd()` says...
	//
	// If "mountable" is set, the device will be treated as one that expects
	// to have a filesystem mounted on it, and a raw device will be created
	// for direct access.
	result = vfs_adddev("nullprint", dev, 0);
	if (result) {
		panic("Could not add nullprint device: %s\n", strerror(result));
	}
}
