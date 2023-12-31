/**

  Copyright (c) 2023, Intel Corporation 
  SPDX-License-Identifier: MIT

**/

#include "Logger.h"
#include <iomanip>
#include <iostream>
#include <memory>
#include <mutex>

std::shared_ptr<Logger> Logger::build() {
    static std::shared_ptr<Logger> object(new Logger);
    return object;
}

void Logger::verbose(int verbosity) {
    std::lock_guard<std::mutex> lock(this->m_mutex);
    this->verbosity = verbosity;
}

void Logger::log_action(const std::string& message, int verbosity, uint64_t bdf) {
    std::lock_guard<std::mutex> lock(this->m_mutex);
    std::ostream os(std::cout.rdbuf());
    os << "| (Device generator " << std::hex << bdf << ":00.0): " << message << std::endl;
}

void Logger::print(const std::string& message, int verbosity) {
    std::lock_guard<std::mutex> lock(this->m_mutex);
    if (verbosity == 50) {
        std::cout << "| (Device generator): " << message << std::endl;
    } else if (verbosity == 54) {
        std::cout << "| (CPU generator): " << message << std::endl;
    } else if (verbosity == 100) {
        std::cout << "| (Target): " << message << std::endl;
    } else if (verbosity == 200) {
        std::cout << "+--------------------------------------------------------------------------------------------------------+" << std::endl;
        std::cout << "| CXLStressTester (Test Flow): " << message << std::endl;
        std::cout << "+--------------------------------------------------------------------------------------------------------+" << std::endl;
    } else if (verbosity == 201) {
        std::cout << "+--------------------------------------------------------------------------------------------------------+" << std::endl;
        std::cout << "| CXLStressTester: " << message << std::endl;
        std::cout << "+--------------------------------------------------------------------------------------------------------+" << std::endl;
    } else if (verbosity == 1000) {
        std::cout << "+--------------------------------------------------------------------------------------------------------+" << std::endl;
        std::cout << "| CXLStressTester: " << message << std::endl;
        std::cout << "+--------------------------------------------------------------------------------------------------------+" << std::endl;
    } else if (verbosity >= this->verbosity) {
        std::cout << message << std::endl;
    }
}

void Logger::print_app_info(void) {
    std::lock_guard<std::mutex> lock(this->m_mutex);
    std::cout << "| Steps to create a hammer file:"<< std::endl;
    std::cout << "| "<< std::endl;
    std::cout << "| 1.- Create a *.hammer file."<< std::endl;
    std::cout << "| "<< std::endl;
    std::cout << "| 2.- Create target(s)."<< std::endl;
    std::cout << "| "<< std::endl;
    std::cout << "| --define-target"<< std::endl;
    std::cout << "| "<< std::endl;
    std::cout << "| \t--id=dec\n|\t\tAssign a decimal value to parameter to specify target ID. Parameter value cannot be duplicated."<< std::endl;
    std::cout << "| "<< std::endl;
    std::cout << "| \t--node=dec\n|\t\tNuma node where to create the target. Host memory if node is on a socket and HDM if target is on AFU node."<< std::endl;
    std::cout << "| "<< std::endl;
    std::cout << "| \t--addr-start=hex\n|\t\tByte offset added to node target."<< std::endl;
    std::cout << "| "<< std::endl;
    std::cout << "| \t--num-sets=dec\n|\t\tNumber of sets. Minimum allowed is 1."<< std::endl;
    std::cout << "| "<< std::endl;
    std::cout << "| \t--set-offset-incr=hex\n|\t\tOffset increment between sets."<< std::endl;
    std::cout << "| "<< std::endl;
    std::cout << "| \t--num-addr-incr=dec\n|\t\tNumber of ways."<< std::endl;
    std::cout << "| "<< std::endl;
    std::cout << "| \t--addr-incr=hex\n|\t\tCache-line increment between ways."<< std::endl;
    std::cout << "| "<< std::endl;
    std::cout << "| 3.- Create thread(s) (CPU or AFU)."<< std::endl;
    std::cout << "| "<< std::endl;
    //std::cout << "| --define-thread --type= --hwid= --algorithm=MulWr --algo-params= --offset= --size= --pattern= --patternsize= --setloops= --patternparam= --cachealigned= --target="<< std::endl;
    std::cout << "| --define-thread"<< std::endl;
    std::cout << "| "<< std::endl;
    std::cout << "| \t--type=core or --type=device\n|\t\tSet switch to core for CPU traffic or device for AFU CXL traffic."<< std::endl;
    std::cout << "| "<< std::endl;
    std::cout << "| \t--hwid=dec\n|\t\tCpu id if type=core. Device BDF if type=device."<< std::endl;
    std::cout << "| "<< std::endl;
    std::cout << "| \t--algorithm=MulWr\n|\t\tCCV AFU Algorithm1a supported only."<< std::endl;
    std::cout << "| "<< std::endl;
    std::cout << "| \t--algo-params=hex\n|\t\tOnly for device-thread. Bit[0-3] WriteSemnticsOpcode. Bit[4-7] VerifyReadSemanticsOpcode."<< std::endl;
    std::cout << "| "<< std::endl;
    std::cout << "| \t--offset=hex\n|\t\tByte offset in cache line. (False-sharing)"<< std::endl;
    std::cout << "| "<< std::endl;
    std::cout << "| \t--size=dec\n|\t\tSize of write."<< std::endl;
    std::cout << "| "<< std::endl;
    std::cout << "| \t--pattern=hex\n|\t\tPattern to write."<< std::endl;
    std::cout << "| "<< std::endl;
    std::cout << "| \t--patternsize=dec\n|\t\tPattern write mask."<< std::endl;
    std::cout << "| "<< std::endl;
    std::cout << "| \t--setloops=dec\n|\t\t"<< std::endl;
    std::cout << "| "<< std::endl;
    std::cout << "| \t--patternparam=dec\n|\t\tReset pattern each set."<< std::endl;
    std::cout << "| "<< std::endl;
    std::cout << "| \t--cachealigned=dec\n|\t\t"<< std::endl;
    std::cout << "| "<< std::endl;
    std::cout << "| \t--target=dec\n|\t\tSpecify target id from defined targets."<< std::endl;
    std::cout << "| "<< std::endl;
    std::cout << "| Example:"<< std::endl;
    std::cout << "| "<< std::endl;
    std::cout << "| 4.- Run application providing your hammer file as test parameter."<< std::endl;
    std::cout << "| \t`CXLStressTesterr *.hammer`"<< std::endl;
    std::cout << "| "<< std::endl;
    std::cout << "+--------------------------------------------------------------------------------------------------------+" << std::endl;
}

