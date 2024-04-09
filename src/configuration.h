
#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>

class Configuration {
public:
    explicit Configuration(const std::string &file_name = "tape.cfg");
    Configuration(Configuration &other) = delete;
    Configuration &operator=(Configuration &other) = delete;
    virtual ~Configuration() = default;

    std::uint64_t getReadDelay() const;
    std::uint64_t getWriteDelay() const;
    std::uint64_t getMoveDelay() const;
    std::uint64_t getRewindDelay() const;
    std::uint64_t getGapCrossDelay() const;
    std::uint64_t getMemoryLimit() const;

private:
    std::uint64_t ReadDelay = 7;
    std::uint64_t WriteDelay = 7;
    std::uint64_t MoveDelay = 1;
    std::uint64_t RewindDelay = 1000;
    std::uint64_t GapCrossDelay = 200;

    // Tape sorter
    std::uint64_t MemoryLimit = 1024 * 1024 * 1024; // 1 GiB

    std::unordered_map<std::string, std::uint64_t *> keysWithDefaultValues = {
        {"ReadDelay", &ReadDelay},
        {"WriteDelay", &WriteDelay},
        {"MoveDelay", &MoveDelay},
        {"RewindDelay", &RewindDelay},
        {"GapCrossDelay", &GapCrossDelay},
        {"MemoryLimit", &MemoryLimit},
    };
};
