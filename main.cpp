#include <cstdint>
#include <vector>
#include <string>
#include <unordered_map>
#include <memory>
#include <stdexcept>
#include <sstream>
#include <algorithm>
#include <random>
#include <iostream>
#include <iomanip>
#include <numeric>
#include <cmath>
#include <chrono>  // 用于基准测试计时

// 关键修复：在main函数前声明run_benchmarks
int run_benchmarks();

// 磁带块结构
struct TapeBlock {
    uint64_t block_id;       // 块ID
    std::vector<uint8_t> data; // 块数据
    bool is_index_block;     // 是否为索引块
    
    TapeBlock(uint64_t id, const std::vector<uint8_t>& d, bool is_index = false)
        : block_id(id), data(d), is_index_block(is_index) {}
};

// 磁带设备模拟器
class TapeDevice {
private:
    // 成员变量顺序调整：与构造函数初始化列表顺序一致（修复警告）
    size_t block_size;              // 块大小(字节)
    double read_speed;              // 读取速度(字节/秒)
    double write_speed;             // 写入速度(字节/秒)
    double seek_time_per_block;     // 块间寻道时间(秒)
    size_t current_position;        // 当前位置（移到最后，与初始化顺序一致）
    std::vector<TapeBlock> blocks;  // 磁带块集合
    
public:
    TapeDevice(size_t block_size = 4096, 
              double read_speed = 1024 * 1024,  // 1MB/s
              double write_speed = 512 * 1024,   // 512KB/s
              double seek_time = 0.01);          // 10ms per block
    
    // 写入块
    double write_block(const TapeBlock& block);
    
    // 读取当前块
    std::pair<TapeBlock, double> read_current_block();
    
    // 移动到指定块
    double seek_to_block(size_t block_index);
    
    // 向前移动n个块
    double move_forward(size_t n = 1);
    
    // 向后移动n个块
    double move_backward(size_t n = 1);
    
    // 获取当前位置
    size_t get_current_position() const;
    
    // 获取块数量
    size_t get_block_count() const;
    
    // 获取指定位置的块
    const TapeBlock& get_block(size_t index) const;
    
    // 重置磁带
    void reset();
    
    // 获取块大小
    size_t get_block_size() const { return block_size; }
};

// 索引策略基类
class IndexStrategy {
public:
    virtual ~IndexStrategy() = default;
    
    // 构建索引
    virtual double build_index(TapeDevice& tape) = 0;
    
    // 查找数据块
    virtual std::pair<size_t, double> find_block(TapeDevice& tape, uint64_t data_id) = 0;
    
    // 获取策略名称
    virtual std::string get_name() const = 0;
    
    // 获取索引统计信息
    virtual std::string get_stats() const = 0;
};

// 无索引策略
class NoIndexStrategy : public IndexStrategy {
public:
    // 修复警告：使用[[maybe_unused]]标记未使用参数
    double build_index([[maybe_unused]] TapeDevice& tape) override;
    std::pair<size_t, double> find_block(TapeDevice& tape, uint64_t data_id) override;
    std::string get_name() const override;
    std::string get_stats() const override;
};

// 固定间隔索引策略
class FixedIntervalIndexStrategy : public IndexStrategy {
private:
    size_t interval;  // 索引间隔
    std::unordered_map<uint64_t, size_t> index_map;  // 数据ID到块位置的映射
    
public:
    FixedIntervalIndexStrategy(size_t interval = 10);
    
    double build_index(TapeDevice& tape) override;
    std::pair<size_t, double> find_block(TapeDevice& tape, uint64_t data_id) override;
    std::string get_name() const override;
    std::string get_stats() const override;
};

// 分层索引策略
class HierarchicalIndexStrategy : public IndexStrategy {
private:
    size_t level1_interval;  // 一级索引间隔
    size_t level2_interval;  // 二级索引间隔
    std::unordered_map<uint64_t, std::pair<size_t, size_t>> index_map;  // 数据ID到(一级块, 二级块)的映射
    
public:
    HierarchicalIndexStrategy(size_t level1 = 100, size_t level2 = 10);
    
    double build_index(TapeDevice& tape) override;
    std::pair<size_t, double> find_block(TapeDevice& tape, uint64_t data_id) override;
    std::string get_name() const override;
    std::string get_stats() const override;
};

