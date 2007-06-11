#!/bin/sh
grep -r "quantlib\.org/.*license\.s*html" * | grep -v '/\.svn/' | grep -v '^Binary file .* matches' | fgrep -v "<http://quantlib.org/license.shtml>"
