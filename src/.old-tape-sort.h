
#pragma once

#include <algorithm>
#include <fstream>

#include "tape-provider.h"
#include "configuration.h"
#include "tape.h"
#include "tape-file.h"

template<typename value_type, typename comparator = std::less<value_type>>
class TapeSort {
public:
    TapeSort(const Configuration &config,
             const TapeProvider<value_type> &provider,
             Tape<value_type> &input_tape,
             Tape<value_type> &output_tape)
        : config(config),
          provider(provider),
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
    const TapeProvider<value_type> &provider;
    Tape<value_type> &input_tape;
    Tape<value_type> &output_tape;

    const std::filesystem::path temporary_directory = std::filesystem::temp_directory_path() / "tapes";

    std::unique_ptr<Tape<value_type>> create_temporary_tape(std::string name) {
        return provider.create_tape(temporary_directory / std::filesystem::path(name));
    }

    void merge(Tape<value_type> &in, Tape<value_type> &out, std::size_t low, std::size_t middle, std::size_t high);
    void merge_sort(Tape<value_type> &in, Tape<value_type> &out, std::size_t low, std::size_t high);
};

template<typename value_type, typename comparator>
void TapeSort<value_type, comparator>::sort() {
    // Divide input array on blocks of size max_buffer_size, sort them and store on temporary tapes
    size_t number_of_tapes_generated = generateSortedTemporaryTapes();

    // Merge candidates queue
    std::queue<size_t> tape_candidate_id_queue;
    for (size_t i = 0; i < number_of_tapes_generated; ++i) {
        tape_candidate_id_queue.push(i);
    }

    // Merge tapes while there are at least two unmerged tapes
    size_t merge_tape_id = number_of_tapes_generated;

    while (!tape_candidate_id_queue.empty()) {
        std::vector<size_t> merge_candidates;
        while (merge_candidates.size() < max_number_merge_candidates_ && !tape_candidate_id_queue.empty()) {
            merge_candidates.push_back(tape_candidate_id_queue.front());
            tape_candidate_id_queue.pop();
        }

        if (tape_candidate_id_queue.empty()) {
            merge(merge_candidates, output_tape_.get());
            return;
        }

        std::unique_ptr<Tape<value_type>> merged_tape(tape_generator_->createTape(std::to_string(merge_tape_id).c_str()));
        merge(merge_candidates, merged_tape.get());
        tape_candidate_id_queue.push(merge_tape_id);
        ++merge_tape_id;
    }
}

bool TapeSort::merge(std::vector<size_t> const& merge_candidates_id, Tape* merged_tape) {
    // Tape pointers for corresponding buffer blocks
    std::vector<std::unique_ptr<Tape>> tapes_to_merge;
    for (size_t id : merge_candidates_id) {
        tapes_to_merge.emplace_back(tape_generator_->createTape(std::to_string(id).c_str()));
    }

    std::vector<int> merged_block;

    BlockBuffer block_buffer(std::move(tapes_to_merge), block_size_);
    while (!block_buffer.isEmpty()) {
        merged_block.push_back(block_buffer.getTopValue());
        block_buffer.popTopValue();

        if (merged_block.size() == block_size_) {
            merged_tape->writeBlock(merged_block.data(), block_size_);
            merged_block.clear();
        }
    }

    if (!merged_block.empty()) {
        merged_tape->writeBlock(merged_block.data(), merged_block.size());
    }

    return true;
}

size_t TapeSort::generateSortedTemporaryTapes() {
    size_t number_of_tapes_generated = 0;

    std::vector<int> buffer(max_buffer_size_);
    size_t values_read_count = 0;
    while ((values_read_count = input_tape_->readBlock(buffer.data(), max_buffer_size_))) {
        std::sort(buffer.begin(), buffer.begin() + static_cast<long>(values_read_count));
        tape_generator_->createTape(std::to_string(number_of_tapes_generated++).c_str())
            ->writeBlock(buffer.data(), values_read_count);
    }

    return number_of_tapes_generated;
}


#if 0

#if 0
template<typename value_type>
void readblock(Tape<value_type> &tape, std::vector<value_type> &v, size_t n) {
        for (std::size_t i = 0; i < n; i++) {
            v[i] = tape.read();
        }
}

