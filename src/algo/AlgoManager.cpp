/**

  Copyright (c) 2023, Intel Corporation 
  SPDX-License-Identifier: MIT

**/

#include <iostream>

#include "AlgoManager.h"
#include "MulWrStream.h"

AlgoManager::AlgoManager(){
	/* Algo directory */
	algo_types["MulWr"]   = &define_algo<MulWr64>;
	algo_types["MulWr64"] = &define_algo<MulWr64>;
	algo_types["MulWr32"] = &define_algo<MulWr32>;
	// algo_types["MulWrStream"] = &define_algo<MulWrStreamNew>;
}

std::shared_ptr<IAlgorithm> AlgoManager::build_algo(std::string& algo){
    return algo_types[algo]();
}
