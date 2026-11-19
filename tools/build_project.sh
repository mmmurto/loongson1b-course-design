#!/bin/bash
# Replicates the LoongIDE 1.1 build for a bare-metal LS1B project (.lxp values:
# CCompiler flags -mips32 -G0 -EL -msoft-float -DLS1B -DOS_NONE, Linker -nostartfiles, Libraries=m)
# Usage: build_project.sh <project_dir> [optimisation] [-extra defines]
set -u
P="$1"
OPT="${2:--O1}"
export PATH=/c/LoongIDE/mips-2015.05/bin:$PATH
cd "$P" || exit 1

INC="-I. -Iinclude -Icore/include -Icore/mips -Ils1x-drv/include -Ilibc ${EXTRA_INC:-}"
FLAGS="-mips32 -G0 -EL -msoft-float -DLS1B ${EXTRA_DEF:--DOS_NONE} $OPT -g -Wall -c -fmessage-length=0 -pipe"
LFLAGS="-mips32 -G0 -EL -msoft-float -nostartfiles"

OUT="${3:-$(basename "$(cd "$P" && pwd)")}.exe"
rm -rf _obj && mkdir -p _obj
fail=0; n=0
for f in $(find . -name '*.c' -o -name '*.S' | grep -v '^./_obj' | sort); do
  o=_obj/$(echo "$f" | sed 's|^./||; s|/|_|g').o
  err=$(mips-sde-elf-gcc.exe $FLAGS $INC "$f" -o "$o" 2>&1)
  if [ -f "$o" ]; then n=$((n+1)); else
    echo "FAIL $f"; echo "$err" | grep -E "error|fatal" | head -3; fail=1
  fi
done
echo "compiled: $n objects"

if [ $fail -eq 0 ]; then
  link_err=$(mips-sde-elf-gcc.exe $LFLAGS -T ld.script _obj/*.o -o "$OUT" -lm -lgcc ${EXTRA_LIBS:-} 2>&1)
  if [ -f "$OUT" ]; then
    echo "LINK OK -> $OUT"
    mips-sde-elf-size.exe "$OUT"
    echo "entry: $(mips-sde-elf-objdump.exe -f "$OUT" | grep 'start address')"
  else
    echo "LINK FAILED"; echo "$link_err" | head -8
  fi
else
  echo "build aborted (compile errors)"
fi
