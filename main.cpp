#include <chrono>
#include <fstream>
#include <iostream>
#include <limits>
#include <thread>

struct MemoryInfo {
    size_t total;
    size_t free;
    size_t available;
    size_t cached;
    size_t cached_swap;
    size_t total_swap;
    size_t free_swap;
};

class MemoryManager {
protected:
    std::ifstream info_file;
    std::ofstream drop_caches_file;

public:
    MemoryManager():
        info_file("/proc/meminfo"),
        drop_caches_file("/proc/sys/vm/drop_caches") {}

    MemoryInfo info() {
        MemoryInfo ret;

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

        for (size_t i = 0; i < 9 && info_file; ++i) {
            info_file.ignore(std::numeric_limits<std::streamsize>::max(), ':');
        }
        info_file >> ret.total_swap;
        ret.total_swap *= 1000;

        info_file.ignore(std::numeric_limits<std::streamsize>::max(), ':');
        info_file >> ret.free_swap;
        ret.free_swap *= 1000;

        return ret;
    }

    void drop_caches() {
        drop_caches_file << 3;
    }
};

int main() {
    MemoryManager manager;
    for (;;) {
        MemoryInfo info = manager.info();
        std::cout << info.available / 1e+9 << " GB" << std::endl;
        std::cout << info.total / 1e+9 << " GB" << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}
