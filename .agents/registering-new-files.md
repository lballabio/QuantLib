# Registering New Files in the Build Systems

Read this whenever you add, rename, or remove a `.hpp` or `.cpp` file.

Every new `.hpp` or `.cpp` file must be added to **all three** build systems. CI
enforces consistency via `tools/check_filelists.sh` (run by
`.github/workflows/filelists.yml`), which diffs the actual source tree against
every build-system file list and fails on any mismatch.

## CMake

Add entries to the explicit lists in `ql/CMakeLists.txt` (paths relative to
`ql/`):

- Headers → `set(QL_HEADERS ...)` — e.g. `instruments/myinstrument.hpp`
- Sources → `set(QL_SOURCES ...)` — e.g. `instruments/myinstrument.cpp`

For test files, use `test-suite/CMakeLists.txt`:

- Test sources → `set(QL_TEST_SOURCES ...)`
- Test headers → `set(QL_TEST_HEADERS ...)`

## Autotools

Each subdirectory under `ql/` has its own `Makefile.am`. Add to the one matching
your file's directory:

- Headers → `this_include_HEADERS` variable
- Sources → `cpp_files` variable

For test files, use `test-suite/Makefile.am`:

- Test sources → `QL_TEST_SRCS`
- Test headers → `QL_TEST_HDRS`

## Visual Studio

Add entries to `QuantLib.vcxproj` (paths use backslashes, relative to the
repository root):

- Headers → `<ClInclude Include="ql\path\myfile.hpp" />`
- Sources → `<ClCompile Include="ql\path\myfile.cpp" />`

And add matching entries in `QuantLib.vcxproj.filters` so files appear in the
correct Solution Explorer folder:

```xml
<ClInclude Include="ql\instruments\myinstrument.hpp">
  <Filter>instruments</Filter>
</ClInclude>
<ClCompile Include="ql\instruments\myinstrument.cpp">
  <Filter>instruments</Filter>
</ClCompile>
```

For test files, update `test-suite/testsuite.vcxproj` and
`test-suite/testsuite.vcxproj.filters` analogously (test paths are relative to
`test-suite/`, and the filter is typically `Source Files` or `Header Files`).

## Local Verification

Run the same check CI uses:

```bash
./autogen.sh && ./configure && ./tools/check_filelists.sh
```
