/**

  Copyright (c) 2023, Intel Corporation 
  SPDX-License-Identifier: MIT

**/

#include <iostream>
#include "IAlgorithm.h"

IAlgorithm::IAlgorithm() {
    mLogger = Logger::build();
}

void IAlgorithm::setAddressList(std::shared_ptr<AddressList> pAddrList)
{
	mpAddrList = std::move(pAddrList);
}
