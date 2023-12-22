#include <chrono>
#include <fstream>
#include <iostream>
#include <limits>
#include <string>
#include <sys/mman.h>
#include <sys/swap.h>
#include <thread>
#include <utility>
#include <vector>

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

class MemoryManager {
protected:
    std::ifstream info_file;
    std::ifstream swap_info_file;
    std::ofstream drop_caches_file;
    std::ofstream sysrq_trigger_file;

public:
    MemoryManager():
        info_file("/proc/meminfo"),
        swap_info_file("/proc/swaps"),
        drop_caches_file("/proc/sys/vm/drop_caches"),
        sysrq_trigger_file("/proc/sysrq-trigger") {}

    MemoryInfo info() {
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

        return ret;
    }

    std::vector<SwapInfo> swap_info() {
        std::vector<SwapInfo> ret;

        swap_info_file.clear();
        swap_info_file.seekg(0);
        swap_info_file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        for (SwapInfo entry; ;) {
            swap_info_file >> entry.filename >> entry.type >> entry.size >> entry.usage >> entry.priority;
            if (swap_info_file.good()) {
                ret.push_back(std::move(entry));
            } else {
                break;
            }
        }

        return ret;
    }

    void drop_caches() {
        drop_caches_file << 3;
    }

    void trigger_oom_killer() {
        sysrq_trigger_file << 'f';
    }
};

int main() {
    mlockall(MCL_FUTURE);
    MemoryManager manager;

    for (std::chrono::steady_clock::time_point start_time, end_time; ;) {
        start_time = std::chrono::steady_clock::now();
        std::this_thread::sleep_for(std::chrono::seconds(1));
        end_time = std::chrono::steady_clock::now();
        if (end_time - start_time - std::chrono::seconds(1) > std::chrono::seconds(1)) {
            manager.drop_caches(); // Immediately drop caches if the system hangs

            MemoryInfo info = manager.info();
            if (info.available < info.total / 20) {
                manager.trigger_oom_killer();
                if (info.total_swap - info.free_swap < info.available) { // If all the swap can fit into 
                    std::vector<SwapInfo> swap_info = manager.swap_info();
                    for (const auto& entry : swap_info) {
                        swapoff(entry.filename.c_str());
                    }
                    for (const auto& entry : swap_info) {
                        swapon(entry.filename.c_str(), (entry.priority << SWAP_FLAG_PRIO_SHIFT) & SWAP_FLAG_PRIO_MASK);
                    }
                }
            }
        }

        // std::cout << "Memory available: " << info.available / 1e+9 << " GB" << std::endl;
        // std::cout << "Total memory:     " << info.total / 1e+9 << " GB" << std::endl;

        // std::vector<SwapInfo> swap_info = manager.swap_info();
        // for (const auto& entry : swap_info) {
        //     std::cout << "  Filename: " << entry.filename << std::endl;
        //     std::cout << "  Size:     " << entry.size << std::endl;
        //     std::cout << "  Usage:    " << entry.usage << std::endl;
        // }

    }
}
