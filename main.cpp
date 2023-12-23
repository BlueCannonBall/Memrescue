#include "manager.hpp"
#include <chrono>
#include <cstddef>
#include <cstdlib>
#include <string>
#include <sys/mman.h>
#include <sys/swap.h>
#include <thread>
#include <unistd.h>
#include <vector>

int main() {
    mlockall(MCL_CURRENT);
    mlockall(MCL_FUTURE);
    ResourceManager manager;
    manager.adjust_oom_score(getpid(), -1000);
    manager.write_message("membomber", "membomber is running");

    for (;;) {
        CPUStats start_cpu_stats = manager.cpu_stats();
        std::this_thread::sleep_for(std::chrono::milliseconds(250));
        CPUStats end_cpu_stats = manager.cpu_stats();
        MemoryInfo memory_info = manager.info();

        double iowait_percentage = (double) (end_cpu_stats.iowait_time - start_cpu_stats.iowait_time) / (end_cpu_stats.total_time - start_cpu_stats.total_time);
        if (iowait_percentage > 0.125) {
            if (memory_info.available < memory_info.total / 15) {
                manager.write_message("membomber", "Killing highest-OOM-score process...");
                pid_t result;
                if ((result = ResourceManager::oom_kill()) == -1) {
                    manager.write_message("membomber", "Error: Failed to kill process " + std::to_string(result));
                } else {
                    manager.write_message("membomber", "Killed process " + std::to_string(result));
                }
                continue;
            }

            if (memory_info.cached) {
                manager.write_message("membomber", "Dropping caches...");
                manager.drop_caches();
                manager.write_message("membomber", "Caches dropped");
                continue;
            }
        }
        if (iowait_percentage > 0.25) {
            if (memory_info.total_swap &&
                memory_info.free_swap < memory_info.total_swap &&
                memory_info.available > memory_info.total_swap - memory_info.free_swap) {
                manager.write_message("membomber", "Clearing swap...");
                std::vector<SwapInfo> swap_info = manager.swap_info();
                for (const auto& entry : swap_info) {
                    swapoff(entry.filename.c_str());
                }
                for (const auto& entry : swap_info) {
                    swapon(entry.filename.c_str(), (entry.priority << SWAP_FLAG_PRIO_SHIFT) & SWAP_FLAG_PRIO_MASK);
                }
                manager.write_message("membomber", "Swap cleared");
            }
        }
    }

    return 0;
}
