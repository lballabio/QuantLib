from conans import ConanFile, CMake
import os
import re

# This easily allows to copy the package in other user or channel
channel = os.getenv("CONAN_CHANNEL", "testing")
username = os.getenv("CONAN_USERNAME", "demo")


def get_version():
    version_hpp = os.path.join(os.path.dirname(__file__), '..', 'ql', 'version.hpp')
    version_pattern = re.compile(r'^#define QL_VERSION "(?P<version>[\d\.]+)"$')
    with open(version_hpp) as f:
        for line in f:
            m = version_pattern.match(line.strip())
            if m:
                 return m.group('version')
    raise Exception("Cannot get version from {!r}".format(version_hpp))
	

class HelloReuseConan(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    requires = "QuantLib/%s@%s/%s" % (get_version(), username, channel)
    generators = "cmake"

    def build(self):
        cmake = CMake(self.settings)
        self.run('cmake "%s" %s' % (self.conanfile_directory, cmake.command_line))
        self.run("cmake --build . %s" % cmake.build_config)

    def test(self):
        # equal to ./bin/greet, but portable win: .\bin\greet
        self.run(os.sep.join([".","bin", "greet"]))


