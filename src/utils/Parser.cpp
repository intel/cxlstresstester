/**

  Copyright (c) 2023, Intel Corporation 
  SPDX-License-Identifier: MIT

**/

#include <regex>
#include <fstream>
#include <random>
#include <array>


#include "Parser.h"

Parser::Parser() {
    this->logger = Logger::build();
}

void Parser::parse_command_line(int parameter_number, char** command_line){
    if (parameter_number < 2){
        this->logger->print("Run 'cxlhammer --help' for more information.",201);
        exit(0);
    }

    std::vector<std::string> options;
    for(int parameter=1;parameter<parameter_number;parameter++){
        options.push_back(command_line[parameter]);
    }

    /* Check switches */
    std::smatch cmd_line;
    for (auto & option: options){
        if (std::regex_match(option, cmd_line, std::regex("--help|-h"))) {
            this->logger->print_helper();
            exit(0);
        }
        else if (std::regex_match(option, cmd_line, std::regex("--info|-i"))) {
            this->logger->print_app_info();
            exit(0);
        }
        else if (std::regex_match(option, cmd_line, std::regex("--dump|-d"))) {
            this->display_dump = true;
        }
        else if (std::regex_match(option, cmd_line, std::regex(".*\\.hammer.*"))) {
            /* Set test file name. */
            this->file = option;
            /* Open test file. */
            std::ifstream test_file(this->file);
            /* Test if file exists. */
            if (!test_file.good()){
                this->logger->report_failure("Test file `" + this->file + "` doesn't exist. Do you even CXL?");
                exit(0);
            }
        }
    }
    return;
}

void Parser::parse_hammer_file(std::unordered_map<std::uint64_t, std::shared_ptr<Target>>& targets,\
                               std::map<std::uint64_t, std::unordered_map<std::string, std::string>>& threads_define)
{
    std::smatch param;
    std::ifstream test_file(this->file);

    this->logger->print("Using test file " + this->file, 200);
    for(std::string line; getline(test_file, line);) {
        line.erase(std::remove(line.begin(), line.end(), '\r'), line.end());

        // do not print if # added in beggining of line
        if (line.empty() || (std::regex_match(line, std::regex("^#.*$")))) { continue; };
        
        // Assign cpus to resources
        /* read cpus from numa */
        uint64_t total_cpus/* = std::thread::hardware_concurrency()*/;
        /* Test override cpu number. */
        if (std::regex_match(line, param, std::regex("^--cpus=(.*)"))) {
            total_cpus = std::stoi(param[1]);
        }

        if (std::regex_match(line, param, std::regex(".*--define-(target|thread)\\b(.*)"))) {
            /* Create target */
            if (param[1] == "target") {
                if (!this->validate_target_switches(line)) {
			        std::cout << "Correct the input hammer parameters! Exiting Bye! "<<std::endl;
			        exit(1); 
                }
                if (!std::regex_match(line, param, std::regex("^.*(?=.*--id=(\\d+)\\b)(?=.*--node=(\\d+)\\b)(?=.*--addr-start=(\\w+)\\b)(?=.*--num-sets=(\\d+)\\b)(?=.*--set-offset-incr=(\\w+)\\b)(?=.*--num-addr-incr=(\\d+)\\b)(?=.*--addr-incr=(\\w+)\\b).*$"))){
                    std::cout << "Correct the input hammer parameters! Exiting Bye! "<<std::endl;
			        exit(1);
                }

                uint32_t target_id;
                uint64_t node_id, addr_start, set_offset_incr, addr_incr;
                uint16_t num_sets, num_addr_incr;

                std::stringstream ss_id(param[1]);
                ss_id.flags(std::ios_base::dec);
                ss_id >> target_id;

                /* Fail if same target ID is used. */
                if (targets.find(target_id) != targets.end()) {
                    throw std::runtime_error("Target ID repeated.");
                }

                std::stringstream ss_node(param[2]);
                ss_node.flags(std::ios_base::dec);
                ss_node >> node_id;

                std::stringstream ss_addr_start(param[3]);
                ss_addr_start.flags(std::ios_base::hex);
                ss_addr_start >> addr_start;

                std::stringstream ss_num_sets(param[4]);
                ss_num_sets.flags(std::ios_base::dec);
                ss_num_sets >> num_sets;

                std::stringstream ss_set_offset_incr(param[5]);
                ss_set_offset_incr.flags(std::ios_base::hex);
                ss_set_offset_incr >> set_offset_incr;

                std::stringstream ss_num_addr_incr(param[6]);
                ss_num_addr_incr.flags(std::ios_base::dec);
                ss_num_addr_incr >> num_addr_incr;

                std::stringstream ss_addr_incr(param[7]);
                ss_addr_incr.flags(std::ios_base::hex);
                ss_addr_incr >> addr_incr;

                // building the target should not be done here
                targets.insert({target_id, std::make_shared<Target>(target_id, node_id, 
                                addr_start, num_sets, set_offset_incr, 
                                num_addr_incr, (addr_incr << 6))});

                /* Create thread */
            } else if (param[1] == "thread"){
                if (!this->validate_thread_params(line)) {
                    continue;
                }

                std::smatch thread_parameters;
                if(!std::regex_match(line, thread_parameters, std::regex("^.*(?=.*--type=(\\w+))(?=.*--hwid=(\\d+))(?=.*--algorithm=([A-Za-z0-9]*))(?=.*--algo-params=(\\w+))(?=.*--offset=(\\d+))(?=.*--size=(\\d+))(?=.*--pattern=(\\w+))(?=.*--patternsize=(\\d+))(?=.*--setloops=(\\w+))(?=.*--patternparam=(\\d+))(?=.*--cachealigned=(\\d+))(?=.*--protocol=(\\d+))(?=.*--target=(\\d+)).*$"))){
                    std::cout << "Correct the input hammer parameters! Exiting Bye! "<<std::endl;
			        exit(1);
                }
                uint32_t hw_id;
                std::string thread_type = thread_parameters[1];
                hw_id = std::stoi(thread_parameters[2]);
                if (thread_type == "core" || thread_type == "device") {
                    threads_define[hw_id]["type"] = std::move(thread_type);
                    threads_define[hw_id]["algorithm"] = thread_parameters[3];
                    threads_define[hw_id]["algo-params"] = thread_parameters[4];
                    threads_define[hw_id]["offset"] = thread_parameters[5];
                    threads_define[hw_id]["size"] = thread_parameters[6];
                    threads_define[hw_id]["pattern"] = thread_parameters[7];
                    threads_define[hw_id]["patternsize"] = thread_parameters[8];
                    threads_define[hw_id]["setloops"] = thread_parameters[9];
                    threads_define[hw_id]["patternparam"] = thread_parameters[10];
                    threads_define[hw_id]["cachealigned"] = thread_parameters[11];
                    threads_define[hw_id]["protocol"] = thread_parameters[12];
                    threads_define[hw_id]["target"] = thread_parameters[13];
                } else {
                        this->logger->report_failure("Unsupported thread type found.");
                        exit(0);
                }
            } else { 
                this->logger->report_failure("Unsupported element. Please select target or thread.");
                exit(0);
            }
        }
    }
}

