
#pragma once

#include <filesystem>
#include <memory>

#include "tape.h"

// Класс, предоставляющий интерфейс создания ленты
template<typename value_type>
class TapeProvider {
public:
    [[nodiscard]] virtual std::unique_ptr<Tape<value_type>> create_tape(const std::filesystem::path &path) const = 0;
};
