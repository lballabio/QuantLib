from conan.packager import ConanMultiPackager
import os


username = os.getenv("CONAN_USERNAME", "jgsogo")


if __name__ == "__main__":
    builder = ConanMultiPackager(username=username, 
                                 stable_branch_pattern='1.9',
                                 visual_runtimes=["MT", "MTd"])
    builder.add_common_builds()
    builder.run()
