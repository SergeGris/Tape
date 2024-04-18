
#pragma once

#include <algorithm>
#include <fstream>
#include <queue>

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

    ~TapeSort() {
        if (std::filesystem::exists(temporary_directory)) {
            std::filesystem::remove_all(temporary_directory);
        }
    }

    void sort();
private:
    const Configuration &config;
    const TapeProvider<value_type> &provider;
    Tape<value_type> &input_tape;
    Tape<value_type> &output_tape;

    const std::filesystem::path temporary_directory = std::filesystem::temp_directory_path() / "tapes";

    std::unique_ptr<Tape<value_type>> create_temporary_tape(std::string name) {
        return provider.create_tape(temporary_directory / std::filesystem::path(name));
    }

    void merge(const std::vector<size_t> &merge_candidates_id, Tape<value_type> &merged_tape);
};

template<typename value_type>
class TapeBlock {
public:
    TapeBlock(std::unique_ptr<Tape<value_type>> tape, std::size_t max_block_size)
        : max_block_size(max_block_size), data(max_block_size), tape(std::move(tape)) {
        update();
    }

    bool update() {
        current_size = tape->readblock(data, max_block_size);
        return current_size != 0;
    }

    std::size_t size() const {
        return current_size;
    }

    value_type at(size_t i) const {
        if (i < current_size) {
            return data[i];
        } else {
            throw std::runtime_error("index out of range");
        }
    }

private:
    size_t current_size = 0;
    size_t max_block_size;
    std::vector<value_type> data;
    std::unique_ptr<Tape<value_type>> tape;
};

template<typename value_type>
class TapeBlockPriorityStack {
public:
    TapeBlockPriorityStack(std::vector<std::unique_ptr<Tape<value_type>>> block_tapes, std::size_t max_block_size)
        : max_block_size(max_block_size) {
        for (auto &tape_ptr : block_tapes) {
            block_stack.emplace_back(std::move(tape_ptr), max_block_size);
        }

        for (auto &block : block_stack) {
            priority_queue.emplace(&block, 0);
        }
    }

    bool empty() const {
        return priority_queue.empty();
    }

    value_type peek() const {
        return priority_queue.top().first->at(priority_queue.top().second);
    }

    void pop() {
        IndexedBlock top_block = priority_queue.top();
        priority_queue.pop();

        if (top_block.second + 1 < top_block.first->size()) {
            top_block.second++;
            priority_queue.emplace(top_block);
            return;
        }

        if (top_block.first->update()) {
            top_block.second = 0;
            priority_queue.emplace(top_block);
        }
    }

private:
    using IndexedBlock = std::pair<TapeBlock<value_type> *, std::size_t>;

    struct IndexedBlockComparator {
        bool operator()(const IndexedBlock &lhs, const IndexedBlock &rhs) {
            return lhs.first->at(lhs.second) > rhs.first->at(rhs.second);
        }
    };

    std::vector<TapeBlock<value_type>> block_stack;
    std::size_t max_block_size;
    std::priority_queue<IndexedBlock, std::vector<IndexedBlock>, IndexedBlockComparator> priority_queue;
};

template<typename value_type, typename comparator>
void TapeSort<value_type, comparator>::sort() {
    const std::size_t max_buffer_size = config.getMemoryLimit() / sizeof(value_type);
    const std::size_t max_tapes_to_merge = max_buffer_size; // No more than memory given

    std::size_t number_of_sorted_tapes = 0;
    std::queue<std::size_t> tapes_queue;

    {
        std::vector<value_type> buffer(max_buffer_size);
        std::size_t count;

        while ((count = input_tape.readblock(buffer, max_buffer_size)) != 0) {
            std::sort(buffer.begin(), buffer.begin() + count, comparator());
            tapes_queue.push(number_of_sorted_tapes);
            auto tape = create_temporary_tape(std::to_string(number_of_sorted_tapes++));
            tape->writeblock(buffer, count);
        }
    }

    std::size_t merge_tape_number = number_of_sorted_tapes;
    std::vector<std::size_t> tapes_to_merge;

    while (!tapes_queue.empty()) {
        while (!tapes_queue.empty() && tapes_to_merge.size() < max_tapes_to_merge) {
            tapes_to_merge.push_back(tapes_queue.front());
            tapes_queue.pop();
        }

        if (tapes_queue.empty()) {
            merge(tapes_to_merge, output_tape);
            return;
        }

        auto merged_tape(create_temporary_tape(std::to_string(merge_tape_number)));
        merge(tapes_to_merge, *merged_tape.get());
        tapes_to_merge.clear();
        tapes_queue.push(merge_tape_number);
        merge_tape_number++;
    }
}

template<typename value_type, typename comparator>
void TapeSort<value_type, comparator>::merge(const std::vector<std::size_t> &merge_candidates_id,
                                             Tape<value_type> &merged_tape) {
    const std::size_t block_size = config.getMemoryLimit() / sizeof(value_type);
    std::vector<std::unique_ptr<Tape<value_type>>> tapes_to_merge;

    for (auto id : merge_candidates_id) {
        tapes_to_merge.emplace_back(create_temporary_tape(std::to_string(id)));
    }

    std::vector<value_type> merged_block;
    TapeBlockPriorityStack stack(std::move(tapes_to_merge), block_size);

    while (!stack.empty()) {
        merged_block.push_back(stack.peek());
        stack.pop();

        if (merged_block.size() == block_size) {
            merged_tape.writeblock(merged_block, block_size);
            merged_block.clear();
        }
    }

    if (!merged_block.empty()) {
        merged_tape.writeblock(merged_block, merged_block.size());
    }
}
