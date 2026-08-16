#!/bin/bash

# the purpose of this script is to make the process of deploying open62541 in open62541-compat
# automated, reproducible

# this script was created to be used only by quasar-developers team.

set -euo pipefail

TAG=v1.5.4

cd "$(dirname "$0")"
WORK=${TMPDIR:-/tmp}/open62541-compat-amalgamation
rm -fr "$WORK"
mkdir -p "$WORK" include src

git clone https://github.com/open62541/open62541.git --depth=1 -b $TAG "$WORK/open62541"
cmake -S "$WORK/open62541" -B "$WORK/open62541/build" -DUA_ENABLE_AMALGAMATION=ON -DUA_ENABLE_METHODCALLS=ON -DUA_LOGLEVEL=100 -DUA_MULTITHREADING=100
cmake --build "$WORK/open62541/build" --target open62541-amalgamation

cp "$WORK/open62541/build/open62541.h" include/open62541.h
sed -i.bak 's|^#define UA_ARCHITECTURE_POSIX$|/* #undef UA_ARCHITECTURE_POSIX */|' include/open62541.h && rm -f include/open62541.h.bak
printf '\n#if defined(__cplusplus) && defined(UA_HAVE_C11_ATOMICS)\n#undef _Atomic\n#undef atomic_uintptr_t\n#endif\n' >> include/open62541.h
cp "$WORK/open62541/build/open62541.c" src/open62541.c
perl -0pi -e 's/# include <iphlpapi\.h>/# include <iphlpapi.h>\n# if defined(_MSC_VER)\n#  pragma comment(lib, "iphlpapi.lib")\n# endif/' src/open62541.c
rm -fr "$WORK"

./check_open62541.sh

flags() { grep -E '^#define UA_(ENABLE|MULTITHREADING|LOGLEVEL|DEBUG|GENERATED)' "$@"; }
if ! diff <(git show HEAD:./include/open62541.h | flags) <(flags include/open62541.h); then
  [ "${ACCEPT_FLAG_DIFF:-0}" = 1 ] || { echo "feature-flag set changed: rerun with ACCEPT_FLAG_DIFF=1 to accept" >&2; exit 1; }
fi

if [ "${1:-}" = "--commit" ]; then
  git add include/open62541.h src/open62541.c
  git commit --author="open62541 <open62541@open62541.org>" -m "Amalgamate open62541 $TAG by prepare_open62541.sh"
fi
