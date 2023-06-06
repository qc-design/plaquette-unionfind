import os
import platform
import sys
import subprocess
import shutil
from pathlib import Path
from setuptools import Extension, setup, find_packages
from setuptools.command.build_ext import build_ext

# Convert distutils Windows platform specifiers to CMake -A arguments
PLAT_TO_CMAKE = {
    "win32": "Win32",
    "win-amd64": "x64",
    "win-arm32": "ARM",
    "win-arm64": "ARM64",
}


# A CMakeExtension needs a sourcedir instead of a file list.
# The name must be the _single_ output extension from the CMake build.
# If you need multiple extensions, see scikit-build.
class CMakeExtension(Extension):
    def __init__(self, name: str, sourcedir: str = "") -> None:
        Extension.__init__(self, name, sources=[])
        self.sourcedir = Path(sourcedir).absolute()


class CMakeBuild(build_ext):
        user_options = build_ext.user_options + [
            ("define=", "D", "Define variables for CMake"),
            ("verbosity", "V", "Increase CMake build verbosity"),
            ("arch=", "A", "Define backend targetted architecture"),
        ]

        def initialize_options(self):
            super().initialize_options()
            self.define = None
            self.arch = None
            self.verbosity = ""

        def finalize_options(self):
            # Parse the custom CMake options and store them in a new attribute
            defines = [] if self.define is None else self.define.split(";")
            self.cmake_defines = [f"-D{define}" for define in defines]
            if self.verbosity != "":
                self.verbosity = "--verbose"

            super().finalize_options()

    
        def build_extension(self, ext: CMakeExtension):
            extdir = str(Path(self.get_ext_fullpath(ext.name)).parent.absolute())
            debug = int(os.environ.get("DEBUG", 0)) if self.debug is None else self.debug
            cfg = "Debug" if debug else "Release"
            ninja_path = str(shutil.which("ninja"))

            # Set Python_EXECUTABLE instead if you use PYBIND11_FINDPYTHON
            configure_args = [
                f"-DCMAKE_CXX_FLAGS=-fno-lto",
                f"-DCMAKE_LIBRARY_OUTPUT_DIRECTORY={extdir}",
                f"-DPYTHON_EXECUTABLE={sys.executable}",
                f"-DPLAQUETTE_UNIONFIND_BUILD_BINDINGS=On",
                f"-DCMAKE_BUILD_TYPE={cfg}",  # not used on MSVC, but no harm
                *(self.cmake_defines),
            ]

            if platform.system() == "Windows":
                configure_args += [
                    "-T clangcl",
                ]
            else:
                configure_args += [
                    "-GNinja",
                    f"-DCMAKE_MAKE_PROGRAM={ninja_path}",
                ]

            build_args = []
            if os.getenv("ARCH") and not self.arch:
                self.arch = os.getenv("ARCH")

            # Add more platform dependent options
            if platform.system() == "Darwin":
                #To support ARM64
                if os.getenv('ARCHS') == "arm64":
                    configure_args += ["-DCMAKE_CXX_COMPILER_TARGET=arm64-apple-macos11",
                                    "-DCMAKE_SYSTEM_NAME=Darwin",
                                    "-DCMAKE_SYSTEM_PROCESSOR=ARM64"]
                else: # X64 arch
                    llvmpath = subprocess.check_output(["brew", "--prefix", "llvm"]).decode().strip()
                    configure_args += [
                            f"-DCMAKE_CXX_COMPILER={llvmpath}/bin/clang++",
                            f"-DCMAKE_LINKER={llvmpath}/bin/lld",
                    ] # Use clang instead of appleclang
                # Disable OpenMP in M1 Macs
                if os.environ.get("USE_OMP"):
                    configure_args += []
                else:
                    configure_args += ["-DPLAQUETTE_ENABLE_OPENMP=OFF"]
            elif platform.system() == "Windows":
                configure_args += ["-DPLAQUETTE_ENABLE_OPENMP=OFF"] # only build with Clang under Windows
            else:
                if platform.system() != "Linux":
                    raise RuntimeError(f"Unsupported '{platform.system()}' platform")

            if not Path(self.build_temp).exists():
                os.makedirs(self.build_temp)

            subprocess.check_call(
                ["cmake", str(ext.sourcedir)] + configure_args, cwd=self.build_temp
            )
            subprocess.check_call(["cmake", "--build", "."] + build_args, cwd=self.build_temp)

# Main metadata is in pyproject.toml
setup(
    ext_modules=[CMakeExtension("plaquette_unionfind_bindings")],
    cmdclass={"build_ext": CMakeBuild},
)
