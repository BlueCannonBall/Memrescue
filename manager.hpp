#include <cstddef>
#include <fstream>
#include <string>
#include <sys/types.h>
#include <utility>
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
    std::string filename;
    std::string type;
    size_t size;
    size_t usage;
    int priority;
};

class ResourceManager {
protected:
    std::ifstream cpu_stats_file;
    std::ifstream info_file;
    std::ifstream swap_info_file;
    std::ofstream drop_caches_file;
    std::ofstream kmsg_file;

public:
    ResourceManager():
        cpu_stats_file("/proc/stat"),
        info_file("/proc/meminfo"),
        swap_info_file("/proc/swaps"),
        drop_caches_file("/proc/sys/vm/drop_caches"),
        kmsg_file("/dev/kmsg") {}

    CPUStats cpu_stats();
    MemoryInfo info();
    std::vector<SwapInfo> swap_info();

    static void adjust_oom_score(pid_t pid, int adjustment);
    static pid_t oom_kill();

    inline void drop_caches() {
        drop_caches_file << 3;
    }

    inline void write_message(const std::string& heading, const std::string& message) {
        kmsg_file << heading << ": " << message << std::endl;
    }
};
