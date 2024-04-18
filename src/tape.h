
#pragma once

#include <fstream>
#include <optional>
#include <vector>

// Интерфейс для работы с лентой. value_type -- тип данных на ленте.
template<typename value_type>
class Tape {
public:
    Tape() = default;
    Tape(const Tape &other) = delete;
    Tape(Tape &&other) = default;
    Tape &operator=(const Tape &other) = delete;
    Tape &operator=(Tape &&other) = default;
    virtual ~Tape() = default;

    [[nodiscard]] virtual std::optional<value_type> peek() = 0;
    [[nodiscard]] virtual std::optional<value_type> read() = 0;
    [[nodiscard]] virtual size_t readblock(std::vector<value_type> &vector, std::size_t count) = 0;
    virtual void write(const value_type &value) = 0;
    virtual void writeblock(const std::vector<value_type> &vector, std::size_t count) = 0;
    [[nodiscard]] virtual std::size_t size() = 0;
};
