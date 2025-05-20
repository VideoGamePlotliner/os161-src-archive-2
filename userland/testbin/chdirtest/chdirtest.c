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

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <err.h>
#include <string.h>
#include <stdio.h>

// This file is based on `userland/testbin/getpidtest/getpidtest.c` and `userland/bin/mkdir/mkdir.c`.

/* Based on part of `kern/include/kern/limits.h` */
#ifndef __PATH_MAX
#define __PATH_MAX 1024
#endif /* __PATH_MAX */

/* Based on part of `kern/include/limits.h` */
#ifndef PATH_MAX
#define PATH_MAX __PATH_MAX
#endif /* PATH_MAX */

#define TEST_CHDIR(pathname) \
    if (chdir(pathname))     \
    err(1, "TEST_CHDIR(\"%s\") failed", pathname)

// Local variables __cwdbuf0, __cwdbuf1, ...
#define CWD_BUF(num) \
    __cwdbuf##num
#define CWD_BUF_NULL_TERM(num) \
    CWD_BUF(num)               \
    [sizeof(CWD_BUF(num)) - 1] = 0
#define CWD_BUF_PRINT(num) \
    (CWD_BUF_NULL_TERM(num), printf("CWD_BUF_PRINT(%d): \"%s\"\n", num, CWD_BUF(num)))
#define CWD_BUF_CHECK(num, expected_pathname)                              \
    if ((CWD_BUF_NULL_TERM(num), strcmp(CWD_BUF(num), expected_pathname))) \
    warnx("CWD_BUF_CHECK(%d) failed: expected \"%s\", but got \"%s\"", num, expected_pathname, CWD_BUF(num))
#define CWD_BUF_ZERO(num) \
    bzero(CWD_BUF(num), sizeof(CWD_BUF(num)))
#define CWD_BUF_DECL(num)            \
    char CWD_BUF(num)[PATH_MAX + 1]; \
    CWD_BUF_ZERO(num)
#define CWD_BUF_GET(num)                                                  \
    if ((CWD_BUF_ZERO(num), !getcwd(CWD_BUF(num), sizeof(CWD_BUF(num))))) \
    err(1, "CWD_BUF_GET(%d) failed", num)

/*
 * Program to test chdir().
 * Usage: chdirtest DIRECTORY
 *
 * Intended for the basic system calls assignment.
 */

static int test_chdir(const char *directory)
{
    CWD_BUF_DECL(0);
    CWD_BUF_GET(0);
    CWD_BUF_PRINT(0);

    TEST_CHDIR(directory);

    {
        CWD_BUF_DECL(1);
        CWD_BUF_GET(1);
        CWD_BUF_CHECK(1, directory);
        CWD_BUF_PRINT(1);
    }

    TEST_CHDIR(CWD_BUF(0));

    {
        CWD_BUF_DECL(2);
        CWD_BUF_GET(2);
        CWD_BUF_CHECK(2, CWD_BUF(0));
        CWD_BUF_PRINT(2);
    }

    printf("Passed.\n");
    return 0;
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        errx(1, "Usage: chdirtest DIRECTORY");
    }
    return test_chdir(argv[1]);
}
