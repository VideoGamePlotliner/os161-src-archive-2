#
# Makefile for src/testbin (sources for programs installed in /testbin)
#

TOP=../..
.include "$(TOP)/mk/os161.config.mk"

SUBDIRS=add argtest badcall bigexec bigfile bigfork bigseek bloat chdirtest conman conmanaltered \
	crash ctest dirconc dirseek dirtest f_test factorial farm faulter \
	filetest filetestaltered forkbomb forktest frack getpidtest hash hog huge \
	malloctest matmult multiexec palin parallelvm poisondisk psort \
	randcall redirect rmdirtest rmtest \
	sbrktest schedpong sort sparsefile tail tictac triplehuge \
	triplemat triplesort usemtest zero

# But not:
#    userthreads    (no support in kernel API in base system)

.include "$(TOP)/mk/os161.subdir.mk"
