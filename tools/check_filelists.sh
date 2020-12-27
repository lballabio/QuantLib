#!/bin/bash

# execute this script from the root of an uncompressed QuantLib tarball

# get reference lists of distributed files (done with find; this is
# why this script should be run from an uncompressed tarball created
# with 'make dist', not from a working copy.)

find ql -name '*.[hc]pp' -or -name '*.[hc]' \
| grep -v 'ql/config\.hpp' | sort > ql.ref.files
find test-suite -name '*.[hc]pp' \
| grep -v 'quantlibbenchmark' | grep -v '/main\.cpp' \
| sort > test-suite.ref.files
find test-suite -name '*.cpp' \
| grep -v 'quantlibbenchmark' \
| sort > test-suite-cpp.ref.files

# extract file names from VC++ projects and clean up so that they
# have the same format as the reference lists.

grep -o -E 'Include=".*\.[hc]p*"' QuantLib.vcxproj \
| awk -F'"' '{ print $2 }' | sed -e 's|\\|/|g' | sed -e 's|^./||' \
| sort > ql.vcx.files

grep -o -E 'Include=".*\.[hc]p*"' test-suite/testsuite.vcxproj \
| awk -F'"' '{ print $2 }' | sed -e 's|\\|/|g' | sed -e 's|^./||' \
| sed -e 's|^|test-suite/|' | sort > test-suite.vcx.files

grep -o -E 'Include=".*\.[hc]p*"' QuantLib.vcxproj.filters \
| awk -F'"' '{ print $2 }' | sed -e 's|\\|/|g' | sed -e 's|^./||' \
| sort > ql.vcx.filters.files

grep -o -E 'Include=".*\.[hc]p*"' test-suite/testsuite.vcxproj.filters \
| awk -F'"' '{ print $2 }' | sed -e 's|\\|/|g' | sed -e 's|^./||' \
| sed -e 's|^|test-suite/|' | sort > test-suite.vcx.filters.files

# same with CMakelists

grep -o -E '[a-zA-Z0-9_/\.]*\.[hc]p*' ql/CMakeLists.txt \
| sed -e 's|^|ql/|' | sort > ql.cmake.files

grep -o -E '[a-zA-Z0-9_/\.]*\.cpp' test-suite/CMakeLists.txt \
| grep -v 'quantlibbenchmark' \
| sed -e 's|^|test-suite/|' | sort -u > test-suite.cmake.files

# write out differences...

diff -b ql.cmake.files ql.ref.files > ql.cmake.diff
diff -b test-suite.cmake.files test-suite-cpp.ref.files > test-suite.cmake.diff

diff -b ql.vcx.files ql.ref.files > ql.vcx.diff
diff -b test-suite.vcx.files test-suite.ref.files > test-suite.vcx.diff

diff -b ql.vcx.filters.files ql.ref.files > ql.vcx.filters.diff
diff -b test-suite.vcx.filters.files test-suite.ref.files > test-suite.vcx.filters.diff

# ...process...
./tools/check_filelists_diffs.py
result=$?

# ...and cleanup
rm -f ql.*.files test-suite.*.files test-suite-cpp.ref.files
rm -f ql.*.diff test-suite.*.diff

exit $result