void Logger::print_helper(void) {
    std::lock_guard<std::mutex> lock(this->m_mutex);
    std::cout << "| "<< std::endl;
    std::cout << "| CXLStressTester is an application designed to exercise cache coherency"<< std::endl;
    std::cout << "| between CPUs and CXL-type AFU engines."<< std::endl;
    std::cout << "| "<< std::endl;
    std::cout << "| Options:"<< std::endl;
    std::cout << "| "<< std::endl;
    std::cout << "| \t-h, --help\tDisplay help center."<< std::endl;
    std::cout << "| \t-i, --info\tDisplay application detailed information."<< std::endl;
    std::cout << "| \t-d, --dump\tPrint memory dump from targets created in .hammer test file."<< std::endl;
    std::cout << "| "<< std::endl;
    std::cout << "| "<< std::endl;
    std::cout << "| Examples: "<< std::endl;
    std::cout << "| "<< std::endl;
    std::cout << "| \tCXLStressTester test_file.hammer"<< std::endl;
    std::cout << "| \tCXLStressTester --dump test_file.hammer"<< std::endl;
    std::cout << "| "<< std::endl;
    std::cout << "+--------------------------------------------------------------------------------------------------------+" << std::endl;
}

void Logger::print_memory(uint64_t address, uint64_t size, bool clear) {
    std::lock_guard<std::mutex> lock(this->m_mutex);
    const uint64_t cache_line_size = 64;
    for(uint64_t i = 0; i<size; i+= cache_line_size){
        uint64_t* ptr = reinterpret_cast<uint64_t*>(address+i);
        std::ostream os(std::cout.rdbuf());
        os << std::hex << "| 0x" << (address+i) << ": ";
        for(uint64_t j = 0; j<cache_line_size/sizeof(uint64_t); ++j){
            if (clear){
                ptr[j] = 0;
            }
            std::cout << " 0x" << std::setfill('0') << std::setw(16) << ptr[j] << " ";
        }
        std::cout << std::endl;
    }
}
void Logger::clear_memory(uint64_t address, uint64_t size, bool clear) {
    std::lock_guard<std::mutex> lock(this->m_mutex);
    const uint64_t cache_line_size = 64;
    for(uint64_t i = 0; i<size; i+= cache_line_size){
        uint64_t* ptr = reinterpret_cast<uint64_t*>(address+i);
        for(uint64_t j = 0; j<cache_line_size/sizeof(uint64_t); ++j){
            if (clear){
                ptr[j] = 0;
            }
            //std::cout << " 0x" << std::setfill('0') << std::setw(16) << ptr[j] << " ";
        }
    }
}
void Logger::print_memory_by_nibble(uint64_t address, uint64_t size, bool clear) {
    std::lock_guard<std::mutex> lock(this->m_mutex);
    constexpr std::size_t cache_line_size = 64;
    std::size_t cache_lines = (size + cache_line_size -1)/cache_line_size;
    for(std::size_t i = 0; i < cache_lines; ++i){
        uint64_t start = address + i * cache_line_size;
        std::cout << std::hex << "0x" << start << ":\t";
        for(std::size_t j = 0; j < cache_line_size; ++j){
            uint64_t nibble = start + j;
            std::cout << std::hex << "0x" <<  *reinterpret_cast<uint8_t*>(nibble) << " ";
        }
        std::cout << std::endl;
    }
}

void Logger::report_failure(const std::string &message) {
    std::lock_guard<std::mutex> lock(this->m_mutex);
        std::cout << "+--------------------------------------------------------------------------------------------------------+" << std::endl;
        std::cout << "| CXLStressTester(Error): " << message << std::endl;
        std::cout << "+--------------------------------------------------------------------------------------------------------+" << std::endl;
}
