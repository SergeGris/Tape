
#include "configuration.h"

#include <sstream>
#include <cstdint>
#include <iostream>
#include <fstream>

Configuration::Configuration(const std::string &file_name)
{
    keysWithDefaultValues["ReadDelay"]           = &ReadDelay;
    keysWithDefaultValues["WriteDelay"]          = &WriteDelay;
    keysWithDefaultValues["MoveDelay"]           = &MoveDelay;
    keysWithDefaultValues["RewindDelay"]         = &RewindDelay;
    keysWithDefaultValues["GapCrossDelay"]       = &GapCrossDelay;
    keysWithDefaultValues["MemoryLimit"]         = &MemoryLimit;

    std::ifstream input(file_name);

    if (!input) {
        std::cerr << "Failed to open configuration file: '" << file_name << "'." << std::endl;
        return;
    }

    std::string line;

    for (size_t lineno = 1;
         getline(input, line, '\n');
         lineno++) {
        std::string key;
        unsigned char assign = 0;
        std::uint64_t value;

        line = line.substr(0, line.find("#", 0));

        if (line.empty()) {
            continue;
        }

        std::stringstream ss(line);

        ss >> key;

        if (ss.fail() || keysWithDefaultValues.find(key) == keysWithDefaultValues.end()) {
            std::cerr << file_name << ": " << lineno
                      << ": invalid key found in config: '" << key << "'. Line ignored." << std::endl;
            continue;
        }

        ss >> assign;

        if (ss.fail() || assign != '=') {
            std::cerr << file_name << ": " << lineno
                      << "Expected '=' symbol. Line ignored." << std::endl;
            continue;
        }

        ss >> value;

        if (ss.fail()) {
            std::cerr << file_name << ": " << lineno
                      << "Invalid value found in config. Line ignored." << std::endl;
            continue;
        }

        *keysWithDefaultValues[key] = value;
    }
}

std::uint64_t Configuration::getReadDelay() const {
    return ReadDelay;
}

std::uint64_t Configuration::getWriteDelay() const {
    return WriteDelay;
}

std::uint64_t Configuration::getMoveDelay() const {
    return MoveDelay;
}

std::uint64_t Configuration::getRewindDelay() const {
    return RewindDelay;
}

std::uint64_t Configuration::getGapCrossDelay() const {
    return GapCrossDelay;
}

std::uint64_t Configuration::getMemoryLimit() const {
    return MemoryLimit;
}
