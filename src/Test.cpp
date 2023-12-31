/**

  Copyright (c) 2023, Intel Corporation 
  SPDX-License-Identifier: MIT

**/

#include <sstream>

#include "Test.h"

Test::Test() {
    this->logger = Logger::build();
    this->algo_manager = std::make_shared<AlgoManager>();
}

void Test::load_generators(void){
    // Iterate thread definition struct
    for (auto & [hw_id, thread_definition] : this->threads_define) {
        uint16_t protocol_id;
        uint32_t loops, pattern_param;
        uint32_t algo_params_offset, pattern_size;
        uint64_t thread_offset, thread_size, thread_target, pattern, cache_aligned;

        std::stringstream ss_algo_params(thread_definition["algo-params"]);
        ss_algo_params.flags(std::ios_base::hex);
        ss_algo_params >> algo_params_offset;

        std::stringstream ss_offset(thread_definition["offset"]);
        ss_offset.flags(std::ios_base::dec);
        ss_offset >> thread_offset;

        std::stringstream ss_pattern(thread_definition["pattern"]);
        ss_pattern.flags(std::ios_base::hex);
        ss_pattern >> pattern;

        std::stringstream ss_protocol(thread_definition["protocol"]);
        ss_pattern.flags(std::ios_base::dec);
        ss_protocol >> protocol_id;
        
        thread_size = std::stoi(thread_definition["size"]);
        pattern_size = std::stoi(thread_definition["patternsize"]);
        loops = std::stoi(thread_definition["setloops"]);
        pattern_param = std::stoi(thread_definition["patternparam"]);
        cache_aligned = std::stoi(thread_definition["cachealigned"]);
        thread_target = std::stoi(thread_definition["target"]);

        /* Get target address */ 
        if (targets.find(thread_target) == targets.end()){
            this->logger->report_failure("Target ID not found.");
            exit(0);
        }

        auto algo = algo_manager->build_algo(thread_definition["algorithm"]);
        auto addrList = targets[thread_target]->GetAddressList();

        if (thread_definition["type"] == "core") {
            auto algoInst = std::make_shared<MulWrStreamNew>(algo_params_offset, pattern, thread_offset, thread_size);
            auto generator = std::make_shared<CpuTrafficGenerator>();
            generator->setAffinity(hw_id);
            generator->setAlgorithm(algoInst);
            generator->setAddressList(std::move(addrList));
            this->generators.push_back(generator);
        } else if (thread_definition["type"] == "device") { 
            auto algoInst = std::make_shared<MulWrStreamNew>(algo_params_offset, pattern, thread_offset, thread_size);
            auto generator = std::make_shared<DeviceTrafficGenerator>(addrList, pattern, pattern_size, pattern_param, 0, hw_id, 0, 0, protocol_id);
            generator->setAlgorithm(algoInst);
            generator->setStartAddressCacheAligned(cache_aligned);
            generator->setAddressList(std::move(addrList));
            generator->setOffset(thread_offset);
            generator->setSize(thread_size);
            generator->setLoops(loops);
            generator->setPatternParam(pattern_param);
            generator->setAlgoParams(algo_params_offset);
            generator->setProtocol(protocol_id);
            this->generators.push_back(generator);
        }
    }

    /* Verify hammer defined a thread type for traffic, exit if not */
        if (this->generators.empty()){
        this->logger->report_failure("No thread-types specified in test file, nothing to run. Do you even Cxl?");
        exit(0);
    }
}

void Test::configure(void){
    for (auto & generator : this->generators) {
       std::thread executor([&]{generator->task();});
       generator->configure();
       this->executors.push_back(std::move(executor));
    }
}

void Test::clear_memory(void){
    for (auto & [id, target] : this->targets) {
        uint64_t target_address = target->address;
        uint64_t target_size = target->size;
        this->logger->clear_memory(target_address, target_size, true);
    }
}

void Test::start(void){
    this->logger->print("Starting threads...", 200);
    for (auto & generator : this->generators) {
        generator->start();
    }
    // Let everything to start running
    std::this_thread::sleep_for(std::chrono::seconds(1));
}

void Test::stop(void){
    this->logger->print("Stopping threads.", 200);
    for (auto & generator : generators) {
        generator->stop();
    }
    // waiti for threads, paciently
    std::this_thread::sleep_for(std::chrono::seconds(1));
    this->logger->print("Waiting for threads...", 200);
    for (auto & executor : this->executors) {
    executor.join();
    }
}

bool Test::verify(void){
    bool status = false;
    
    this->logger->print("Verify results.", 200);
    uint8_t errFlag = 0;
    ret_t retCode = 0;
    for (auto& generator : generators) {
        retCode = generator->check();
        if (retCode < 0) {
            errFlag = 1;
        }
    }
    if (errFlag != 0) {
        this->logger->print("TEST FAILED.", 200);
        // To dump the AFU register values in case of failure
        for (auto& generator : this->generators) {
            generator->dump();
        }
    } else {
        this->logger->print("TEST PASSED.", 200);
        status = true;
    }
    return status;
}


void Test::dump(void){
    for (auto & [id, target] : this->targets) {
        std::string msg = "Target " + std::to_string(id) + " memory dump.";
        this->logger->print(msg, 200);
        uint64_t target_address = target->address;
        uint64_t target_size = target->size;
        this->logger->print_memory(target_address, target_size, false);
    }
    this->logger->print("Statistics.", 200);
    for (auto & generator : this->generators) {
        generator->print();
    }

}