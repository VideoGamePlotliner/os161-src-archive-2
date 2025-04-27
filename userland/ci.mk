# The following is based on
# https://github.com/VideoGamePlotliner/c-cpp-sandbox-archive/blob/daf6750d146a248c878e6aacd1deac60cfe578f0/Makefile
# https://github.com/VideoGamePlotliner/c-cpp-sandbox-archive/blob/daf6750d146a248c878e6aacd1deac60cfe578f0/cpp/normal/Makefile

# https://www.gnu.org/software/make/manual/make.html#Automatic-Variables
# https://www.gnu.org/software/make/manual/make.html#Combine-By-Prerequisite
# https://www.gnu.org/software/make/manual/make.html#Error-Messages
# https://www.gnu.org/software/make/manual/make.html#index-C_002c-rule-to-compile
# https://www.gnu.org/software/make/manual/make.html#index-test-_0028standard-target_0029
# https://www.gnu.org/software/make/manual/make.html#make-Deduces
# https://www.gnu.org/software/make/manual/make.html#Pattern-Examples
# https://www.gnu.org/software/make/manual/make.html#Pattern_002dspecific
# https://www.gnu.org/software/make/manual/make.html#Phony-Targets
# https://www.gnu.org/software/make/manual/make.html#Recursion
# https://www.gnu.org/software/make/manual/make.html#Rule-Introduction
# https://www.gnu.org/software/make/manual/make.html#Variables-Simplify
# https://www.gnu.org/software/make/manual/make.html#Simple-Makefile

CPPFLAGS += -g -Wall -Werror -Wpedantic

