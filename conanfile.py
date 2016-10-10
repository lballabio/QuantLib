from conans import ConanFile, CMake


def get_version():
    return "1.8"


class QuantLibConan(ConanFile):
    name = "QuantLib"
    settings = "os", "compiler", "build_type", "arch"
    version = get_version()
    exports = "*"
    requires = "Boost/1.60.0@lasote/stable"
    generators = "cmake"

    def build(self):
        cmake = CMake(self.settings)
        self.run('cmake %s %s' % (self.conanfile_directory, cmake.command_line))
        self.run("cmake --build . %s" % cmake.build_config)

    def package(self):
        self.copy("cmake/FindQuantlib.cmake", ".", ".")
        self.copy("cmake/quantlib.cmake", ".", ".")
        
        self.copy(pattern="*", dst="ql", src="ql")
        self.copy("*.lib", dst="lib", src="lib")
        self.copy("*.a", dst="lib", src="lib")
        self.copy("*.bin", dst="bin", src="bin")
        self.copy("*.so", dst="bin", src="bin")

    def package_info(self):
        self.cpp_info.libs = ["QuantLib"]

