//
// Created by matt on 16/03/19.
//

#include <string>
#include <sstream>
#include <iostream>
#include <bzlib.h>
#include <emscripten/bind.h>
#include <iomanip>

class BZ2CC {
public:
    BZ2CC() = default;
    int error = BZ_OK;
    std::string error_msg;
    unsigned char* output;
    unsigned int outLen;

    emscripten::val getOutput() const {
        return emscripten::val(emscripten::typed_memory_view(outLen, output));
    }
};

BZ2CC compressBZ2 (std::string input, int block_size, int work_factor) {
    BZ2CC resp;
    // Convert C++ strings to terrible C pointer arrays
    auto inp = (char*) input.c_str();
    resp.outLen = (unsigned int) (input.length() * 1.01) + 600; // Advised output buffer length is 101% of input + 600 bytes
    char* out = new char[resp.outLen];
    resp.error = BZ2_bzBuffToBuffCompress(out, &resp.outLen, inp, (unsigned int) input.length(), block_size, 0, work_factor);
    switch (resp.error) {
        case BZ_OK:
            resp.output = (unsigned char*) out;
            break;
        case BZ_CONFIG_ERROR:
        case BZ_PARAM_ERROR:
        case BZ_OUTBUFF_FULL:
            resp.error_msg = "There's been an issue with libbzip2-wasm. Please file an issue at https://github.com/artemisbot/libbzip2-wasm.";
            break;
        case BZ_MEM_ERROR:
            resp.error_msg = "libbzip2-wasm has run out of memory.";
            break;
        default:
            resp.error_msg = "Unknown error.";
            break;
    }
    return resp;
}

BZ2CC decompressBZ2 (std::string input, int small) {
    BZ2CC resp;
    // Convert C++ strings to terrible C pointer arrays
    auto inp = (char*) input.c_str();
    resp.outLen = (unsigned int) 10000000; // I'm pretty sure that bzip2 isn't *this* good
    char* out = new char[resp.outLen];
    resp.error = BZ2_bzBuffToBuffDecompress(out, &resp.outLen, inp, (unsigned int) input.length(), small, 0);
    switch (resp.error) {
        case BZ_OK:
            resp.output = (unsigned char*) out;
            break;
        case BZ_CONFIG_ERROR:
        case BZ_PARAM_ERROR:
            resp.error_msg = "There's been an issue with libbzip2-wasm. Please file an issue at https://github.com/artemisbot/libbzip2-wasm.";
            break;
        case BZ_OUTBUFF_FULL:
            resp.error_msg = "The decompressed data exceeds the length of the destination buffer. The max size of a decrypted file is 100MB.";
            break;
        case BZ_MEM_ERROR:
            resp.error_msg = "libbzip2-wasm has run out of memory.";
            break;
        case BZ_DATA_ERROR:
            resp.error_msg = "The input data did not pass the integrity checks.";
            break;
        case BZ_DATA_ERROR_MAGIC:
            resp.error_msg = "The input data does not begin with the correct magic bytes.";
            break;
        case BZ_UNEXPECTED_EOF:
            resp.error_msg = "The input data ends unexpectedly.";
            break;
        default:
            resp.error_msg = "Unknown error.";
            break;
    }
    return resp;
}



EMSCRIPTEN_BINDINGS(my_module) {
    emscripten::class_<BZ2CC>("BZ2CC")
            .property("errorMsg", &BZ2CC::error_msg)
            .property("error", &BZ2CC::error)
            .property("output", &BZ2CC::getOutput);

    emscripten::function("compressBZ2", &compressBZ2);
    emscripten::function("decompressBZ2", &decompressBZ2);
}