// 索引策略工厂
class IndexStrategyFactory {
public:
    static std::unique_ptr<IndexStrategy> create_strategy(const std::string& type, 
                                                         size_t param1 = 0, 
                                                         size_t param2 = 0);
};

// 模拟结果结构
struct SimulationResult {
    std::string strategy_name;
    double index_build_time;      // 索引构建时间
    double average_access_time;   // 平均访问时间
    size_t total_seeks;           // 总寻道次数
    size_t total_blocks_accessed; // 总访问块数
    double total_access_time;     // 总访问时间
};

// 磁带模拟器
class TapeSimulator {
private:
    TapeDevice tape_device;
    std::unique_ptr<IndexStrategy> current_strategy;
    std::vector<SimulationResult> results;
    
    // 生成测试数据
    void generate_test_data(size_t block_count, double data_size_ratio = 0.5);
    
public:
    TapeSimulator(size_t block_size = 4096);
    
    // 设置索引策略
    void set_strategy(std::unique_ptr<IndexStrategy> strategy);
    
    // 运行模拟
    SimulationResult run_simulation(size_t block_count, 
                                   const std::vector<uint64_t>& query_ids,
                                   bool generate_new_data = true);
    
    // 运行对比模拟
    std::vector<SimulationResult> run_comparison(size_t block_count,
                                                const std::vector<uint64_t>& query_ids,
                                                const std::vector<std::string>& strategy_types);
    
    // 打印结果
    void print_results() const;

    // 基准测试：测量索引构建时间（返回毫秒）
    double benchmark_index_build(size_t block_count) {
        generate_test_data(block_count);
        auto start = std::chrono::high_resolution_clock::now();
        current_strategy->build_index(tape_device);
        auto end = std::chrono::high_resolution_clock::now();
        return std::chrono::duration<double, std::milli>(end - start).count();
    }

    // 基准测试：测量查询性能（返回毫秒）
    double benchmark_queries(const std::vector<uint64_t>& query_ids) {
        auto start = std::chrono::high_resolution_clock::now();
        for (uint64_t id : query_ids) {
            current_strategy->find_block(tape_device, id);
        }
        auto end = std::chrono::high_resolution_clock::now();
        return std::chrono::duration<double, std::milli>(end - start).count();
    }
};

// TapeDevice 实现
TapeDevice::TapeDevice(size_t block_size, double read_speed, double write_speed, double seek_time)
    : block_size(block_size), read_speed(read_speed), write_speed(write_speed),
      seek_time_per_block(seek_time), current_position(0) {}  // 初始化顺序与成员声明顺序一致

double TapeDevice::write_block(const TapeBlock& block) {
    blocks.push_back(block);
    double time = block.data.size() / write_speed;
    return time;
}

std::pair<TapeBlock, double> TapeDevice::read_current_block() {
    if (current_position >= blocks.size()) {
        throw std::out_of_range("Position out of range");
    }
    
    const TapeBlock& block = blocks[current_position];
    double time = block.data.size() / read_speed;
    return {block, time};
}

double TapeDevice::seek_to_block(size_t block_index) {
    if (block_index >= blocks.size()) {
        throw std::out_of_range("Block index out of range");
    }
    
    size_t distance = std::abs(static_cast<int64_t>(block_index) - static_cast<int64_t>(current_position));
    double time = distance * seek_time_per_block;
    current_position = block_index;
    return time;
}

double TapeDevice::move_forward(size_t n) {
    size_t new_pos = current_position + n;
    if (new_pos >= blocks.size()) {
        new_pos = blocks.size() - 1;
    }
    
    return seek_to_block(new_pos);
}

double TapeDevice::move_backward(size_t n) {
    size_t new_pos = (current_position >= n) ? current_position - n : 0;
    return seek_to_block(new_pos);
}

size_t TapeDevice::get_current_position() const {
    return current_position;
}

size_t TapeDevice::get_block_count() const {
    return blocks.size();
}

const TapeBlock& TapeDevice::get_block(size_t index) const {
    if (index >= blocks.size()) {
        throw std::out_of_range("Block index out of range");
    }
    return blocks[index];
}

void TapeDevice::reset() {
    blocks.clear();
    current_position = 0;
}

// NoIndexStrategy 实现
double NoIndexStrategy::build_index(TapeDevice& tape) {
    return 0.0; // 无索引，构建时间为0
}

