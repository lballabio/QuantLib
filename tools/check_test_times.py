import re
import sys
import xml.etree.ElementTree as ET

regex1 = re.compile(".*test_case\(&(.*?)__(.*?)\)")
regex2 = re.compile(".*ext__bind\(&(.*?)__(.*?)__.*\)\)")


def parse_simple(t):
    # test names get mangled - here we extract class and method name
    m = regex1.match(t.attrib["name"])
    if m:
        return m.groups()


def parse_bound(t):
    # same, for the case of tests split in multiple runs
    m = regex2.match(t.attrib["name"])
    if m:
        return m.groups()


def extract_test_info(filename):
    root = ET.parse(filename).getroot()
    tests = root.findall("testcase")
    for t in tests:
        cls, method = parse_simple(t) or parse_bound(t) or (None, None)
        if method:
            time = float(t.attrib["time"])
            yield (cls, method, time)


def check(filename, max_time, action):
    errors = False
    for cls, method, time in extract_test_info(filename):
        if time > max_time:
            print(f"{cls}.{method} ({time:.2f} s) should be {action}")
            errors = True
    return errors


errors = check("faster.xml", 4.0, "moved out of the 'Faster' section")
errors |= check("fast.xml", 10.0, "moved out of the 'Fast' section")
errors |= check("all.xml", 30.0, "made to run under 30 seconds")

if errors:
    sys.exit(1)
