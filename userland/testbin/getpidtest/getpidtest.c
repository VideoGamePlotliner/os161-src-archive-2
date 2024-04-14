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
 * Program to test getpid().
 *
 * Intended for the basic system calls assignment.
 */

#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <kern/limits.h>



/* Based on parts of `kern/include/kern/limits.h` */

#ifndef PID_MIN
#define PID_MIN __PID_MIN
#endif /* PID_MIN */

#ifndef PID_MAX
#define PID_MAX __PID_MAX
#endif /* PID_MAX */



static int test_getpid(pid_t *p);



int
main(void)
{
    pid_t pid1;
    pid_t pid2;
    int result;

    result = test_getpid(&pid1);
    if (result) {
        printf("\nERROR: test_getpid() failed the first time\n");
        return 1;
    }

    result = test_getpid(&pid2);
    if (result) {
        printf("\nERROR: test_getpid() failed the second time\n");
        return 2;
    }

    if (pid1 != pid2) {
        printf("\nERROR: getpid() returned a different value the second time\n");
        return 3;
    }

    printf("\nPassed getpidtest\n");
	return 0;
}

// This function is only to be called from `main()` -- the `main()` function in this file, that is.
// This function returns 0 for success or an error number for failure.
// If success occurs, `*p` is set to the return value of `getpid()`. Otherwise, `*p` is unaffected.
static
int
test_getpid(pid_t *p)
{
    if (p == NULL) {
        printf("\nERROR: argument for %s cannot be NULL\n", __func__);
        return EINVAL;
    }

    errno = 0;
    const pid_t getpid_return_value = getpid();
    const int errn = errno;

    printf("\ngetpid() returned %d\nErrno: %d (%s)\n", (int)getpid_return_value, errn, strerror(errn));

    if (errn != 0) {
        printf("\nERROR: getpid() failed\n");
        return errn;
    }
    if (getpid_return_value < PID_MIN) {
        printf("\nERROR: return value of getpid() was less than %d\n", (int)PID_MIN);
        return ERANGE;
    }
    if (getpid_return_value > PID_MAX) {
        printf("\nERROR: return value of getpid() was greater than %d\n", (int)PID_MAX);
        return ERANGE;
    }

    *p = getpid_return_value;
    return 0;
}
