#!/bin/sh
grep -r "quantlib\.org/.*license\.s*html" * | grep -v '/\.svn/' | grep -v '^Binary file .* matches' | fgrep -v "<https://www.quantlib.org/license.shtml>"
