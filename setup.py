from setuptools import setup

# Available at setup time due to pyproject.toml
from pybind11.setup_helpers import Pybind11Extension, build_ext
from pybind11 import get_cmake_dir,get_include

import sys

__version__ = "0.0.1"

# The main interface is through Pybind11Extension.
# * You can add cxx_std=11/14/17, and then build_ext can be removed.
# * You can set include_pybind11=false to add the include directory yourself,
#   say from a submodule.
#
# Note:
#   Sort input source files if you glob sources to ensure bit-for-bit
#   reproducible builds (https://github.com/pybind/python_example/pull/53)

ext_modules = [
    Pybind11Extension(
    "fast_graph",
    ['graph_compiled/main.cpp', 'graph_compiled/graph.cpp'],
    include_dirs=[
      get_include(False),
      get_include(True ),
    ],
    language='c++',

    # Example: passing in the version to the compiled code
    define_macros = [('VERSION_INFO', __version__)],
    ),
]

setup(
    name="fast_graph",
    version=__version__,
    author="Team_mopsi",
    url="mandatory but useless",
    description="homemade compiled graph library",
    ext_modules=ext_modules,
    # Currently, build_ext only provides an optional "highest supported C++
    # level" feature, but in the future it may provide more features.
    cmdclass={"build_ext": build_ext},
    zip_safe=False,
)
