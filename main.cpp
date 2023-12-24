#include "./config.hpp"
#include "manager.hpp"
#include <chrono>
#include <ctime>
#include <string>
#include <sys/mman.h>
#include <sys/swap.h>
#include <thread>
#include <unistd.h>

int main() {
    mlockall(MCL_CURRENT);
    mlockall(MCL_FUTURE);
    ResourceManager manager;
    manager.adjust_oom_score(getpid(), -1000);
    manager.log("membomber is running");

    for (;;) {
        CPUStats start_cpu_stats = manager.cpu_stats();
        std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_TIME));
        CPUStats end_cpu_stats = manager.cpu_stats();
        MemoryInfo memory_info = manager.info();
        time_t now = std::time(nullptr);

        double iowait_percentage = (double) (end_cpu_stats.iowait_time - start_cpu_stats.iowait_time) / (end_cpu_stats.total_time - start_cpu_stats.total_time);
        double memory_percentage = 1 - (double) memory_info.available / memory_info.total;

#ifdef DEBUG
        manager.log("{ iowait_percentage: " + std::to_string(iowait_percentage) + ", memory_percentage: " + std::to_string(memory_percentage) + " }");
#endif

        // RAM free
        if (memory_percentage > MAX_MEMORY_USAGE) {
            if (now - manager.clears.kill > KILL_TIMEOUT) {
                auto highest = manager.get_hightest();
                manager.kill_proc(highest.pid);
            }
            if (now - manager.clears.cache > CACHE_TIMEOUT) {
                if (memory_info.cached) {
                    manager.drop_caches();
                }
            }
        }

        // Swap IO clear
        else if (iowait_percentage > IOWAIT_THRES && now - manager.clears.swap > SWAP_TIMEOUT) {
            if (memory_info.total_swap &&
                memory_info.free_swap < memory_info.total_swap &&
                memory_info.available > memory_info.total_swap - memory_info.free_swap) {
                manager.clear_swap();
            }
        }
    }

    return 0;
}
