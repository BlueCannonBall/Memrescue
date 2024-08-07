#pragma once

#include "config.hpp"
#include <chrono>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <string_view>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

struct CPUStats {
    long user_time;
    long nice_time;
    long system_time;
    long idle_time;
    long iowait_time;
    long irq_time;
    long softirq_time;
    long stolen_time;
    long guest_time;
    long guest_nice_time;
    long total_time;
};

struct MemoryInfo {
    size_t total;
    size_t free;
    size_t available;
    size_t cached;
    size_t cached_swap;
    size_t total_swap;
    size_t free_swap;
};

struct SwapInfo {
    std::filesystem::path path;
    std::string type;
    size_t size;
    size_t usage;
    int priority;
};

struct ProcessInfo {
    pid_t pid;
    size_t uss; // Unique set size
    int oom_score;
};

class ResourceManager {
protected:
    std::ifstream cpu_stats_file;
    std::ifstream info_file;
    std::ifstream swap_info_file;
    std::ofstream drop_caches_file;
    std::ofstream log_file;

public:
    struct {
        std::chrono::steady_clock::time_point cache;
        std::chrono::steady_clock::time_point swap;
    } clears;

    ResourceManager():
        cpu_stats_file("/proc/stat"),
        info_file("/proc/meminfo"),
        swap_info_file("/proc/swaps"),
        drop_caches_file("/proc/sys/vm/drop_caches"),
        log_file(LOG_FILE) {}

    CPUStats cpu_stats();
    MemoryInfo info();
    std::vector<SwapInfo> swap_info();

    void drop_caches(void);
    void clear_swap(void);
    void kill_process(pid_t pid);

    static void adjust_oom_score(pid_t pid, int adjustment);
    static void adjust_niceness(int adjustment);
    static std::vector<ProcessInfo> get_processes();

    inline void log(std::string_view heading, std::string_view message) {
#ifdef DEBUG
        std::cout << heading << ": " << message << std::endl;
#else
        log_file << heading << ": " << message << std::endl;
#endif
    }

    inline void log(std::string_view message) {
        log("memrescue", message);
    }
};