template<typename value_type>
void writeblock(Tape<value_type> &tape, const std::vector<value_type> &v, size_t n) {
        for (std::size_t i = 0; i < n; i++) {
            tape.write(v[i]);
        }
}

template<typename value_type, typename comparator>
void TapeSort<value_type, comparator>::sort() {
    //(input_tape, output_tape, 0, input_tape.size() / sizeof(value_type));
    const std::size_t size = input_tape.size() / sizeof(value_type);

    input_tape.rewind();
    output_tape.rewind();

    // Если у нас достаточно памяти, чтобы уместить данный отрезок, то отсортируем его в памяти.
    if (size <= config.getMemoryLimit() / sizeof(value_type)) {
        std::vector<value_type> vec(size);

        readblock(input_tape, vec, size);
        std::sort(vec.begin(), vec.end(), comparator());
        writeblock(output_tape, vec, size);
        return;
    }

    const std::size_t block_size = config.getMemoryLimit() / sizeof(value_type);
    const std::size_t count_of_blocks = size / block_size + (size % block_size != 0 ? 1 : 0);
    std::vector<Tape<value_type>> tapes();

    for (std::size_t i = 0; i < size / block_size; i++) {
        std::vector<value_type> vec(block_size);

        for (std::size_t i = 0; i < block_size; i++) {
            vec[i] = input_tape.read();
        }

        std::sort(vec.begin(), vec.end(), comparator());

        for (std::size_t i = 0; i < size; i++) {
            output_tape.write(vec[i]);
        }
    }
}
#else
template<typename value_type, typename comparator>
void TapeSort<value_type, comparator>::sort() {
    merge_sort(input_tape, output_tape, 0, input_tape.size() / sizeof(value_type));
}

template<typename value_type, typename comparator>
void TapeSort<value_type, comparator>::merge(Tape<value_type> &in, Tape<value_type> &out,
                                             std::size_t low, std::size_t middle, std::size_t high) {
    std::size_t low_size = middle - low + 1;
    std::size_t high_size = high - middle;
    auto compare = comparator();

    // Так как сортировка производится в один поток, то можем быть уверены,
    // что в данный момент эти файлы не используются
    auto first = create_temporary_tape("0");
    auto second = create_temporary_tape("1");

    in.setpos(low);

    for (std::size_t i = 0; i < low_size; i++) {
        first->write(in.read());
    }

    for (std::size_t j = 0; j < high_size; j++) {
        second->write(in.read());
    }

    std::size_t i = 0;
    std::size_t j = 0;

    first->rewind();
    second->rewind();
    out.setpos(low);

    while (i < low_size && j < high_size) {
        auto first_i = first->peek();
        auto second_j = second->peek();

        if (compare(first_i, second_j)) {
            out.write(first_i);
            first->stepForward();
            i++;
        } else {
            out.write(second_j);
            second->stepForward();
            j++;
        }
    }

    while (i < low_size) {
        out.write(first->read());
        i++;
    }

    while (j < high_size) {
        out.write(second->read());
        j++;
    }
}

template<typename value_type, typename comparator>
void TapeSort<value_type, comparator>::merge_sort(Tape<value_type> &in,
                                                  Tape<value_type> &out,
                                                  std::size_t low,
                                                  std::size_t high) {
    if (low >= high) {
        return;
    }

    std::size_t total = high - low;

    // Если у нас достаточно памяти, чтобы уместить данный отрезок, то отсортируем его в памяти.
    if (total <= config.getMemoryLimit() / sizeof(value_type)) {
        std::vector<value_type> vec(total);

        in.setpos(low);

        for (std::size_t i = 0; i < total; i++) {
            vec[i] = in.read();
        }

        std::sort(vec.begin(), vec.end(), comparator());

        out.setpos(low);

        for (std::size_t i = 0; i < total; i++) {
            out.write(vec[i]);
        }
    } else {
        std::size_t middle = low + (high - low) / 2;

        merge_sort(in, out, low, middle);
        merge_sort(in, out, middle + 1, high);
        merge(in, out, low, middle, high);
    }
}
#endif
#endif
