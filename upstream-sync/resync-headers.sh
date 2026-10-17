#!/bin/sh
# Re-sync the amigaos copies of newlib's generic headers after merging
# upstream newlib-cygwin; see README.md.
#
#   upstream-sync/resync-headers.sh merge OLD NEW
#       three-way merge each copy in newlib/libc/sys/amigaos/include that
#       has a generic counterpart in OLD's newlib/libc/include: base OLD,
#       theirs NEW, ours the copy with __stdargs stripped.  Writes the
#       results back without __stdargs; conflicts are left as diff3
#       markers and listed.
#   upstream-sync/resync-headers.sh annotate
#       re-add __stdargs to every copy, once the conflicts are resolved.
#
# OLD is the upstream commit of the previous sync, NEW the one merged now.
set -e
cd "$(git rev-parse --show-toplevel)"
inc=newlib/libc/sys/amigaos/include
gen=newlib/libc/include
here=upstream-sync
work=$(git rev-parse --git-dir)/upstream-sync

copies() {
	(cd $inc && find . -name '*.h' | sed 's|^\./||' | sort) | while read -r f; do
		git cat-file -e "$1:$gen/$f" 2>/dev/null && echo "$f"
	done
}

case "$1" in
merge)
	old=$2 new=$3
	[ -n "$old" ] && [ -n "$new" ] || { echo "usage: $0 merge OLD NEW" >&2; exit 2; }
	rm -rf "$work" && mkdir -p "$work"
	copies "$old" > "$work/list"
	conflicts=
	while read -r f; do
		t=$work/$(echo "$f" | tr / _)
		if ! git cat-file -e "$new:$gen/$f" 2>/dev/null; then
			echo "removed upstream, left alone: $f"
			continue
		fi
		git show "$old:$gen/$f" > "$t.base"
		git show "$new:$gen/$f" > "$t.theirs"
		$here/stdargs.py strip "$inc/$f" > "$t.ours"
		if ! git merge-file -q --diff3 -L amiga -L base -L upstream "$t.ours" "$t.base" "$t.theirs"; then
			conflicts="$conflicts $f"
		fi
		cp "$t.ours" "$inc/$f"
	done < "$work/list"
	echo "merged $(wc -l < "$work/list") copies; resolve the conflicts in:"
	for f in $conflicts; do echo "  $inc/$f"; done
	echo "then run: $0 annotate"
	;;
annotate)
	if grep -rl '^<<<<<<< \|^>>>>>>> ' $inc; then
		echo "resolve the conflicts in the files above first" >&2
		exit 1
	fi
	copies HEAD | while read -r f; do
		$here/stdargs.py add "$inc/$f" > "$inc/$f.tmp" && mv "$inc/$f.tmp" "$inc/$f"
	done
	;;
*)
	echo "usage: $0 merge OLD NEW | annotate" >&2
	exit 2
	;;
esac
