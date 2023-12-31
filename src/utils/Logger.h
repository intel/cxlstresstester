/**

  Copyright (c) 2023, Intel Corporation 
  SPDX-License-Identifier: MIT

**/

#pragma once
#include <iostream>
#include <memory>
#include <mutex>

/**
 * @brief A singleton used for printing synchronously.
 */
class Logger {
   private:
    Logger() {}
    Logger(const Logger &) {}
    Logger &operator=(const Logger &) { return *this; }
    std::mutex m_mutex;
    int verbosity = 0;

   public:
    ~Logger() {}
    static std::shared_ptr<Logger> build();

    /**
     * @brief Sets the Logger's verbosity level.
     * @param verbosity An integer (0-5 typically) to set the Logger's verbosity to.
     */
    void verbose(int verbosity);

    /**
     * Outputs a message to std::cout. A message will only be output if the passed verbosity is greater than
     * or equal to the verbosity level set inside the Logger.
     *
     * Level 0: Tool Control
     * Level 1: Configuration Error
     * Level 2: Flow Information, Test Sequence
     * Level 3: Hardware Information
     * Level 4:
     * Level 5:
     */
    void log_action(const std::string &message, int verbosity, uint64_t bdf);
    void print(const std::string &message, int verbosity);
    void print_helper(void);
    void print_app_info(void);
    void print_memory(uint64_t address, uint64_t size, bool clear);
    void clear_memory(uint64_t address, uint64_t size, bool clear);
    void print_memory_by_nibble(uint64_t address, uint64_t size, bool clear);

    /**
     * @brief Outputs a message to std::cout. Additionally outputs "FAIL" to std::cerr.
     */
    void report_failure(const std::string &message);
};
