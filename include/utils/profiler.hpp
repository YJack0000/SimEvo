#ifndef PROFILER_H
#define PROFILER_H

#include <chrono>
#include <iostream>
#include <string>
#include <unordered_map>

class Profiler {
public:
    static Profiler& getInstance() {
        static Profiler instance;
        return instance;
    }

    void start(const std::string& key) {
        startTimes[key] = std::chrono::high_resolution_clock::now();
    }

    void stop(const std::string& key) {
        auto end = std::chrono::high_resolution_clock::now();
        auto start = startTimes.find(key);
        if (start != startTimes.end()) {
            durations[key] +=
                std::chrono::duration<double, std::milli>(end - start->second).count();
            counts[key]++;
        }
    }

    void report(std::string key) const {
        auto entry = durations.find(key);
        if (entry != durations.end()) {
            auto count = counts.find(key)->second;
            double average = entry->second / count;
            std::cout << key << ": " << entry->second << " ms total, " << count << " times, "
                      << average << " ms average" << std::endl;
        }
    }

    void report() const {
        for (const auto& entry : durations) {
            auto count = counts.find(entry.first)->second;
            double average = entry.second / count;
            std::cout << entry.first << ": " << entry.second << " ms total, " << count << " times, "
                      << average << " ms average" << std::endl;
        }
    }

    void reset() {
        durations.clear();
        startTimes.clear();
        counts.clear();
    }

private:
    Profiler() = default;
    ~Profiler() = default;
    Profiler(const Profiler&) = delete;
    Profiler& operator=(const Profiler&) = delete;

    std::unordered_map<std::string, std::chrono::high_resolution_clock::time_point> startTimes;
    std::unordered_map<std::string, double> durations;
    std::unordered_map<std::string, int> counts;
};

#endif  // PROFILER_H
