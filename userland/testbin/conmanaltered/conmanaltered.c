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
 * conmanaltered.c
 *
 * Echoes characters once a certain number of them are typed.
 * This should work once the basic system calls are implemented.
 */

#include <unistd.h>
#include <err.h>

#include <string.h>
#include <stdio.h>
#include <errno.h>

#define CONMANALTERED_MAIN_BUFSIZE 4

int
main(void)
{
	if (CONMANALTERED_MAIN_BUFSIZE <= 0) {
		printf("\nERROR: CONMANALTERED_MAIN_BUFSIZE <= 0\n");
		return 3;
	}

	char buf[CONMANALTERED_MAIN_BUFSIZE];
	ssize_t len;
	int errn;

	bzero((void *)buf, sizeof(buf));

	errno = 0;
	len = read(STDIN_FILENO, (void *)buf, sizeof(buf));
	errn = errno;

	printf("\nNum bytes read via `read()`: %ld\nErrno: %d (%s)\n", (long)len, errn, strerror(errn));

	if (len < (ssize_t)0 || errn != 0) {
		printf("\nERROR: read from stdin failed\n");
		return 1;
	}
	else if (len == (ssize_t)0) {
		printf("\nEOF\n");
	}
	else {
		size_t numbytestowrite = sizeof(buf);

		printf("\nNum bytes to write: %lu\n", (unsigned long)numbytestowrite);

		errno = 0;
		len = write(STDOUT_FILENO, (const void *)buf, numbytestowrite);
		errn = errno;

		printf("\nNum bytes written via `write()`: %ld\nErrno: %d (%s)\n", (long)len, errn, strerror(errn));

		if (len != (ssize_t)numbytestowrite) {
			printf("\nWARNING: len does not equal %lu\n", (unsigned long)numbytestowrite);
		}
		if (errn != 0) {
			printf("\nERROR: write to stdout failed\n");
			return 2;
		}

	}
	return 0;
}
