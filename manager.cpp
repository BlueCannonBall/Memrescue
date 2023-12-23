#include "manager.hpp"
#include <algorithm>
#include <cctype>
#include <filesystem>
#include <limits>
#include <signal.h>
#include <stdexcept>
#include <unordered_map>

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
        swap_info_file >> entry.filename >> entry.type >> entry.size >> entry.usage >> entry.priority;
    }

    if (!swap_info_file && !swap_info_file.eof()) {
        throw std::runtime_error("Failed to read /proc/swaps");
    }
    return ret;
}

void ResourceManager::adjust_oom_score(pid_t pid, int adjustment) {
    std::ofstream file("/proc/" + std::to_string(pid) + "/oom_score_adj");
    file << adjustment;
    if (!file && !file.eof()) {
        throw std::runtime_error("Failed to write to /proc/" + std::to_string(pid) + "/oom_score_adj");
    }
}

pid_t ResourceManager::oom_kill() {
    std::unordered_map<pid_t, int> oom_scores;

    for (const auto& entry : std::filesystem::directory_iterator("/proc")) {
        if (entry.is_directory()) {
            std::string filename = entry.path().filename();
            for (char c : filename) {
                if (!isdigit(c)) {
                    goto next_entry;
                }
            }

            std::ifstream oom_score_file(entry.path() / "oom_score");
            int oom_score;
            oom_score_file >> oom_score;

            if (oom_score_file) {
                oom_scores[std::stoi(filename)] = oom_score;
            }
        }
    next_entry:;
    }

    pid_t pid = std::max_element(oom_scores.begin(), oom_scores.end(), [](const auto& a, const auto& b) {
        return a.second < b.second;
    })->first;
    if (kill(pid, SIGKILL) == -1) {
        return -1;
    }
    return pid;
}