std::pair<size_t, double> NoIndexStrategy::find_block(TapeDevice& tape, uint64_t data_id) {
    double time = 0.0;
    size_t original_pos = tape.get_current_position();
    
    // 从当前位置开始搜索
    for (size_t i = 0; i < tape.get_block_count(); ++i) {
        size_t pos = (original_pos + i) % tape.get_block_count();
        time += tape.seek_to_block(pos);
        
        auto [block, read_time] = tape.read_current_block();
        time += read_time;
        
        if (!block.is_index_block && block.block_id == data_id) {
            return {pos, time};
        }
    }
    
    // 未找到
    return {std::string::npos, time};
}

std::string NoIndexStrategy::get_name() const {
    return "No Index";
}

std::string NoIndexStrategy::get_stats() const {
    return "No index used";
}

// FixedIntervalIndexStrategy 实现
FixedIntervalIndexStrategy::FixedIntervalIndexStrategy(size_t interval) : interval(interval) {}

double FixedIntervalIndexStrategy::build_index(TapeDevice& tape) {
    index_map.clear();
    double time = 0.0;
    size_t original_pos = tape.get_current_position();
    
    // 回到起始位置
    time += tape.seek_to_block(0);
    
    // 创建索引
    for (size_t i = 0; i < tape.get_block_count(); ++i) {
        // 读取当前块
        auto [block, read_time] = tape.read_current_block();
        time += read_time;
        
        // 如果是数据块，添加到索引
        if (!block.is_index_block) {
            index_map[block.block_id] = i;
            
            // 每隔interval个数据块创建一个索引块
            if (index_map.size() % interval == 0) {
                std::vector<uint8_t> index_data;
                TapeBlock index_block(block.block_id + 1000000, index_data, true);
                time += tape.write_block(index_block);
                time += tape.move_forward(1);
            }
        }
        
        // 移动到下一个块
        if (i < tape.get_block_count() - 1) {
            time += tape.move_forward(1);
        }
    }
    
    // 回到原始位置
    time += tape.seek_to_block(original_pos);
    
    return time;
}

std::pair<size_t, double> FixedIntervalIndexStrategy::find_block(TapeDevice& tape, uint64_t data_id) {
    double time = 0.0;
    // 修复警告：移除未使用的变量original_pos
    // size_t original_pos = tape.get_current_position();
    
    auto it = index_map.find(data_id);
    if (it == index_map.end()) {
        return {std::string::npos, 0.0};
    }
    
    size_t target_pos = it->second;
    time += tape.seek_to_block(target_pos);
    
    auto [block, read_time] = tape.read_current_block();
    time += read_time;
    
    if (block.block_id != data_id) {
        return {std::string::npos, time};
    }
    
    return {target_pos, time};
}

std::string FixedIntervalIndexStrategy::get_name() const {
    return "Fixed Interval Index";
}

std::string FixedIntervalIndexStrategy::get_stats() const {
    std::stringstream ss;
    ss << "Interval: " << interval << ", Index entries: " << index_map.size();
    return ss.str();
}

// HierarchicalIndexStrategy 实现
HierarchicalIndexStrategy::HierarchicalIndexStrategy(size_t level1, size_t level2)
    : level1_interval(level1), level2_interval(level2) {}

double HierarchicalIndexStrategy::build_index(TapeDevice& tape) {
    index_map.clear();
    double time = 0.0;
    size_t original_pos = tape.get_current_position();
    size_t block_count = tape.get_block_count();
    
    time += tape.seek_to_block(0);
    
    std::vector<std::pair<uint64_t, size_t>> data_blocks;
    for (size_t i = 0; i < block_count; ++i) {
        auto [block, read_time] = tape.read_current_block();
        time += read_time;
        
        if (!block.is_index_block) {
            data_blocks.emplace_back(block.block_id, i);
        }
        
        if (i < block_count - 1) {
            time += tape.move_forward(1);
        }
    }
    
    std::vector<uint8_t> level2_data;
    TapeBlock level2_block(1000000, level2_data, true);
    time += tape.write_block(level2_block);
    
    std::vector<uint8_t> level1_data;
    TapeBlock level1_block(2000000, level1_data, true);
    time += tape.write_block(level1_block);
    
    for (size_t i = 0; i < data_blocks.size(); ++i) {
        auto [id, pos] = data_blocks[i];
        size_t level2_idx = i / level2_interval;
        size_t level1_idx = level2_idx / level1_interval;
        index_map[id] = {level1_idx, level2_idx};
    }
    
    time += tape.seek_to_block(original_pos);
    
    return time;
}

