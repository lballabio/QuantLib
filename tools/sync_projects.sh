#!/bin/bash

# execute this script from the root of an uncompressed QuantLib tarball

# get reference lists of distributed files (done with find; this is
# why this script should be run from an uncompressed tarball created
# with 'make dist', not from a working copy.)

find ql -name '*.[hc]pp' -or -name '*.[hc]' \
| grep -v 'ql/config\.hpp' | grep -v 'ql/ad\.hpp' | sort > ql.ref.files
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
| sort > ql.vcx.filters

grep -o -E 'Include=".*\.[hc]p*"' test-suite/testsuite.vcxproj.filters \
| awk -F'"' '{ print $2 }' | sed -e 's|\\|/|g' | sed -e 's|^./||' \
| sed -e 's|^|test-suite/|' | sort > test-suite.vcx.filters

# same with CMakelists

grep -o -E '[a-zA-Z0-9_/\.]*\.[hc]p*' ql/CMakeLists.txt \
| sed -e 's|^|ql/|' | sort > ql.cmake.files

grep -o -E '[a-zA-Z0-9_/\.]*\.cpp' test-suite/CMakeLists.txt \
| grep -v 'quantlibbenchmark' \
| sed -e 's|^|test-suite/|' | sort -u > test-suite.cmake.files

# write out differences...

echo 'CMake:'
diff -b ql.cmake.files ql.ref.files
diff -b test-suite.cmake.files test-suite-cpp.ref.files
echo

echo 'Visual Studio:'
echo 'project:'
diff -b ql.vcx.files ql.ref.files
diff -b test-suite.vcx.files test-suite.ref.files
echo 'filters:'
diff -b ql.vcx.filters ql.ref.files
diff -b test-suite.vcx.filters test-suite.ref.files

# ...and cleanup
rm -f ql.ref.files test-suite.ref.files test-suite-cpp.ref.files
rm -f ql.cmake.files test-suite.cmake.files
rm -f ql.vcx.files test-suite.vcx.files
rm -f ql.vcx.filters test-suite.vcx.filters

