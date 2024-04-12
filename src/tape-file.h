
#pragma once

#include <chrono>
#include <thread>
#include <filesystem>
#include <fstream>
#include <string>

#include "configuration.h"
#include "tape.h"

// Эмулятор ленты, использующий файл.
template<typename value_type = std::int32_t>
class FileTape : public Tape<value_type> {
public:
    FileTape(const Configuration &config, const std::string &name);
    FileTape(FileTape &&other) = default;
    FileTape(const FileTape &other) = delete;
    FileTape &operator=(const FileTape &other) = delete;
    FileTape &operator=(FileTape &&other) = default;

    [[nodiscard]] value_type peek() override;
    [[nodiscard]] value_type read() override;
    void write(const value_type &value) override;

    void stepForward() override;
    bool setpos(std::fstream::pos_type off) override;
    void rewind() override;

    [[nodiscard]] std::size_t size() override;

private:
    void write_fast(const value_type &value);
    [[nodiscard]] value_type read_fast();

    std::fstream file_stream;

    std::chrono::microseconds read_delay;
    std::chrono::microseconds write_delay;
    std::chrono::microseconds move_delay;
    std::chrono::microseconds rewind_delay;
};

template<typename value_type>
FileTape<value_type>::FileTape(const Configuration &config,
                               const std::string &file_name)
    : read_delay(config.getReadDelay()),
      write_delay(config.getWriteDelay()),
      move_delay(config.getMoveDelay()),
      rewind_delay(config.getRewindDelay()) {
    auto mode = std::ios_base::in | std::ios_base::out | std::ios_base::binary;

    if (!std::filesystem::exists(file_name)) {
        std::ofstream{file_name};
    }

    file_stream = std::fstream(file_name, mode);

    if (!file_stream) {
        throw std::invalid_argument("Failed to open the file with name '" + file_name + "'.");
    }
}

template<typename value_type>
value_type FileTape<value_type>::read() {
    std::this_thread::sleep_for(read_delay + move_delay);
    return read_fast();
}

template<typename value_type>
void FileTape<value_type>::write(const value_type &value) {
    std::this_thread::sleep_for(write_delay + move_delay);
    write_fast(value);
}

template<typename value_type>
value_type FileTape<value_type>::peek() {
    std::this_thread::sleep_for(read_delay);
    const auto before = file_stream.tellg();
    value_type value;
    file_stream.read(reinterpret_cast<char *>(&value), sizeof(value));
    file_stream.seekg(before);
    return value;
}

template<typename value_type>
value_type FileTape<value_type>::read_fast() {
    value_type value;
    const auto before = file_stream.tellg();
    file_stream.read(reinterpret_cast<char *>(&value), sizeof(value));
    return value;
}

template<typename Value>
void FileTape<Value>::write_fast(const Value &value) {
    const auto before = file_stream.tellg();
    file_stream.write(reinterpret_cast<const char *>(&value), sizeof(value));
}

template<typename value_type>
bool FileTape<value_type>::setpos(std::fstream::pos_type off) {
    std::this_thread::sleep_for(rewind_delay);
    file_stream.seekg(off * std::streamoff(sizeof(value_type)));
    return true;
}

template<typename value_type>
void FileTape<value_type>::stepForward() {
    std::this_thread::sleep_for(move_delay);
    const auto initial_pos = file_stream.tellg();
    file_stream.seekg(initial_pos + std::streamoff(sizeof(value_type)));
}

template<typename value_type>
void FileTape<value_type>::rewind() {
    std::this_thread::sleep_for(rewind_delay);
    file_stream.seekg(0);
}

template<typename value_type>
std::size_t FileTape<value_type>::size() {
    const auto initial = file_stream.tellg();
    file_stream.seekg(0, std::ios_base::end);
    const auto end = file_stream.tellg();
    file_stream.seekg(initial);
    return static_cast<std::size_t>(end);
}