std::pair<size_t, double> HierarchicalIndexStrategy::find_block(TapeDevice& tape, uint64_t data_id) {
    double time = 0.0;
    // 修复警告：移除未使用的变量original_pos
    // size_t original_pos = tape.get_current_position();
    
    auto it = index_map.find(data_id);
    if (it == index_map.end()) {
        return {std::string::npos, 0.0};
    }
    
    auto [level1_idx, level2_idx] = it->second;
    
    size_t level1_pos = tape.get_block_count() - 2;
    time += tape.seek_to_block(level1_pos);
    time += tape.read_current_block().second;
    
    size_t level2_pos = tape.get_block_count() - 1;
    time += tape.seek_to_block(level2_pos);
    time += tape.read_current_block().second;
    
    size_t target_pos = (level1_idx * level1_interval + level2_idx) * level2_interval;
    if (target_pos >= tape.get_block_count() - 2) {
        target_pos = tape.get_block_count() - 3;
    }
    
    time += tape.seek_to_block(target_pos);
    
    auto [block, read_time] = tape.read_current_block();
    time += read_time;
    
    if (block.block_id != data_id) {
        return {std::string::npos, time};
    }
    
    return {target_pos, time};
}

std::string HierarchicalIndexStrategy::get_name() const {
    return "Hierarchical Index";
}

std::string HierarchicalIndexStrategy::get_stats() const {
    std::stringstream ss;
    ss << "Level1 interval: " << level1_interval 
       << ", Level2 interval: " << level2_interval
       << ", Index entries: " << index_map.size();
    return ss.str();
}

// IndexStrategyFactory 实现
std::unique_ptr<IndexStrategy> IndexStrategyFactory::create_strategy(const std::string& type, 
                                                                    size_t param1, 
                                                                    size_t param2) {
    if (type == "none") {
        return std::make_unique<NoIndexStrategy>();
    } else if (type == "fixed") {
        return std::make_unique<FixedIntervalIndexStrategy>(param1 > 0 ? param1 : 10);
    } else if (type == "hierarchical") {
        return std::make_unique<HierarchicalIndexStrategy>(
            param1 > 0 ? param1 : 100, 
            param2 > 0 ? param2 : 10
        );
    } else {
        throw std::invalid_argument("Unknown index strategy: " + type);
    }
}

// TapeSimulator 实现
TapeSimulator::TapeSimulator(size_t block_size) : tape_device(block_size) {}

void TapeSimulator::set_strategy(std::unique_ptr<IndexStrategy> strategy) {
    current_strategy = std::move(strategy);
}

void TapeSimulator::generate_test_data(size_t block_count, double data_size_ratio) {
    tape_device.reset();
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<uint64_t> id_dist(1, 1000000);
    std::uniform_int_distribution<size_t> size_dist(1, static_cast<size_t>(tape_device.get_block_size() * data_size_ratio));
    
    for (size_t i = 0; i < block_count; ++i) {
        uint64_t id = id_dist(gen);
        size_t data_size = size_dist(gen);
        
        std::vector<uint8_t> data(data_size);
        std::generate(data.begin(), data.end(), [&]() { return static_cast<uint8_t>(gen() % 256); });
        
        tape_device.write_block(TapeBlock(id, data, false));
    }
}

SimulationResult TapeSimulator::run_simulation(size_t block_count, 
                                              const std::vector<uint64_t>& query_ids,
                                              bool generate_new_data) {
    if (!current_strategy) {
        throw std::runtime_error("No index strategy set");
    }
    
    if (generate_new_data) {
        generate_test_data(block_count);
    }
    
    SimulationResult result;
    result.strategy_name = current_strategy->get_name();
    result.index_build_time = current_strategy->build_index(tape_device);
    
    result.total_access_time = 0.0;
    result.total_seeks = 0;
    result.total_blocks_accessed = 0;
    
    for (uint64_t id : query_ids) {
        auto [pos, time] = current_strategy->find_block(tape_device, id);
        result.total_access_time += time;
        result.total_blocks_accessed++;
        
        if (time > 0) {
            result.total_seeks++;
        }
    }
    
    if (result.total_blocks_accessed > 0) {
        result.average_access_time = result.total_access_time / result.total_blocks_accessed;
    }
    
    results.push_back(result);
    return result;
}

