/**

  Copyright (c) 2023, Intel Corporation 
  SPDX-License-Identifier: MIT

**/

#include "ITrafficGenerator.h"

ITrafficGenerator::ITrafficGenerator() {
    mLogger = Logger::build();
}