bool Parser::validate_target_switches(std::string str){
    std::smatch param;
    std::string missingSwitches = "";

    if (!std::regex_match(str, param, std::regex("^.*--id=(\\d+)\\b.*$"))) {
        missingSwitches.append("--id= ");
    }

    if (!std::regex_match(str, param, std::regex("^.*--node=(\\d+)\\b.*$"))) {
        missingSwitches.append("--node= ");
    }

    if (!std::regex_match(str, param, std::regex("^.*--addr-start=(\\w+)\\b.*$"))) {
        missingSwitches.append("--addr-start= ");
    }

    if (!std::regex_match(str, param, std::regex("^.*--num-sets=(\\d+)\\b.*$"))) {
        missingSwitches.append("--num-sets= ");
    }

    if (!std::regex_match(str, param, std::regex("^.*--set-offset-incr=(\\w+)\\b.*$"))) {
        missingSwitches.append("--set-offset-incr= ");
    }

    if (!std::regex_match(str, param, std::regex("^.*--num-addr-incr=(\\d+)\\b.*$"))) {
        missingSwitches.append("--num-addr-incr= ");
    }

    if (!std::regex_match(str, param, std::regex("^.*--addr-incr=(\\w+)\\b.*$"))) {
        missingSwitches.append("--addr-incr= ");
    }

    if (missingSwitches.length() > 0) {
        this->logger->print("Missing switch(es): " + missingSwitches, 2);
        return false;
    }

    return true;
}

std::uint64_t Parser::pick_choice(std::vector<std::pair<std::uint64_t, std::uint64_t>> weighted_choices, std::uint64_t distribution){
    for (auto& [choice, weight] : weighted_choices) {
        if (distribution < weight){
            return choice;
        }
        distribution -= weight;
    }
    throw std::runtime_error("Weighted algo couldn't determine choice.");
}

bool Parser::validate_thread_params(std::string str) {
    std::array<std::string, 13> paramList = {
        "--type=(\\w+)",
        "--hwid=(\\d+)",
        "--algorithm=([A-Za-z0-9]*)",
        "--algo-params=(\\w+)",
        "--offset=(\\d+)",
        "--size=(\\d+)",
        "--pattern=(\\w+)",
        "--patternsize=(\\d+)",
        "--setloops=(\\w+)",
        "--patternparam=(\\d+)",
        "--cachealigned=(\\d+)",
        "--protocol=(\\d+)",
        "--target=(\\d+)"
    };

    std::string missingParams = "";
    for (auto s = paramList.begin(); s != paramList.end(); ++s) {
        if (!std::regex_match(str, std::regex("^.*" + *s + "\\b.*$"))) {
            missingParams.append((*s).substr(0, (*s).find("(")) + " ");
        }
    }
    
    if (missingParams.length() > 0) {
        this->logger->print("Missing params(s): " + missingParams, 2);
        return false;
    }

    return true;
}