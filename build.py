from conan.packager import ConanMultiPackager
import os


username = os.getenv("CONAN_USERNAME", "jgsogo")


if __name__ == "__main__":
    builder = ConanMultiPackager(username=username, 
                                 stable_branch_pattern='1.9',
                                 visual_versions=["12", "14", "15"],
                                 gcc_versions=["4.8", "4.9", "5.2", "5.3"])
    builder.add_common_builds()
    builder.run()
