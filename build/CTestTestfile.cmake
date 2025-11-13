# CMake generated Testfile for 
# Source directory: /Users/jeffwhynot/activity/CLionProjects/fakeTape
# Build directory: /Users/jeffwhynot/activity/CLionProjects/fakeTape/build
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(tape_benchmark "/Users/jeffwhynot/activity/CLionProjects/fakeTape/build/tape_simulator" "benchmark")
set_tests_properties(tape_benchmark PROPERTIES  PASS_REGULAR_EXPRESSION "Benchmark Results" TIMEOUT "60" _BACKTRACE_TRIPLES "/Users/jeffwhynot/activity/CLionProjects/fakeTape/CMakeLists.txt;22;add_test;/Users/jeffwhynot/activity/CLionProjects/fakeTape/CMakeLists.txt;0;")
