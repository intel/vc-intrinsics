# -*- Python -*-

import lit.formats
import lit.util

from lit.llvm import llvm_config
from lit.llvm.subst import ToolSubst
from lit.llvm.subst import FindTool

# Configuration file for the 'lit' test runner.

# name: The name of this test suite.
config.name = 'vc-intrinsics'

# testFormat: The test format to use to interpret tests.
config.test_format = lit.formats.ShTest(not llvm_config.use_lit_shell)

# suffixes: A list of file extensions to treat as test files.
config.suffixes = ['.ll']

# excludes: A list of directories  and files to exclude from the testsuite.
config.excludes = ['CMakeLists.txt', 'Plugin']

used_llvm = "llvm{}".format(config.llvm_version_major)
config.available_features = [used_llvm]

# test_source_root: The root path where tests are located.
config.test_source_root = os.path.dirname(__file__)

# test_exec_root: The root path where tests should be run.
config.test_exec_root = os.path.join(config.test_run_dir, 'test_output')

llvm_config.use_default_substitutions()

config.substitutions.append(('%PATH%', config.environment['PATH']))

tool_dirs = [config.llvm_tools_dir]

# Add extra args for opt to remove boilerplate from tests.
args_load_plugin = ['-load', config.vc_intrinsics_plugin]
tools = [ToolSubst('opt', extra_args=args_load_plugin)]

llvm_config.add_tool_substitutions(tools, tool_dirs)
