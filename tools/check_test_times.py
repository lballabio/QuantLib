import re
import sys
import xml.etree.ElementTree as ET
from collections import namedtuple, defaultdict

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


Case = namedtuple("Case", ["files", "times"])


def collect():
    data = defaultdict(lambda: Case([], []))
    for filename in ["faster.xml", "fast.xml", "all.xml"]:
        for cls, method, time in extract_test_info(filename):
            c = data[(cls, method)]
            c.files.append(filename)
            c.times.append(time)
    return data


def check(data, condition, action):
    errors = False
    for cls, method in data:
        c = data[(cls, method)]
        if condition(c):
            print(
                f"{cls}.{method} ({min(c.times):.2f} s / {max(c.times):.2f} s) {action}"
            )
            errors = True
    return errors


data = collect()

errors = check(
    data,
    lambda c: "faster.xml" in c.files and min(c.times) > 3.0,
    "should be moved out of the 'Faster' section",
)
errors |= check(
    data,
    lambda c: "fast.xml" in c.files and min(c.times) > 8.0,
    "should be moved out of the 'Fast' section",
)
errors |= check(
    data, lambda c: max(c.times) > 30.0, "should be kept reliably under 30 seconds"
)

errors |= check(
    data,
    lambda c: "faster.xml" not in c.files and max(c.times) < 1.0,
    "could be moved into the 'Faster' section",
)
errors |= check(
    data,
    lambda c: "fast.xml" not in c.files and max(c.times) < 3.0,
    "could be moved into the 'Fast' section",
)


if errors:
    sys.exit(1)
