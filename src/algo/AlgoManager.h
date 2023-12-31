/**

  Copyright (c) 2023, Intel Corporation 
  SPDX-License-Identifier: MIT

**/

#pragma once
#include <unordered_map>

#include "IAlgorithm.h"


typedef std::shared_ptr<IAlgorithm> algo_pointer;

/**
 * @brief A function template to create a shared pointer to a class that implements the IAlgorithm interface.
 * 
 * @tparam AlgoClass The class to create the shared pointer from.
 * @return A shared pointer to an instance of the AlgoClass.
 */
template <typename AlgoClass> algo_pointer define_algo() {
    return std::make_shared<AlgoClass>();
}

class AlgoManager
{
    private:
        using add_algo = algo_pointer (*)();
        std::unordered_map<std::string, add_algo> algo_types;
    protected:
    public:
        /**
         * @brief Default constructor for the AlgoManager class.
         * 
         * Initializes the `algo_types` map member with algo names as the keys and pointers to the algo classes as the values.
         */
        AlgoManager();

        /**
         * @brief Builds an algorithm based on the given name.
         * 
         * @param algo Name of the algorithm to build.
         * @return A shared pointer to the built algorithm.
         */
        std::shared_ptr<IAlgorithm> build_algo(std::string& algo);
};
