libc_a_SOURCES += \
	%D%/getpid.c %D%/lseek.c %D%/unlink.c %D%/dummy.c %D%/open.c \
	%D%/__adddf3.c %D%/__addsf3.c %D%/__divdf3.c %D%/__divsf3.c \
	%D%/__divsi3.c %D%/__eqdf2.c %D%/__eqsf2.c %D%/__extendsfdf2.c \
	%D%/__fixdfsi.c %D%/__fixsfsi.c %D%/__fixunsdfsi.c %D%/__fixunssfsi.c \
	%D%/__floatsidf.c %D%/__floatsisf.c %D%/__floatunsidf.c %D%/__floatunsisf.c \
	%D%/__muldf3.c %D%/__mulsf3.c %D%/__mulsi3.c %D%/__negdf2.c %D%/__negsf2.c \
	%D%/__subdf3.c %D%/__subsf3.c %D%/__truncdfsf2.c %D%/__udivsi3.c \
	%D%/isinf.c %D%/isnan.c \
	%D%/cxxglue.c %D%/__dosbase.c %D%/__unorddf2.c %D%/__mathieeedoubbas.c \
	%D%/abort.c %D%/basename.c %D%/dirname.c %D%/__nocommandline.c %D%/uname.c \
	%D%/clock.c %D%/statvfs.c %D%/__stdiowin.c \
	%D%/__seterrno.c %D%/stat.c %D%/fstat.c %D%/mkdir.c %D%/chmod.c %D%/chdir.c \
	%D%/gettimeofday.c %D%/sleep.c %D%/utime.c %D%/dirent.c %D%/kill.c %D%/link.c

## The amiga allocator replaces dlmalloc.  libc.a keeps only the last
## object of each name, so these replace stdlib's objects of the same
## names: the dlmalloc parts with no amiga counterpart become empty.
libc_a_SOURCES += \
	%D%/mallocr.cpp %D%/callocr.c %D%/reallocr.c \
	%D%/cfreer.c %D%/freer.c %D%/malignr.c %D%/mallinfor.c %D%/malloptr.c \
	%D%/mallstatsr.c %D%/msizer.c %D%/pvallocr.c %D%/vallocr.c

## The startup file is installed as newlib-crt0.o, not crt0.o: on this
## target crt0.o, bcrt0.o and friends are ixemul's, and a crt0.o in the
## multilib directories shadowed ixemul's flat ixemul/lib/crt0.o in gcc's
## startfile search.  configure.host turns the generic crt0.o off.
toollib_DATA += newlib-crt0.o
newlib-crt0.o: %D%/crt0.o
	$(AM_V_at)rm -f $@
	$(AM_V_GEN)ln $< $@ >/dev/null 2>/dev/null || cp $< $@
%D%/crt0.o: stmp-targ-include
CLEANFILES += newlib-crt0.o
if HAVE_MULTISUBDIR
noinst_DATA += $(BUILD_MULTISUBDIR)/newlib-crt0.o
$(BUILD_MULTISUBDIR)/newlib-crt0.o: %D%/crt0.o $(BUILD_MULTISUBDIR)
	$(AM_V_at)rm -f $@
	$(AM_V_GEN)ln $< $@ >/dev/null 2>/dev/null || cp $< $@
endif

## Earlier installs put the startup file in toollibdir as crt0.o, where it
## would keep shadowing ixemul's; remove it.
INSTALL_DATA_LOCAL += install-amigaos-remove-crt0
install-amigaos-remove-crt0:
	rm -f $(DESTDIR)$(toollibdir)/crt0.o
