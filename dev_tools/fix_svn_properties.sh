#!/bin/sh
# text files
find . -name '*.[hc]pp' -o -name '*.[hc]' \
    -o -name '*.html' -o -name '*.css' -o -name '*.docs' -o -name '*.doxy' \
    -o -name '*.sh' -o -name '*.bat' -o -name '*.el' -o -name '*.py' \
    -o -name '*.txt' -o -name '*.TXT' -o -name '*.tex' \
 | xargs -n 1 svn propset svn:eol-style native
# these should have UNIX eol, even when extracted from a .zip
find . -name '*.a[cm]' -o -name '*.m4' -o -name '*.in' \
 | xargs -n 1 svn propset svn:eol-style LF
# these should have Windows eol, even when extracted from a .tar.gz
find . -name '*.dev' -o  -name '*.nsi' -o  -name '*.sln' -o  -name '*.vcproj' \
 | xargs -n 1 svn propset svn:eol-style CRLF
