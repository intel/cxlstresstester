/**

  Copyright (c) 2023, Intel Corporation 
  SPDX-License-Identifier: MIT

**/

#pragma once
#include <vector>
#include <memory>
#include <map>
#include <unordered_map>

#include "Logger.h"
#include "Target.h"

typedef struct{
    std::uint64_t thread;
    std::uint64_t target;
    std::uint64_t stride;
    std::uint64_t xstep;
} Weights;

class Parser {
   private:
    std::shared_ptr<Logger> logger;
    bool validate_target_switches(std::string str);
    bool validate_thread_params(std::string str);
    std::uint64_t pick_choice(std::vector<std::pair<std::uint64_t, std::uint64_t>> weighted_choices, std::uint64_t distribution);

   public:
    bool display_dump = false;
    std::string file;
    Parser();
    ~Parser(){}
    void parse_command_line(int parameter_number, char** command_line);
    void parse_hammer_file(std::unordered_map<std::uint64_t, std::shared_ptr<Target>>& targets,\
                           std::map<std::uint64_t, std::unordered_map<std::string, std::string>>& threads_define);
};
