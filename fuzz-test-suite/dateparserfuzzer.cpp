#include <ql/time/asx.hpp>
#include <ql/time/date.hpp>
#include <ql/time/ecb.hpp>
#include <ql/time/imm.hpp>
#include <ql/time/timeunit.hpp>
#include <ql/utilities/dataparsers.hpp>
#include <string>

#ifndef __clang__
#pragma message("Fuzzer headers are available from clang, other compilers have not been tested.")
#endif
#include <fuzzer/FuzzedDataProvider.h>

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    FuzzedDataProvider fdp(data, size);
    constexpr int kMaxString = 100;
    try {
        (void)QuantLib::DateParser::parseFormatted(fdp.ConsumeRandomLengthString(kMaxString), "%Y-%m-%d");
        (void)QuantLib::DateParser::parseISO(fdp.ConsumeRandomLengthString(kMaxString));
    } catch (const std::exception& e) {
        // Handle or ignore exceptions
    }
    return 0;
}
