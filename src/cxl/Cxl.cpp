/**

  Copyright (c) 2023, Intel Corporation 
  SPDX-License-Identifier: MIT

**/

#include <iostream>
#include <sstream>

#include "Cxl.h"

Cxl::Cxl(uint32_t tseg, uint32_t tbus, uint32_t tdev, uint32_t tfun): 
    seg(tseg), bus(tbus), dev(tdev), fun(tdev)
{
    this->logger = Logger::build();
}

Cxl::Cxl(uint64_t& phy_addr, uint64_t& phy_size): 
    physical_address(phy_addr), size(phy_size)
{
    this->logger = Logger::build();
    std::stringstream ss;
    ss << "HDM at address: 0x" << std::hex << physical_address << " with size: " << std::dec << size << std::endl;
    this->logger->print(ss.str(), 2);
}

void Cxl::set_physical_address(uint64_t& phy_addr){
	physical_address = phy_addr;
}

void Cxl::get_physical_address(uint64_t& phy_addr){
	phy_addr = physical_address;
}

void Cxl::set_physical_size(uint64_t& phy_size){
	size = phy_size;
}
