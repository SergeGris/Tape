
#pragma once

#include <chrono>
#include <thread>
#include <filesystem>
#include <fstream>
#include <string>
#include <optional>

#include "configuration.h"
#include "tape.h"

// Эмулятор ленты, использующий файл.
template<typename value_type>
class FileTape : public Tape<value_type> {
public:
    FileTape(const Configuration &config, const std::string &name);
    FileTape(FileTape &&other) = default;
    FileTape(const FileTape &other) = delete;
    FileTape &operator=(const FileTape &other) = delete;
    FileTape &operator=(FileTape &&other) = default;

    [[nodiscard]] std::optional<value_type> peek() override;
    [[nodiscard]] std::optional<value_type> read() override;
    [[nodiscard]] std::size_t readblock(std::vector<value_type> &vector, std::size_t count) override;
    void write(const value_type &value) override;
    void writeblock(const std::vector<value_type> &vector, std::size_t count) override;
    [[nodiscard]] std::size_t size() override;

private:
    void write_fast(const value_type &value);
    [[nodiscard]] std::optional<value_type> read_fast();

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
std::optional<value_type> FileTape<value_type>::read() {
    std::this_thread::sleep_for(read_delay + move_delay);
    return read_fast();
}

template<typename value_type>
std::size_t FileTape<value_type>::readblock(std::vector<value_type> &vec, std::size_t count) {
    std::size_t i;

    for (i = 0; i < count; i++) {
        const auto value = read_fast();

        if (!value) {
            break;
        }

        vec[i] = *value;
    }

    std::this_thread::sleep_for((read_delay + move_delay) * i);
    return i;
}

template<typename value_type>
void FileTape<value_type>::write(const value_type &value) {
    std::this_thread::sleep_for(write_delay + move_delay);
    write_fast(value);
}

template<typename value_type>
void FileTape<value_type>::writeblock(const std::vector<value_type> &vec, std::size_t count) {
    std::size_t i;

    for (i = 0; i < count && !file_stream.eof(); i++) {
        write_fast(vec[i]);
    }

    std::this_thread::sleep_for((write_delay + move_delay) * i);
}

template<typename value_type>
std::optional<value_type> FileTape<value_type>::peek() {
    std::this_thread::sleep_for(read_delay);
    const auto before = file_stream.tellg();
    auto value = read_fast();
    file_stream.seekg(before);
    return value;
}

template<typename value_type>
std::optional<value_type> FileTape<value_type>::read_fast() {
    value_type value;
    file_stream.read(reinterpret_cast<char *>(&value), sizeof(value));

    if (file_stream.eof()) {
        return std::nullopt;
    }

    return value;
}

template<typename Value>
void FileTape<Value>::write_fast(const Value &value) {
    file_stream.write(reinterpret_cast<const char *>(&value), sizeof(value));
}

template<typename value_type>
std::size_t FileTape<value_type>::size() {
    const auto initial = file_stream.tellg();
    file_stream.seekg(0, std::ios_base::end);
    const auto end = file_stream.tellg();
    file_stream.seekg(initial);
    return static_cast<std::size_t>(end);
}
