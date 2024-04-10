
#pragma once

#include <fstream>
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

    [[nodiscard]] virtual value_type peek() = 0;
    [[nodiscard]] virtual value_type read() = 0;
    virtual void write(const value_type &value) = 0;

    virtual void stepForward() = 0;
    virtual bool setpos(std::fstream::pos_type off) = 0;
    virtual void rewind() = 0;

    [[nodiscard]] virtual std::size_t size() = 0;
};
