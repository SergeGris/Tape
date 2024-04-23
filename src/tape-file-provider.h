
#pragma once

#include <filesystem>
#include <memory>

#include "configuration.h"
#include "tape.h"
#include "tape-file.h"
#include "tape-provider.h"

template<typename value_type>
class FileTapeProvider : public TapeProvider<value_type> {
private:
    const Configuration &config;
public:
    FileTapeProvider(const Configuration &config) : config(config) { }

    [[nodiscard]] std::unique_ptr<Tape<value_type>> create_tape(const std::filesystem::path &path) const {
        return std::make_unique<FileTape<value_type>>(config, path.c_str());
    }
};
