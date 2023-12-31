/**

  Copyright (c) 2023, Intel Corporation 
  SPDX-License-Identifier: MIT

**/

#pragma once
#include <iostream>
#include <vector>
#include <memory>
#include "cxl/CxlTypes.h"

#include "AddressList.h"
#include "utils/Logger.h"

#define MAPSIZE 0x100000

/**
 * @class Target
 * @brief The Target class provides functions to allocate memory, touch pages, unmap memory, and get address list.
 * Contains member variables mID and mNodeID and a shared pointer to an instance of the AddressList class.
 * Contains member functions to allocate memory, touch pages, unmap memory, and get a shared pointer to the address list.
*/
class Target
{
    private:
    uint32_t mID;
    uint16_t mNodeID;
    std::shared_ptr<AddressList> mAddrList;
    std::shared_ptr<Logger> mLogger;

    public:
        uint64_t mrequiredSize = 0;

        /**
         * @brief The constructor of Target class.
         * Initializes the member variables mID and mNodeID to 0xFFFFFFFF and 0xFFFF respectively.
         */
    	Target();

        /**
         * @brief The constructor of Target class with parameters.
         * Initializes the member variables mID and mNodeID to the passed values of id and node_id respectively.
         * @param id The id of the target.
         * @param node_id The node id of the target.
         */
        Target(uint32_t id, uint16_t node_id);

        /**
         * @brief The constructor of Target class with parameters.
         * Initializes the member variables mID and mNodeID to the passed values of id and node_id respectively.
         * Creates an instance of AddressList class and initializes it with the passed values of addrStart, numSets, setOffsetAddrIncr, numAddrIncr, and addrIncr.
         * Calls the GenerateAddressList() and RebaseAddressList() methods of AddressList class.
         * Calculates the required size and allocates the memory.
         * @param id The id of the target.
         * @param node_id The node id of the target.
         * @param addrStart The start address of the target.
         * @param numSets The number of sets of the target.
         * @param setOffsetAddrIncr The set offset address increment of the target.
         * @param numAddrIncr The number of address increments of the target.
         * @param addrIncr The address increment of the target.
         */
        Target(uint32_t id, uint16_t node_id,
            uint64_t addrStart, uint16_t numSets, uint64_t setOffsetAddrIncr,
            uint16_t numAddrIncr, uint64_t addrIncr);

        uint64_t address = 0, size = 0;

        /**
         * @brief Allocates memory with the given size.
         * 
         * This function allocates memory with the given size. If the size is <= 2 MB, it uses the huge page size of 2 MB.
         * If the size is between 2 MB and 1 GB, it uses the huge page size of more than 2 MB. 
         * If the size is > 1 GB, it reports an error message.
         * 
         * @param size Size of the memory to be allocated.
         * @return uint64_t Logical address of the allocated memory.
         */
        uint64_t AllocateMemory(uint64_t size);

        /**
         * @brief Accesses the address list at a given index.
         * If the index is out-of-bound, function throws an invalid argument exception.
         * 
         * @param index Index of the address to be accessed in the address list.
         * @return uint64_t The address at the given index.
         */
        uint64_t operator[](int index);

        /**
         * @brief Unmaps the memory at the given address.
         * @param vAddr The address to be unmapped.
         * @param mapSize The size of memory to be unmapped.
         */
        void UnMapMemory(unsigned long* vAddr,off_t mapSize);

        /**
         * @brief Gets the shared pointer to address list.
         * @return std::shared_ptr<AddressList> Shared pointer to the address list.
         */
        std::shared_ptr<AddressList> GetAddressList();

        /**
         * @brief Touches the pages starting from the `LogicalAddress2` and going through `MemorySpan` bytes.
         * @param LogicalAddress2 The start address of the memory region to touch.
         * @param MemorySpan The size of the memory region to touch in bytes.
         */
        void TouchPages(unsigned long LogicalAddress2, unsigned long MemorySpan);
};
