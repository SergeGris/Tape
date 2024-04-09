
#pragma once

#include <algorithm>
#include <fstream>

#include "configuration.h"
#include "tape.h"
#include "tape-file.h"

template<typename value_type, typename comparator = std::less<value_type>>
class TapeSort {
public:
    TapeSort(const Configuration &config,
             Tape<value_type> &input_tape,
             Tape<value_type> &output_tape)
        : config(config),
          input_tape(input_tape),
          output_tape(output_tape) {
        if (std::filesystem::exists(temporary_directory)) {
            std::filesystem::remove_all(temporary_directory);
        }

        std::filesystem::create_directory(temporary_directory);
    }

    void sort();

    ~TapeSort() {
        if (std::filesystem::exists(temporary_directory)) {
            std::filesystem::remove_all(temporary_directory);
        }
    }

private:
    const Configuration &config;
    Tape<value_type> &input_tape;
    Tape<value_type> &output_tape;

    std::string temporary_directory = "/tmp/tapes";

    FileTape<value_type> create_temporary_tape(std::string name) {
        return FileTape<value_type>(config, (std::filesystem::path(temporary_directory) / std::filesystem::path(name)).c_str());
    }

    void merge(Tape<value_type> &in, Tape<value_type> &out, size_t low, size_t middle, size_t high);
    void merge_sort(Tape<value_type> &in, Tape<value_type> &out, size_t low, size_t high);
};

template<typename value_type, typename comparator>
void TapeSort<value_type, comparator>::sort() {
    merge_sort(input_tape, output_tape, 0, input_tape.size() / sizeof(value_type));
}

template<typename value_type, typename comparator>
void TapeSort<value_type, comparator>::merge(Tape<value_type> &in, Tape<value_type> &out,
                                             size_t low, size_t middle, size_t high) {
    std::size_t low_size = middle - low + 1;
    std::size_t high_size = high - middle;
    auto compare = comparator();

    // Так как сортировка производится в один поток, то можем быть уверены,
    // что в данный момент эти файлы не используются
    auto first = create_temporary_tape("0");
    auto second = create_temporary_tape("1");

    in.setpos(low);

    for (std::size_t i = 0; i < low_size; i++) {
        first.write(in.read());
    }

    in.setpos(middle + 1);

    for (std::size_t j = 0; j < high_size; j++) {
        second.write(in.read());
    }

    std::size_t i = 0;
    std::size_t j = 0;
    std::size_t k = low;

    first.rewind();
    second.rewind();
    out.setpos(k);

    while (i < low_size && j < high_size) {
        auto first_i = first.peek();
        auto second_j = second.peek();

        if (compare(first_i, second_j)) {
            out.write(first_i);
            first.stepForward();
            i++;
        } else {
            out.write(second_j);
            second.stepForward();
            j++;
        }

        k++;
    }

    while (i < low_size) {
        out.write(first.read());
        i++;
        k++;
    }

    while (j < high_size) {
        out.write(second.read());
        j++;
        k++;
    }
}

template<typename value_type, typename comparator>
void TapeSort<value_type, comparator>::merge_sort(Tape<value_type> &in,
                                                  Tape<value_type> &out,
                                                  size_t low, size_t high) {
    size_t total = high - low;

    if (low >= high) {
        return;
    }

    // Если у нас достаточно памяти, чтобы уместить данный отрезок, то отсортируем его в памяти.
    if (high - low <= config.getMemoryLimit() / sizeof(value_type)) {
        std::vector<value_type> vec(total);

        in.setpos(low);

        for (size_t i = 0; i < total; i++) {
            vec[i] = in.read();
        }

        std::sort(vec.begin(), vec.end(), comparator());

        out.setpos(low);

        for (size_t i = 0; i < total; i++) {
            out.write(vec[i]);
        }
    } else {
        size_t middle = low + (high - low) / 2;

        merge_sort(in, out, low, middle);
        merge_sort(in, out, middle + 1, high);
        merge(in, out, low, middle, high);
    }
}
