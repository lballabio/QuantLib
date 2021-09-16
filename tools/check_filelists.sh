#!/bin/bash

# get reference lists of existing files (done with find)

find ql -name '*.[hc]pp' -or -name '*.[hc]' \
| grep -v 'ql/config\.hpp' | sort > ql.ref.files
find test-suite -name '*.[hc]pp' \
| grep -v 'quantlibbenchmark' | grep -v '/main\.cpp' \
| sort > test-suite.ref.files

# get list of distributed files from packaged tarball

make dist

mkdir dist-check
mv QuantLib-*.tar.gz dist-check
cd dist-check
tar xfz QuantLib-*.tar.gz
rm QuantLib-*.tar.gz
cd QuantLib-*

find ql -name '*.[hc]pp' -or -name '*.[hc]' \
| grep -v 'ql/config\.hpp' | sort > ../../ql.dist.files
find test-suite -name '*.[hc]pp' \
| grep -v 'quantlibbenchmark' | grep -v '/main\.cpp' \
| sort > ../../test-suite.dist.files

cd ../..
rm -rf dist-check

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

# reference files above align only to autotools build system
grep -o -E '[a-zA-Z0-9_/\.]*\.[hc]p*' ql/CMakeLists.txt \
| grep -v '/cmake/' | grep -v 'ql/config\.hpp' | sed -e 's|/ql/||' | sed -e 's|^|ql/|' | sort > ql.cmake.files

grep -o -E '[a-zA-Z0-9_/\.]*\.[hc]pp' test-suite/CMakeLists.txt \
| grep -v 'quantlibbenchmark' | grep -v 'main\.cpp' \
| sed -e 's|^|test-suite/|' | sort -u > test-suite.cmake.files

# write out differences...

diff -b ql.dist.files ql.ref.files > ql.dist.diff
diff -b test-suite.dist.files test-suite.ref.files > test-suite.dist.diff

diff -b ql.cmake.files ql.ref.files > ql.cmake.diff
diff -b test-suite.cmake.files test-suite.ref.files > test-suite.cmake.diff

diff -b ql.vcx.files ql.ref.files > ql.vcx.diff
diff -b test-suite.vcx.files test-suite.ref.files > test-suite.vcx.diff

diff -b ql.vcx.filters.files ql.ref.files > ql.vcx.filters.diff
diff -b test-suite.vcx.filters.files test-suite.ref.files > test-suite.vcx.filters.diff

# ...process...
./tools/check_filelists_diffs.py
result=$?

# ...and cleanup
rm -f ql.*.files test-suite.*.files
rm -f ql.*.diff test-suite.*.diff

exit $result
