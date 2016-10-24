import os
import re
from conans import ConanFile, CMake



def get_version():
    version_hpp = os.path.join(os.path.dirname(__file__), 'ql', 'version.hpp')
    version_pattern = re.compile(r'^#define QL_VERSION "(?P<version>[\d\.]+)"$')
    with open(version_hpp) as f:
        for line in f:
            m = version_pattern.match(line.strip())
            if m:
                 return m.group('version')
    raise Exception("Cannot get version from {!r}".format(version_hpp))


class QuantLibConan(ConanFile):
    name = "QuantLib"
    settings = "os", "compiler", "build_type", "arch"
    version = get_version()
    exports = "CMakeLists.txt", "cmake/*", "ql/*", "Examples/*", "test-suite/*", "*.TXT", "README.md"
    requires = "Boost/1.60.0@lasote/stable"
    generators = "cmake"
    license = 'http://quantlib.org/license.shtml'
    url = 'http://quantlib.org/index.shtml'

    options = {"examples": [True, False], "tests": [True, False]}    
    default_options = "examples=False", "tests=False"

    def build(self):
        cmake = CMake(self.settings)
        build_examples = "-DBUILD_EXAMPLES:BOOL={}".format("ON" if self.options.examples else "OFF")
        build_tests = "-DBUILD_TESTS:BOOL={}".format("ON" if self.options.tests else "OFF")
        self.run('cmake {} {} {} {}'.format(self.conanfile_directory, cmake.command_line, build_examples, build_tests))
        self.run("cmake --build . {}".format(cmake.build_config))

    def package(self):
        self.copy(pattern="*.cmake", src="cmake", dst=".")
        
        self.copy(pattern="*", dst="include/ql", src="ql")
        self.copy("*.lib", dst="lib", src="lib")
        self.copy("*.a", dst="lib", src="lib")
        self.copy("*.bin", dst="bin", src="bin")
        self.copy("*.so", dst="bin", src="bin")

    def package_info(self):
        self.cpp_info.libs = ["QuantLib"]


