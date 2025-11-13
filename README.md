# Tape Storage Index Strategy Simulator

A tape storage system simulator designed to compare the performance of different index strategies in tape-based storage environments. By emulating the physical characteristics of tape devices (e.g., block I/O, seek time), this tool evaluates the access efficiency of three index strategies.

## Features

- Emulates tape device physical properties (block size, read/write speed, seek time)
- Implements three index strategies:
  - No Index (linear search)
  - Fixed Interval Index
  - Hierarchical Index (two-level index structure)
- Supports performance benchmarking for index build time and query response time
- Provides detailed performance comparison reports, including average access time and speedup analysis

## Build & Installation

### Prerequisites

- C++17 compatible compiler (GCC 7+ or Clang 5+)
- CMake 3.10+
- (macOS) Xcode Command Line Tools or Homebrew-installed GCC

### Build Steps

1. Clone or download the project code
2. Compile the project:

```bash
# Create build directory
mkdir build && cd build

# Generate build files
cmake ..

# Compile
make
```

3. (Optional) Install to system:

```bash
sudo make install
```

## Usage

### Basic Simulation

Run the program directly to execute the default simulation with 10,000 blocks and 1,000 queries:

```bash
./tape_simulator
```

The program will output performance comparisons of the three index strategies, including:
- Index build time
- Average access time
- Total seek operations
- Total access time
- Speedup ratio relative to the no-index strategy

### Benchmark Mode

Run the benchmarking mode:

```bash
./tape_simulator benchmark
```

The benchmark will output performance data in CSV format, including index build time (milliseconds) and query time (milliseconds) for each strategy.

### Run Tests with CTest

```bash
# In the build directory
ctest -V
```

## Configuration Parameters

Adjust the following simulation parameters in `main.cpp`:
- `BLOCK_COUNT`: Number of tape blocks (default: 10,000)
- `QUERY_COUNT`: Number of queries (default: 1,000)
- `BLOCK_SIZE`: Block size in bytes (default: 4096)
- Tape device parameters (read/write speed, seek time, etc.)

## Performance Metrics

- **Index Build Time**: Time required to create the index structure
- **Average Access Time**: Average response time per query
- **Total Seeks**: Total number of seek operations across all queries
- **Speedup Ratio**: Performance ratio of the no-index strategy vs. other strategies (higher values indicate greater performance improvement)

## Example Output

```
Starting tape storage simulation with 10000 blocks and 1000 queries...

Simulation Results:

Strategy                       Index Build Time (s)    Avg Access Time (s)     Total Seeks     Total Access Time (s)
--------------------------------------------------------------------------------------------------------------
No Index                       0.000000                0.523478                1000            523.478123
Fixed Interval Index           0.045678                0.012345                1000            12.345678
Hierarchical Index             0.067890                0.008765                1000            8.765432

Performance Analysis:
Fixed Interval Index is 42.40x faster than no index strategy
Hierarchical Index is 59.72x faster than no index strategy
```

## License

This project is licensed under the MIT License. See the LICENSE file for details.