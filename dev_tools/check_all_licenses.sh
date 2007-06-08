#!/bin/sh
grep -r "/license\.s*html" * | grep -v '/\.svn/' | grep -v "<http://quantlib.org/license\.shtml>"
