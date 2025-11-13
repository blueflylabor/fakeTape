cmake_minimum_required(VERSION 3.10)
project(tape_simulator VERSION 1.0.0 LANGUAGES CXX)

enable_testing()

# 指定实际的 GCC 14 路径（根据你的系统输出）
set(CMAKE_C_COMPILER "/opt/homebrew/bin/gcc-14")
set(CMAKE_CXX_COMPILER "/opt/homebrew/bin/g++-14")

# 强制 C++17 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# GCC 编译选项
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic -O2")

# 生成可执行文件
add_executable(tape_simulator main.cpp)

# 测试配置
add_test(
    NAME tape_benchmark
    COMMAND tape_simulator benchmark
)
set_tests_properties(tape_benchmark PROPERTIES 
    TIMEOUT 60
    PASS_REGULAR_EXPRESSION "Benchmark Results"
)