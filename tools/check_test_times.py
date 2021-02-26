import re
import sys
import xml.etree.ElementTree as ET

regex1 = re.compile(".*test_case\(&(.*?)__(.*?)\)")
regex2 = re.compile(".*ext__bind\(&(.*?)__(.*?)__.*\)\)")
regex3 = re.compile(".*{_?([a-zA-Z]*Test)__([a-zA-Z]*?)\(")


def parse_simple(t):
    # test names get mangled - here we extract class and method name
    m = regex1.match(t.attrib["name"])
    if m:
        return m.groups()


def parse_bound(t):
    # same, for cases inside a bind
    m = regex2.match(t.attrib["name"])
    if m:
        return m.groups()


def parse_lambda(t):
    # same, for cases inside a lambda
    m = regex3.match(t.attrib["name"])
    if m:
        return m.groups()


def extract_test_info(filename):
    root = ET.parse(filename).getroot()
    tests = root.findall("testcase")
    tests = tests[1:-1]  # exclude start_timer / stop_timer
    for t in tests:
        cls, method = parse_simple(t) or parse_bound(t) or parse_lambda(t)
        time = float(t.attrib["time"])
        yield (cls, method, time)


def check(filename, max_time, action):
    errors = False
    for cls, method, time in extract_test_info(filename):
        if time > max_time:
            print(f"{cls}.{method} ({time:.2f} s) should be {action}")
            errors = True
    return errors


errors = check("faster.xml", 3.0, "moved out of the 'Faster' section")
errors |= check("fast.xml", 8.0, "moved out of the 'Fast' section")
errors |= check("all.xml", 30.0, "kept under 30 seconds")

if errors:
    sys.exit(1)
