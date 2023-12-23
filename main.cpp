#include "manager.hpp"
#include <chrono>
#include <cstddef>
#include <iostream>
#include <string>
#include <sys/mman.h>
#include <sys/swap.h>
#include <thread>
#include <vector>

int main() {
    mlockall(MCL_CURRENT);
    mlockall(MCL_FUTURE);
    ResourceManager manager;

    std::cout << "membomber is running!" << std::endl;
    for (;;) {
        CPUStats start_cpu_stats = manager.cpu_stats();
        std::this_thread::sleep_for(std::chrono::milliseconds(250));
        CPUStats end_cpu_stats = manager.cpu_stats();

        double iowait_percentage = (double) (end_cpu_stats.iowait_time - start_cpu_stats.iowait_time) / (end_cpu_stats.total_time - start_cpu_stats.total_time);
        if (iowait_percentage > 0.125) {
            MemoryInfo info = manager.info();
            if (info.available < info.total / 10) {
                std::cout << "Killing highest-OOM-score process..." << std::endl;
                pid_t result;
                if ((result = ResourceManager::oom_kill()) == -1) {
                    std::cerr << "Failed to kill process " << result << '!' << std::endl;
                } else {
                    std::cout << "Killed process " << result << '!' << std::endl;
                }
            }
        } else if (iowait_percentage > 0.25) {
            std::cout << "Dropping caches..." << std::endl;
            manager.drop_caches();
            std::cout << "Caches dropped!" << std::endl;

            MemoryInfo info = manager.info();
            if (info.available > info.total_swap - info.free_swap) {
                std::cout << "Clearing swap..." << std::endl;
                std::vector<SwapInfo> swap_info = manager.swap_info();
                for (const auto& entry : swap_info) {
                    swapoff(entry.filename.c_str());
                }
                for (const auto& entry : swap_info) {
                    swapon(entry.filename.c_str(), (entry.priority << SWAP_FLAG_PRIO_SHIFT) & SWAP_FLAG_PRIO_MASK);
                }
                std::cout << "Swap cleared!" << std::endl;
            }
        }
    }

    return 0;
}
