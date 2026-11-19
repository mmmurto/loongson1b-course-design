#!/bin/bash
# Rebuild a LoongIDE project strictly from its .lxp (what the IDE will use):
#  - sources = files directly in project root + files in each folder of [Folders]
#  - flags/include paths/defines from [CCompiler], libs from [Linker]
# Usage: verify_lxp_build.sh <project_dir>
set -u
P="$1"
cd "$P" || exit 1
LXP=$(ls *.lxp | head -1)
export PATH=/c/LoongIDE/mips-2015.05/bin:$PATH

val() { grep -m1 "^$1=" "$LXP" | cut -d= -f2- ; }

CFLAGS=$(val GxxFlags | sed 's/[[:space:]]*$//')
DEFS=$(val DefinedSymbols | tr ';' ' ')
INCS=$(val IncludePaths | tr ';' ' ')
LFXX=$(awk '/^\[Linker\]/{f=1} f&&/^GxxFlags=/{print;exit}' "$LXP" | cut -d= -f2-)
LIBS=$(awk '/^\[Linker\]/{f=1} f&&/^Libraries=/{print;exit}' "$LXP" | cut -d= -f2- | tr ';' ' ')

INCFLAGS=""
for d in $INCS; do INCFLAGS="$INCFLAGS -I$d"; done
DEFFLAGS=""
for d in $DEFS; do DEFFLAGS="$DEFFLAGS -D$d"; done
LIBFLAGS=""
for l in $LIBS; do LIBFLAGS="$LIBFLAGS -l$l"; done

echo "lxp        : $LXP"
echo "GxxFlags   : $CFLAGS"
echo "Defined    : $DEFS"
echo "Includes   : $INCS"
echo "Linker     : $LFXX | libs: $LIBS"

# ---- source list exactly as the IDE sees it ----
FOLDERS=""
n=$(grep -m1 "^FoldersCount=" "$LXP" | cut -d= -f2)
i=1
while [ "$i" -le "$n" ]; do
  f=$(grep -m1 "^Folders$i=" "$LXP" | cut -d= -f2-)
  [ -n "$f" ] && FOLDERS="$FOLDERS $f"
  i=$((i+1))
done

SRC=$(ls *.c *.S 2>/dev/null)
# the IDE also adds every project folder to the include search path
for d in $FOLDERS; do INCFLAGS="$INCFLAGS -I$d"; done
for d in $FOLDERS; do
  [ -d "$d" ] && SRC="$SRC $(ls $d/*.c $d/*.S 2>/dev/null)"
done

rm -rf _v && mkdir -p _v
nok=0; nbad=0
for f in $SRC; do
  o=_v/$(echo "$f" | sed 's|/|_|g').o
  out=$(mips-sde-elf-gcc.exe $CFLAGS $DEFFLAGS $INCFLAGS "$f" -o "$o" 2>&1)
  if [ -f "$o" ] && ! echo "$out" | grep -q " error:"; then nok=$((nok+1)); else
    nbad=$((nbad+1)); echo "FAIL $f"; echo "$out" | grep -E "error|fatal" | head -3
  fi
done
echo "sources: $((nok+nbad))  ok: $nok  failed: $nbad"

OUT=$(basename "$(pwd)").exe
rm -f "$OUT"
if [ $nbad -eq 0 ]; then
  link=$(mips-sde-elf-gcc.exe $LFXX -T ld.script _v/*.o -o "$OUT" $LIBFLAGS -lgcc 2>&1)
  if [ -f "$OUT" ]; then
    echo "LINK OK -> $OUT"; mips-sde-elf-size.exe "$OUT" | tail -1
  else
    echo "LINK FAILED"; echo "$link" | head -6
  fi
else
  echo "NOT LINKED (compile failures)"
fi
rm -rf _v
