
#pragma once

#include <memory>

#include "tape.h"
#include "tape-file.h"
#include "configuration.h"

// Класс, предоставляющий интерфейс создания ленты
template<typename value_type>
class TapeProvider {
public:
    [[nodiscard]] virtual std::unique_ptr<Tape<value_type>> create_tape(const std::filesystem::path &path) const = 0;
};

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
