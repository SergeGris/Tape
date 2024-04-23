
#include <cstdint>
#include <iostream>

#include "tape-file-provider.h"
#include "tape-file.h"
#include "tape-sort.h"

int main(int argc, char **argv) {
    if (argc < 3) {
        std::cerr << argv[0]
                  << ": input and output files must be provided." << std::endl
                  << "Example: " << argv[0] << " [input-file] [output-file]" << std::endl;
        return 1;
    }

    try {
        const Configuration config;
        FileTape<std::int32_t> input_file_tape(config, argv[1]), output_file_tape(config, argv[2]);
        FileTapeProvider<std::int32_t> provider(config);
        TapeSort<std::int32_t>(config, provider, input_file_tape, output_file_tape).sort();
        return 0;
    } catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Unknown error" << std::endl;
        return 1;
    }
}
