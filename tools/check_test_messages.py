"""Check that each test case declares a BOOST_TEST_MESSAGE."""

import re
import sys
from pathlib import Path
from typing import Iterator, NamedTuple


TEST_CASE = re.compile(
    r"\b(BOOST_AUTO_TEST_CASE_TEMPLATE|BOOST_FIXTURE_TEST_CASE_TEMPLATE|"
    r"BOOST_AUTO_TEST_CASE|BOOST_FIXTURE_TEST_CASE|"
    r"BOOST_DATA_TEST_CASE_F|BOOST_DATA_TEST_CASE)\b"
)
RAW_STRING_START = re.compile(r'R"([^ ()\\\t\r\n]{0,16})\(')
MESSAGE = re.compile(r"\bBOOST_TEST_MESSAGE\s*\(")


class TestCase(NamedTuple):
    """A test-case declaration and its body."""

    name: str
    line: int
    body: str


def mask_non_code(text: str) -> str:
    """Replace comments and literals with spaces while preserving offsets."""
    masked = list(text)
    i = 0
    while i < len(text):
        if text.startswith("//", i):
            end = text.find("\n", i)
            end = len(text) if end == -1 else end
            for j in range(i, end):
                masked[j] = " "
            i = end
        elif text.startswith("/*", i):
            end = text.find("*/", i + 2)
            end = len(text) if end == -1 else end + 2
            for j in range(i, end):
                if text[j] != "\n":
                    masked[j] = " "
            i = end
        else:
            raw_start = RAW_STRING_START.match(text, i)
            if raw_start:
                terminator = ")" + raw_start.group(1) + '"'
                end = text.find(terminator, raw_start.end())
                end = len(text) if end == -1 else end + len(terminator)
                for j in range(i, end):
                    if text[j] != "\n":
                        masked[j] = " "
                i = end
            elif (
                text[i] == "'"
                and i > 0
                and i + 1 < len(text)
                and text[i - 1] in "0123456789abcdefABCDEF"
                and text[i + 1] in "0123456789abcdefABCDEF"
            ):
                i += 1
            elif text[i] in ('"', "'"):
                quote = text[i]
                j = i + 1
                while j < len(text):
                    if text[j] == "\\":
                        j += 2
                    elif text[j] == quote:
                        j += 1
                        break
                    else:
                        j += 1
                for k in range(i, min(j, len(text))):
                    if text[k] != "\n":
                        masked[k] = " "
                i = j
            else:
                i += 1
    return "".join(masked)


def matching_delimiter(source: str, opening: int, left: str, right: str) -> int:
    """Return the matching closing delimiter or -1 if it is unbalanced."""
    depth = 0
    for i in range(opening, len(source)):
        if source[i] == left:
            depth += 1
        elif source[i] == right:
            depth -= 1
            if depth == 0:
                return i
    return -1


def test_cases(path: Path, text: str, code: str) -> Iterator[TestCase]:
    """Yield parsed test cases with function-style bodies."""
    for declaration in TEST_CASE.finditer(code):
        opening = code.find("(", declaration.end())
        if opening == -1:
            continue
        closing = matching_delimiter(code, opening, "(", ")")
        if closing == -1:
            raise ValueError(f"{path}:{text.count(chr(10), 0, declaration.start()) + 1}: "
                             "unbalanced test-case declaration")

        body_opening = closing + 1
        while body_opening < len(code) and code[body_opening].isspace():
            body_opening += 1
        if body_opening == len(code) or code[body_opening] != "{":
            continue

        body_closing = matching_delimiter(code, body_opening, "{", "}")
        if body_closing == -1:
            line = text.count("\n", 0, declaration.start()) + 1
            raise ValueError(f"{path}:{line}: unbalanced test-case body")

        arguments = text[opening + 1 : closing]
        name = arguments.split(",", 1)[0].strip()
        line = text.count("\n", 0, declaration.start()) + 1
        yield TestCase(name, line, code[body_opening + 1 : body_closing])


def main() -> int:
    """Check all C++ test cases and report declarations missing a message."""
    errors = 0
    checked = 0
    for path in sorted(Path("test-suite").rglob("*.cpp")):
        text = path.read_text(encoding="utf-8")
        code = mask_non_code(text)
        try:
            cases = test_cases(path, text, code)
            for case in cases:
                checked += 1
                if not MESSAGE.search(case.body):
                    message = f"{case.name} is missing BOOST_TEST_MESSAGE"
                    print(f"::error file={path},line={case.line}::{message}")
                    errors += 1
        except ValueError as error:
            print(f"::error file={path}::{error}", file=sys.stderr)
            errors += 1

    if errors:
        print(f"Found {errors} issue(s) while checking {checked} test case(s).")
        return 1

    print(f"All {checked} test cases contain BOOST_TEST_MESSAGE.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
