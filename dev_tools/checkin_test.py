#!/usr/bin/python

# Copyright (C) 2001, 2002, 2003 Vladimir Prus. Permission to copy, use, modify, sell and
# distribute this software is granted, provided this copyright notice appears
# in all copies and modified versions are clearly marked as such. This software
# is provided "as is" without express or implied warranty, and with no claim as
# to is suitability for any purpose.

import os;
import re;
import string;
import getopt;
import sys;
import string;

no_tabs = 0
no_endings = 0
no_trailing = 0
no_last = 0

def get_files_list(current_path=""):
    try:
        entries_file = open("CVS/Entries")
    except IOError:
        pass

    result = []
    entries = parse_entries_file(entries_file)
    for e in entries:
        if e[1] == "file":
            result.append(current_path + e[0])
        elif e[1] == "dir":
            cwd = os.path.abspath(os.getcwd())
            try:
                os.chdir(e[0])
                result.extend(get_files_list(current_path + e[0] + "/"))
                os.chdir(cwd)
            except OSError:
                pass
        else:
            assert(0)
    def is_text(f):
        i = string.rfind(f, ".")
        return i != -1 and f[i:] in [".cpp", ".hpp", ".py", ".jam"]
    return filter(is_text, result)


entries_re = re.compile("(.*?)/(.+?)/");
def parse_entries_file(entries_file):
    def mk(x):
        m = entries_re.match(x)
        if m is None:
            return None
        if m.group(1) == "":
            type = "file"
        elif m.group(1) == "D":
            type = "dir"
        else:
            print x
            print m.group(1)
            assert(0)
        return (m.group(2), type)

    return filter(lambda x: x, map(mk, entries_file.readlines()))


check_file_re = re.compile("(?s).+[ \t\r]+\n")
def check_file(name):
    complaints = []
    f = open(name, "rb")
    s = f.read()
    if not no_tabs and '\t' in s:
        complaints.append("tabs")
    if not no_endings and '\r' in s:
        complaints.append("line endings")
    if not no_trailing and check_file_re.match(s) is not None:
        complaints.append("trailing spaces")
    if not no_last and len(s) != 0 and s[-1] != '\n':
        complaints.append("last line")
    f.close()
    if len(complaints) != 0:
        return name +  " : " + string.join(complaints, ",")
    else:
        return None

def main():

    global no_tabs, no_endings, no_trailing, no_last
    try:
        opts, args = getopt.getopt(sys.argv[1:], "", ["no-tabs", "no-endings", "no-trailing", "no-last"])
    except getopt.error, x:
        print x
        print "usage: checkin_test.py [--no-tabs] [--no-endings] [--no-trailing] [--no-last]"
        sys.exit(1)

    for i in opts:
        if i[0] == "--no-tabs":
            no_tabs = 1
        elif i[0] == "--no-endings":
            no_endings = 1
        elif i[0] == "--no-trailing":
            no_trailing = 1
        elif i[0] == "--no-last":
            no_last = 1
        else:
            assert(0)


    files = get_files_list()
    complaints = filter(lambda x: x, map(check_file, files))
    if len(complaints) != 0:
        print "There are problem, see 'checking_problems' file for details."
        f = open("checkin_problems", "w")
        for c in complaints:
            f.write(c + "\n")
    else:
        print "Ok."

main()


