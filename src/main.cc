
#include <cstdint>
#include <iostream>

#include "tape-file.h"
#include "tape-sort.h"

int main(int argc, char **argv) {
    if (argc < 3) {
        std::cerr << argv[0]
                  << ": input and output files must be provided." << std::endl
                  << "Example: " << argv[0] << " [input-file] [output-file]" << std::endl;
        return 1;
    }

    using value_type = std::int32_t;
    const Configuration config;
    FileTape<value_type> input_file_tape(config, argv[1]);
    FileTape<value_type> output_file_tape(config, argv[2]);

    try {
        TapeSort<value_type>(config, input_file_tape, output_file_tape).sort();
        return 0;
    } catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Unknown error" << std::endl;
        return 1;
    }
}
