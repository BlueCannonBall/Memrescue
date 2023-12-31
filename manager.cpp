#include "manager.hpp"
#include <cctype>
#include <limits>
#include <signal.h>
#include <stdexcept>
#include <sys/swap.h>
#include <unistd.h>
#include <utility>

CPUStats ResourceManager::cpu_stats() {
    CPUStats ret;

    cpu_stats_file.seekg(0);
    cpu_stats_file.ignore(std::numeric_limits<std::streamsize>::max(), ' ');
    cpu_stats_file >>
        ret.user_time >>
        ret.nice_time >>
        ret.system_time >>
        ret.idle_time >>
        ret.iowait_time >>
        ret.irq_time >>
        ret.softirq_time >>
        ret.stolen_time >>
        ret.guest_time >>
        ret.guest_nice_time;

    ret.total_time = ret.user_time +
                     ret.nice_time +
                     ret.system_time +
                     ret.idle_time +
                     ret.iowait_time +
                     ret.irq_time +
                     ret.softirq_time +
                     ret.stolen_time +
                     ret.guest_time +
                     ret.guest_nice_time;

    if (!cpu_stats_file && !cpu_stats_file.eof()) {
        throw std::runtime_error("Failed to read /proc/stat");
    }
    return ret;
}

MemoryInfo ResourceManager::info() {
    MemoryInfo ret;

    info_file.clear();
    info_file.seekg(0);
    info_file.ignore(std::numeric_limits<std::streamsize>::max(), ':');
    info_file >> ret.total;
    ret.total *= 1000;

    info_file.ignore(std::numeric_limits<std::streamsize>::max(), ':');
    info_file >> ret.free;
    ret.free *= 1000;

    info_file.ignore(std::numeric_limits<std::streamsize>::max(), ':');
    info_file >> ret.available;
    ret.available *= 1000;

    info_file.ignore(std::numeric_limits<std::streamsize>::max(), ':');
    info_file.ignore(std::numeric_limits<std::streamsize>::max(), ':');
    info_file >> ret.cached;
    ret.cached *= 1000;

    info_file.ignore(std::numeric_limits<std::streamsize>::max(), ':');
    info_file >> ret.cached_swap;
    ret.cached_swap *= 1000;

    for (size_t i = 0; i < 9 && info_file.good(); ++i) {
        info_file.ignore(std::numeric_limits<std::streamsize>::max(), ':');
    }
    info_file >> ret.total_swap;
    ret.total_swap *= 1000;

    info_file.ignore(std::numeric_limits<std::streamsize>::max(), ':');
    info_file >> ret.free_swap;
    ret.free_swap *= 1000;

    if (!info_file && !info_file.eof()) {
        throw std::runtime_error("Failed to read /proc/meminfo");
    }
    return ret;
}

std::vector<SwapInfo> ResourceManager::swap_info() {
    std::vector<SwapInfo> ret;

    swap_info_file.clear();
    swap_info_file.seekg(0);
    swap_info_file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    for (SwapInfo entry; !swap_info_file.eof(); ret.push_back(std::move(entry))) {
        swap_info_file >> entry.path >> entry.type >> entry.size >> entry.usage >> entry.priority;
    }

    if (!swap_info_file && !swap_info_file.eof()) {
        throw std::runtime_error("Failed to read /proc/swaps");
    }
    return ret;
}

void ResourceManager::drop_caches() {
    log("Dropping caches...");
    sync();
    if (drop_caches_file << 3 << std::endl) {
        log("Caches dropped");
    } else {
        log("Failed to drop caches");
    }

    clears.cache = std::chrono::steady_clock::now();
}

void ResourceManager::clear_swap() {
    log("Clearing swap...");
    auto info = swap_info();
    for (const auto& entry : info) {
        swapoff(entry.path.c_str());
    }
    for (const auto& entry : info) {
        swapon(entry.path.c_str(), (entry.priority << SWAP_FLAG_PRIO_SHIFT) & SWAP_FLAG_PRIO_MASK);
    }
    log("Swap cleared");

    clears.swap = std::chrono::steady_clock::now();
}

void ResourceManager::kill_process(pid_t pid) {
    pid_t result;
    log("Killing process " + std::to_string(pid) + "...");
    if ((result = kill(pid, SIGKILL)) == -1) {
        log("Error: Failed to kill process " + std::to_string(pid));
    } else {
        log("Killed process " + std::to_string(pid));
    }
}

void ResourceManager::adjust_oom_score(pid_t pid, int adjustment) {
    std::ofstream file("/proc/" + std::to_string(pid) + "/oom_score_adj");
    if (!(file << adjustment << std::endl)) {
        throw std::runtime_error("Failed to write to /proc/" + std::to_string(pid) + "/oom_score_adj");
    }
}

void ResourceManager::adjust_niceness(int adjustment) {
    if (nice(adjustment) == -1) {
        throw std::runtime_error("Failed to adjust niceness");
    }
}

std::vector<ProcessInfo> ResourceManager::get_processes() {
    std::vector<ProcessInfo> ret;

    for (const auto& entry : std::filesystem::directory_iterator("/proc")) {
        if (entry.is_directory()) {
            std::string filename = entry.path().filename();
            for (char c : filename) {
                if (!isdigit(c)) {
                    goto next_entry;
                }
            }

            size_t private_clean_memory;
            size_t private_dirty_memory;
            {
                std::ifstream smaps_rollup_file(entry.path() / "smaps_rollup");

                smaps_rollup_file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                for (size_t i = 0; i < 8; ++i) {
                    smaps_rollup_file.ignore(std::numeric_limits<std::streamsize>::max(), ':');
                }
                smaps_rollup_file >> private_clean_memory;
                private_clean_memory *= 1000;

                smaps_rollup_file.ignore(std::numeric_limits<std::streamsize>::max(), ':');
                smaps_rollup_file >> private_dirty_memory;
                private_dirty_memory *= 1000;

                if (!smaps_rollup_file && !smaps_rollup_file.eof()) {
                    continue;
                }
            }

            int oom_score;
            {
                std::ifstream oom_score_file(entry.path() / "oom_score");
                if (!(oom_score_file >> oom_score) && !oom_score_file.eof()) {
                    continue;
                }
            }

            ret.push_back({
                std::stoi(filename),
                private_clean_memory + private_dirty_memory,
                oom_score,
            });
        }
    next_entry:;
    }

    return ret;
}