std::vector<SimulationResult> TapeSimulator::run_comparison(size_t block_count,
                                                          const std::vector<uint64_t>& query_ids,
                                                          const std::vector<std::string>& strategy_types) {
    std::vector<SimulationResult> comparison_results;
    generate_test_data(block_count);
    
    for (const auto& type : strategy_types) {
        set_strategy(IndexStrategyFactory::create_strategy(type));
        comparison_results.push_back(run_simulation(block_count, query_ids, false));
    }
    
    return comparison_results;
}

void TapeSimulator::print_results() const {
    std::cout << std::left << std::setw(30) << "Strategy"
              << std::setw(20) << "Index Build Time (s)"
              << std::setw(20) << "Avg Access Time (s)"
              << std::setw(15) << "Total Seeks"
              << std::setw(20) << "Total Access Time (s)" << std::endl;
    
    std::cout << std::string(110, '-') << std::endl;
    
    for (const auto& res : results) {
        std::cout << std::left << std::setw(30) << res.strategy_name
                  << std::setw(20) << std::fixed << std::setprecision(6) << res.index_build_time
                  << std::setw(20) << std::fixed << std::setprecision(6) << res.average_access_time
                  << std::setw(15) << res.total_seeks
                  << std::setw(20) << std::fixed << std::setprecision(6) << res.total_access_time << std::endl;
    }
}

// 主程序（默认执行模拟）
int main(int argc, char**argv) {
    // 如果有命令行参数 "benchmark"，则执行基准测试模式
    if (argc > 1 && std::string(argv[1]) == "benchmark") {
        return run_benchmarks();
    }

    // 否则执行常规模拟
    try {
        const size_t BLOCK_COUNT = 10000;
        const size_t QUERY_COUNT = 1000;
        const size_t BLOCK_SIZE = 4096;
        
        TapeSimulator simulator(BLOCK_SIZE);
        
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<uint64_t> id_dist(1, 1000000);
        
        std::vector<uint64_t> queries;
        for (size_t i = 0; i < QUERY_COUNT; ++i) {
            queries.push_back(id_dist(gen));
        }
        
        std::vector<std::string> strategies = {"none", "fixed", "hierarchical"};
        
        std::cout << "Starting tape storage simulation with " << BLOCK_COUNT 
                  << " blocks and " << QUERY_COUNT << " queries..." << std::endl;
        
        auto results = simulator.run_comparison(BLOCK_COUNT, queries, strategies);
        
        std::cout << "\nSimulation Results:\n" << std::endl;
        simulator.print_results();
        
        std::cout << "\nPerformance Analysis:" << std::endl;
        const auto& no_index = results[0];
        
        for (size_t i = 1; i < results.size(); ++i) {
            double speedup = no_index.average_access_time / results[i].average_access_time;
            std::cout << results[i].strategy_name << " is " 
                      << std::fixed << std::setprecision(2) << speedup 
                      << "x faster than no index strategy" << std::endl;
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}

// 基准测试入口（供CTest调用）
int run_benchmarks() {
    try {
        const size_t BLOCK_COUNT = 10000;
        const size_t QUERY_COUNT = 1000;
        const size_t BLOCK_SIZE = 4096;
        
        TapeSimulator simulator(BLOCK_SIZE);
        std::vector<std::string> strategies = {"none", "fixed", "hierarchical"};
        
        // 生成测试查询
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<uint64_t> id_dist(1, 1000000);
        std::vector<uint64_t> queries;
        for (size_t i = 0; i < QUERY_COUNT; ++i) {
            queries.push_back(id_dist(gen));
        }

        // 输出基准测试结果（CTest会捕获这些输出）
        std::cout << "Benchmark Results (ms):\n";
        std::cout << "Strategy,IndexBuildTime,QueryTime\n";

        for (const auto& strategy : strategies) {
            simulator.set_strategy(IndexStrategyFactory::create_strategy(strategy));
            
            // 测量索引构建时间
            double build_time = simulator.benchmark_index_build(BLOCK_COUNT);
            
            // 测量查询时间
            double query_time = simulator.benchmark_queries(queries);
            
            // 输出CSV格式，便于CTest解析
            std::cout << strategy << "," << build_time << "," << query_time << "\n";
        }

        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Benchmark failed: " << e.what() << std::endl;
        return 1;
    }
}