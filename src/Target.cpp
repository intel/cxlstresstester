/**

  Copyright (c) 2023, Intel Corporation 
  SPDX-License-Identifier: MIT

**/

#include <iostream>
#include <string>
#include <sstream>
#include <sys/mman.h>

#include "Target.h"
#include "AddressList.h"

//#define MAPSIZE 0x100000
#define MAP_HUGE_SHIFT 26
#define MAP_HUGE_MASK 0x3f
#define MAP_HUGE_2MB (21 << MAP_HUGE_SHIFT)
#define MAP_HUGE_1GB (30 << MAP_HUGE_SHIFT)
#define TARGET_LOGGER_ID       100

extern "C"
{
    #include <numa.h>
    #include <numaif.h>
    #include <fcntl.h>
    #include <unistd.h> 
}

Target::Target()
{
    mLogger = Logger::build();
    mID = 0xFFFFFFFF;
    mNodeID = 0xFFFF;
}

Target::Target(uint32_t id, uint16_t node_id)
{
    mLogger = Logger::build();
    mID = id;
    mNodeID = node_id;
}

Target::Target(uint32_t id, uint16_t node_id,
                uint64_t addrStart, uint16_t numSets, uint64_t setOffsetAddrIncr,
                uint16_t numAddrIncr, uint64_t addrIncr)
{
    mLogger = Logger::build();
    mLogger->print("Building new target.", TARGET_LOGGER_ID);
	mID = id;
	mNodeID = node_id;
	mAddrList = std::make_shared<AddressList>(addrStart, numSets, setOffsetAddrIncr,
			numAddrIncr, addrIncr);

	mAddrList->GenerateAddressList();
    mrequiredSize = mAddrList->GetSizeRequirements();
    mLogger->print("Required size is estimated to " + std::to_string(mrequiredSize) + " bytes.", TARGET_LOGGER_ID);

    uint64_t allocatedRegion = AllocateMemory(mrequiredSize);
    // Allocate in public target members
    this->address = allocatedRegion;
    this->size = mrequiredSize;
    std::stringstream ss;
    ss << "Allocated region starts at address 0x" << std::hex << allocatedRegion;
    mLogger->print(ss.str(), TARGET_LOGGER_ID);

    // Now, rebase address list to this region of memory
    mAddrList->RebaseAddressList(allocatedRegion);
    mLogger->print("Target built.", TARGET_LOGGER_ID);
}

uint64_t Target::operator[] (int index)
{
    AddressList *lAddrList = mAddrList.get();

    if (index < lAddrList->GetEntrySize()) {
        uint64_t retAddr = (*lAddrList)[index];
        return retAddr;
    } else {
        mLogger->print("Array index accessed out-of-range at " + std::to_string(index) + " max index is " + std::to_string(lAddrList->GetEntrySize()), TARGET_LOGGER_ID);
        throw std::invalid_argument("Array index out of bound.");
    }
}

std::shared_ptr<AddressList> Target::GetAddressList()
{
	return mAddrList;
}


uint64_t Target::AllocateMemory(uint64_t size)
{
	void* LogicalAddressCopy = (void*)-1; //Initializing the value do solve a Coverity scan issue
	nodemask_t tMask;
	numa_set_strict(1);
	nodemask_zero(&tMask);
    try{
        int maxnode = numa_num_configured_nodes ();
        if(mNodeID > maxnode-1)
        {
            throw std::runtime_error("ERROR NUMA Node/s Not enabled.");
        }

       nodemask_set_compat(&tMask, mNodeID); 

    } catch(...){
        mLogger->report_failure(" Please enable NUMA or select the right NUMA node and restart the test!.. Exiting !! ");
        exit(0);
    }
	numa_set_membind_compat(&tMask);

	if (size <= 0x200000)
	{
		LogicalAddressCopy = mmap(0, size, PROT_READ | PROT_WRITE, 
				MAP_PRIVATE | MAP_NORESERVE | MAP_ANONYMOUS | MAP_HUGETLB, 
				-1, 0);
	} else if ((size > 0x200000) && (size <= 0x40000000))
	{
		LogicalAddressCopy = mmap(0, size, PROT_READ | PROT_WRITE,
				MAP_PRIVATE | MAP_NORESERVE | MAP_ANONYMOUS | MAP_HUGETLB | MAP_HUGE_1GB,
				-1, 0);
	} else {
        mLogger->report_failure("Unable to allocate requested size exceeding 1 GB. size = " + std::to_string(size));
	}	

	if (LogicalAddressCopy == MAP_FAILED)
	{
        mLogger->report_failure("Unable to allocate " + std::to_string(MAPSIZE) + " Bytes of memory." +
                                "\nPlease make sure hugepages memory pool is allocated for Node " + std::to_string(mNodeID) + 
                                "\nExiting Test ...\n");
        exit(0);
	}
	TouchPages((uint64_t)LogicalAddressCopy,size);
    mlock((const void*)LogicalAddressCopy,(size_t)(size));

	return (uint64_t)LogicalAddressCopy;
}

void Target :: TouchPages(unsigned long LogicalAddress2, unsigned long MemorySpan)
{
    unsigned long j;
    unsigned long tempdata = 0;

    for(j = 0; j < MemorySpan; j += 0x40){
        tempdata = LogicalAddress2 + j;
        *((unsigned long *)(tempdata)) = tempdata;
    }
}

void Target :: UnMapMemory(unsigned long* vAddr, off_t mapSize)
{
    munmap(vAddr,mapSize);
}
