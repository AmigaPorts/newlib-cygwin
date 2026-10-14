# Syncing with upstream newlib-cygwin

The `amiga` branch merges upstream newlib-cygwin from time to time. The last sync merged upstream `80df0a2b5`; use it as `OLD` for the next one.

## Merge

```
git remote add upstream https://sourceware.org/git/newlib-cygwin.git
git fetch upstream
git -c merge.directoryRenames=false merge upstream/main
```

- Turn off directory-rename detection: with it on, git decides `winsup/cygwin` was renamed to `newlib/libc/sys/amigaos` and moves new Cygwin files there.
- `winsup/` and `libgloss/` stay deleted: resolve their modify/delete conflicts with `git rm -r winsup libgloss`, and remove any new files the merge added there.
- File-rename detection still pairs unrelated files now and then. Check `git diff HEAD -- newlib/libc/sys/amigaos` for changes that did not come from the generic headers.

## Header copies

`newlib/libc/sys/amigaos/include` holds copies of many generic headers from `newlib/libc/include`, with Amiga changes and `__stdargs` on the function declarations. A merge does not update them. After the merge:

```
upstream-sync/resync-headers.sh merge OLD upstream/main
# resolve the conflicts it lists
upstream-sync/resync-headers.sh annotate
```

`merge` strips `__stdargs` from each copy, then does a three-way merge of it against the generic header in `OLD` (base) and in `upstream/main`. Most conflicts are upstream changing lines the Amiga copy also changed. `annotate` re-adds `__stdargs` to every prototype, function typedef and function pointer.

## Build files

Regenerate `newlib/configure` and `newlib/Makefile.in` with the autotools versions upstream uses (currently autoconf 2.69 and automake 1.15.1; see the first lines of the generated files):

```
cd newlib && autoreconf
```

Sources in `newlib/libc/sys/amigaos/Makefile.inc` replace generic objects of the same name, because `libc.a` keeps only the last object for each name. That is how the Amiga allocator replaces dlmalloc. Watch for new upstream files whose names collide with Amiga ones.

## Test

Build the toolchain with the merged branch and run `make check-newlib` in the toolchain repository.
