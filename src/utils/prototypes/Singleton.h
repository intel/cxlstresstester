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
class Singleton {
   private:
    Singleton() {}
    Singleton(const Singleton &) {}
    Singleton &operator=(const Singleton &) { return *this; }

   public:
    ~Singleton() {}
    std::shared_ptr<Singleton> build();

};
