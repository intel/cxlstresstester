/**

  Copyright (c) 2023, Intel Corporation 
  SPDX-License-Identifier: MIT

**/

#pragma once
#include <map>
#include <thread>
#include <unordered_map>

#include "algo/AlgoManager.h"
#include "algo/MulWrStream.h"
#include "utils/Logger.h"
#include "generator/CpuTrafficGenerator.h"
#include "generator/DeviceTrafficGenerator.h"
#include "Target.h"

class Test {
   private:
    std::shared_ptr<Logger> logger;

   public:
    bool display_dump = false;
    /* Targets handle memory management request. */
    std::unordered_map<std::uint64_t, std::shared_ptr<Target>> targets;
    /* Define threads data struct according to total CPUs in system */
    std::map<std::uint64_t, std::unordered_map<std::string, std::string>> threads_define;
    /* Generators object are the ones that run the thread. */
    std::vector<std::shared_ptr<ITrafficGenerator>> generators;
    /* Threads where generators run on top. */
    std::vector<std::thread> executors;
    /* manager that creates functions to be run */
    std::shared_ptr<AlgoManager> algo_manager;
    //auto resource_manager = std::make_shared<ResourceManager>();

    void load_generators(void);
    void run(void);
    void configure(void);
    void clear_memory(void);
    void start(void);
    void stop(void);
    bool verify(void);
    void dump(void);
    Test();
    ~Test(){}
};