# https://www.gnu.org/software/make/manual/make.html#Wildcard-Pitfall
# https://www.gnu.org/software/make/manual/make.html#Wildcard-Function
ALL_C_FILES = $(wildcard */*/*.c)
ALL_USEMTEST_FILES = $(wildcard sem:usemtest.g*) $(wildcard sem:usemtest.w*)

# https://www.gnu.org/software/make/manual/make.html#Text-Functions
ALL_C_FILES_00 = $(ALL_C_FILES:bin/cat/cat.c=)
ALL_C_FILES_01 = $(ALL_C_FILES_00:bin/cp/cp.c=)
ALL_C_FILES_02 = $(ALL_C_FILES_01:bin/ls/ls.c=)
ALL_C_FILES_03 = $(ALL_C_FILES_02:bin/mv/mv.c=)
ALL_C_FILES_04 = $(ALL_C_FILES_03:bin/rm/rm.c=)
ALL_C_FILES_05 = $(ALL_C_FILES_04:bin/sh/sh.c=)
ALL_C_FILES_06 = $(ALL_C_FILES_05:bin/tac/tac.c=)
ALL_C_FILES_07 = $(ALL_C_FILES_06:lib/hostcompat/err.c=)
ALL_C_FILES_08 = $(ALL_C_FILES_07:lib/hostcompat/hostcompat.c=)
ALL_C_FILES_09 = $(ALL_C_FILES_08:lib/hostcompat/ntohll.c=)
ALL_C_FILES_10 = $(ALL_C_FILES_09:lib/hostcompat/time.c=)
ALL_C_FILES_11 = $(ALL_C_FILES_10:lib/libtest/triple.c=)
ALL_C_FILES_12 = $(ALL_C_FILES_11:sbin/dumpsfs/dumpsfs.c=)
ALL_C_FILES_13 = $(ALL_C_FILES_12:sbin/halt/halt.c=)
ALL_C_FILES_14 = $(ALL_C_FILES_13:sbin/mksfs/disk.c=)
ALL_C_FILES_15 = $(ALL_C_FILES_14:sbin/mksfs/mksfs.c=)
ALL_C_FILES_16 = $(ALL_C_FILES_15:sbin/mksfs/support.c=)
ALL_C_FILES_17 = $(ALL_C_FILES_16:sbin/poweroff/poweroff.c=)
ALL_C_FILES_18 = $(ALL_C_FILES_17:sbin/reboot/reboot.c=)
ALL_C_FILES_19 = $(ALL_C_FILES_18:sbin/sfsck/freemap.c=)
ALL_C_FILES_20 = $(ALL_C_FILES_19:sbin/sfsck/inode.c=)
ALL_C_FILES_21 = $(ALL_C_FILES_20:sbin/sfsck/main.c=)
ALL_C_FILES_22 = $(ALL_C_FILES_21:sbin/sfsck/pass1.c=)
ALL_C_FILES_23 = $(ALL_C_FILES_22:sbin/sfsck/pass2.c=)
ALL_C_FILES_24 = $(ALL_C_FILES_23:sbin/sfsck/sb.c=)
ALL_C_FILES_25 = $(ALL_C_FILES_24:sbin/sfsck/sfs.c=)
ALL_C_FILES_26 = $(ALL_C_FILES_25:sbin/sfsck/utils.c=)
ALL_C_FILES_27 = $(ALL_C_FILES_26:testbin/badcall/%.c=)
ALL_C_FILES_28 = $(ALL_C_FILES_27:testbin/bigexec/bigexec.c=)
ALL_C_FILES_29 = $(ALL_C_FILES_28:testbin/bigfile/bigfile.c=)
ALL_C_FILES_30 = $(ALL_C_FILES_29:testbin/bigfork/bigfork.c=)
ALL_C_FILES_31 = $(ALL_C_FILES_30:testbin/bigseek/bigseek.c=)
ALL_C_FILES_32 = $(ALL_C_FILES_31:testbin/crash/crash.c=)
ALL_C_FILES_33 = $(ALL_C_FILES_32:testbin/dirseek/dirseek.c=)
ALL_C_FILES_34 = $(ALL_C_FILES_33:testbin/f_test/f_read.c=)
ALL_C_FILES_35 = $(ALL_C_FILES_34:testbin/f_test/f_test.c=)
ALL_C_FILES_36 = $(ALL_C_FILES_35:testbin/f_test/f_write.c=)
ALL_C_FILES_37 = $(ALL_C_FILES_36:testbin/farm/farm.c=)
ALL_C_FILES_38 = $(ALL_C_FILES_37:testbin/filetest/filetest.c=)
ALL_C_FILES_39 = $(ALL_C_FILES_38:testbin/filetestaltered/filetestaltered.c=)
ALL_C_FILES_40 = $(ALL_C_FILES_39:testbin/forktest/forktest.c=)
ALL_C_FILES_41 = $(ALL_C_FILES_40:testbin/frack/check.c=)
ALL_C_FILES_42 = $(ALL_C_FILES_41:testbin/frack/data.c=)
ALL_C_FILES_43 = $(ALL_C_FILES_42:testbin/frack/do.c=)
ALL_C_FILES_44 = $(ALL_C_FILES_43:testbin/frack/main.c=)
ALL_C_FILES_45 = $(ALL_C_FILES_44:testbin/frack/name.c=)
ALL_C_FILES_46 = $(ALL_C_FILES_45:testbin/frack/ops.c=)
ALL_C_FILES_47 = $(ALL_C_FILES_46:testbin/frack/pool.c=)
ALL_C_FILES_48 = $(ALL_C_FILES_47:testbin/frack/workloads.c=)
ALL_C_FILES_49 = $(ALL_C_FILES_48:testbin/getpidtest/getpidtest.c=)
ALL_C_FILES_50 = $(ALL_C_FILES_49:testbin/multiexec/multiexec.c=)
ALL_C_FILES_51 = $(ALL_C_FILES_50:testbin/palin/palin.c=)
ALL_C_FILES_52 = $(ALL_C_FILES_51:testbin/parallelvm/parallelvm.c=)
ALL_C_FILES_53 = $(ALL_C_FILES_52:testbin/poisondisk/poisondisk.c=)
ALL_C_FILES_54 = $(ALL_C_FILES_53:testbin/randcall/main.c=)
ALL_C_FILES_55 = $(ALL_C_FILES_54:testbin/redirect/redirect.c=)
ALL_C_FILES_56 = $(ALL_C_FILES_55:testbin/rmdirtest/rmdirtest.c=)
ALL_C_FILES_57 = $(ALL_C_FILES_56:testbin/rmtest/rmtest.c=)
ALL_C_FILES_58 = $(ALL_C_FILES_57:testbin/sbrktest/sbrktest.c=)
ALL_C_FILES_59 = $(ALL_C_FILES_58:testbin/schedpong/grind.c=)
ALL_C_FILES_60 = $(ALL_C_FILES_59:testbin/schedpong/main.c=)
ALL_C_FILES_61 = $(ALL_C_FILES_60:testbin/schedpong/pong.c=)
ALL_C_FILES_62 = $(ALL_C_FILES_61:testbin/schedpong/results.c=)
ALL_C_FILES_63 = $(ALL_C_FILES_62:testbin/schedpong/think.c=)
ALL_C_FILES_64 = $(ALL_C_FILES_63:testbin/schedpong/usem.c=)
ALL_C_FILES_65 = $(ALL_C_FILES_64:testbin/triplehuge/triplehuge.c=)
ALL_C_FILES_66 = $(ALL_C_FILES_65:testbin/tail/tail.c=)
ALL_C_FILES_67 = $(ALL_C_FILES_66:testbin/triplemat/triplemat.c=)
ALL_C_FILES_68 = $(ALL_C_FILES_67:testbin/triplesort/triplesort.c=)
ALL_C_FILES_69 = $(ALL_C_FILES_68:testbin/userthreads/userthreads.c=)
ALL_C_FILES_70 = $(ALL_C_FILES_69:testbin/zero/zero.c=)
ALL_MAIN_FILES = $(ALL_C_FILES_70:.c=)
ALL_O_FILES = $(ALL_C_FILES_70:.c=.o)
ALL_C_FILES_71 = $(ALL_C_FILES_70:testbin/bloat/bloat.c=)
ALL_C_FILES_72 = $(ALL_C_FILES_71:testbin/forkbomb/forkbomb.c=)
ALL_C_FILES_73 = $(ALL_C_FILES_72:testbin/malloctest/malloctest.c=)
ALL_C_FILES_74 = $(ALL_C_FILES_73:testbin/conman/conman.c=)
ALL_C_FILES_75 = $(ALL_C_FILES_74:testbin/conmanaltered/conmanaltered.c=)
ALL_C_FILES_76 = $(ALL_C_FILES_75:testbin/tictac/tictac.c=)
ALL_CHECKS = $(ALL_C_FILES_76:.c=.check)

# https://www.gnu.org/software/make/manual/make.html#Automatic-Prerequisites
# https://gcc.gnu.org/onlinedocs/gcc/Preprocessor-Options.html#Preprocessor-Options
# %.d: %.c
# 	set -e; rm -f $@; \
# 	$(CC) -M $(CPPFLAGS) $< | sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' > $@;

# https://www.gnu.org/software/make/manual/make.html#Include
# include $(ALL_C_FILES:.c=.d)

.PHONY: all
all: $(ALL_MAIN_FILES)

# https://www.gnu.org/software/make/manual/make.html#Secondary-Expansion
.SECONDEXPANSION:
$(ALL_MAIN_FILES): $$@.o
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ $<

# https://www.gnu.org/software/make/manual/make.html#index-test-_0028standard-target_0029
# Remember to run 'make' before running 'make check'
.PHONY: check
check: $(ALL_CHECKS)

$(ALL_CHECKS):
	-./$(patsubst %.check,%,$@)

.PHONY: clean
clean:
	rm -vrf $(ALL_MAIN_FILES) $(ALL_O_FILES) $(ALL_USEMTEST_FILES)
