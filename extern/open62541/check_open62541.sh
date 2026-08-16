#!/bin/bash
set -euo pipefail
cd "$(dirname "$0")"
H=include/open62541.h
C=src/open62541.c
TAG=$(sed -n 's/^TAG=//p' prepare_open62541.sh)
fail() { echo "check_open62541: $*" >&2; exit 1; }
[ "$(grep -c '^#define UA_ARCHITECTURE_POSIX$' $H)" = 0 ] || fail "hard UA_ARCHITECTURE_POSIX define present (DCS-615)"
[ "$(grep -c '^/\* #undef UA_ARCHITECTURE_POSIX \*/$' $H)" = 1 ] || fail "UA_ARCHITECTURE_POSIX not neutralised exactly once"
[ "$(tail -4 $H)" = "$(printf '#if defined(__cplusplus) && defined(UA_HAVE_C11_ATOMICS)\n#undef _Atomic\n#undef atomic_uintptr_t\n#endif')" ] || fail "C11 atomics undef block missing at end of header"
[ "$(grep -c '^#  pragma comment(lib, "iphlpapi.lib")$' $C)" = 1 ] || fail "iphlpapi pragma not present exactly once (OPCUA-3418)"
grep -A2 '^# include <iphlpapi.h>$' $C | grep -q 'pragma comment(lib, "iphlpapi.lib")' || fail "iphlpapi pragma not directly under its include"
grep -q 'amalgamated original file "/include/open62541/types.h"' $H || fail "path stamps leaked the work dir"
[ "$(sed -n '1,5s/^ \* Git-Revision: //p' $H)" = "$TAG" ] || fail "header Git-Revision != TAG"
[ "$(sed -n '1,5s/^ \* Git-Revision: //p' $C)" = "$TAG" ] || fail "source Git-Revision != TAG"
grep -q "^#define UA_OPEN62541_VERSION \"$TAG\"$" $H || fail "UA_OPEN62541_VERSION != TAG"
echo "open62541 bundle $TAG: post-edits and version stamps OK"
