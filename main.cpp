#include "config.hpp"
#include "manager.hpp"
#include <algorithm>
#include <chrono>
#include <sys/mman.h>
#include <thread>

int main() {
    // Prevent Memrescue's memory from being paged to the swap area
    mlockall(MCL_CURRENT | MCL_FUTURE | MCL_ONFAULT);

    ResourceManager manager;
    manager.adjust_oom_score(getpid(), -1000);
    manager.adjust_niceness(-20);
    manager.log("Memrescue is running");

    for (;;) {
        CPUStats start_cpu_stats = manager.cpu_stats();
        std::this_thread::sleep_for(std::chrono::milliseconds(IOWAIT_SLEEP_TIME));
        CPUStats end_cpu_stats = manager.cpu_stats();
        auto now = std::chrono::steady_clock::now();

        double iowait_percentage = (double) (end_cpu_stats.iowait_time - start_cpu_stats.iowait_time) / (end_cpu_stats.total_time - start_cpu_stats.total_time);
#ifdef DEBUG
        manager.log("iowait_percentage: " + std::to_string(iowait_percentage));
#endif
        if (iowait_percentage > IOWAIT_THRES) {
            MemoryInfo memory_info = manager.info();
            double memory_percentage = 1. - (double) memory_info.available / memory_info.total;
#ifdef DEBUG
            manager.log("memory_percentage: " + std::to_string(memory_percentage));
#endif
            if (memory_percentage > MAX_MEMORY_USAGE) {
                if (now - manager.clears.cache > std::chrono::milliseconds(CACHE_TIMEOUT) && memory_info.cached) {
                    manager.drop_caches();
                }

                // Update memory info
                std::this_thread::sleep_for(std::chrono::milliseconds(MEMORY_SLEEP_TIME));
                memory_info = manager.info();
                memory_percentage = 1. - (double) memory_info.available / memory_info.total;

                auto processes = manager.get_processes();
                std::sort(processes.begin(), processes.end(), [](const auto& a, const auto& b) {
                    if (a.oom_score != b.oom_score) {
                        return a.oom_score < b.oom_score;
                    } else {
                        return a.uss < b.uss;
                    }
                });
                while (memory_percentage > MAX_MEMORY_USAGE && !processes.empty()) {
                    auto highest_process = std::max_element(processes.end() - std::min<size_t>(OOM_KILLER_SAMPLE_SIZE, processes.size()), processes.end(), [](const auto& a, const auto& b) {
                        return a.uss < b.uss;
                    });
                    manager.kill_process(highest_process->pid);
                    processes.erase(highest_process);

                    // Update memory info
                    std::this_thread::sleep_for(std::chrono::milliseconds(MEMORY_SLEEP_TIME));
                    memory_info = manager.info();
                    memory_percentage = 1. - (double) memory_info.available / memory_info.total;
                }
            } else if (now - manager.clears.swap > std::chrono::milliseconds(SWAP_TIMEOUT) &&
                       memory_info.total_swap &&
                       memory_info.free_swap < memory_info.total_swap &&
                       memory_info.available > memory_info.total_swap - memory_info.free_swap) {
                manager.clear_swap();
            }
        }
    }

    return 0;
}
