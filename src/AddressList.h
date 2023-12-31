/**

  Copyright (c) 2023, Intel Corporation 
  SPDX-License-Identifier: MIT

**/

#pragma once

#include "cxl/CxlTypes.h"
#include "utils/Logger.h"

#define EXCEPTION_ID           505

class AddressList
{
	private:
		// Address List parameters as setup by the user
		uint64_t mAddrStart;
		uint16_t mNumAddrIncr;
		uint64_t mAddrIncr;
		uint16_t mNumSets;
		uint64_t mSetOffsetAddrIncr;
		// Number of valid entires in array list
		uint64_t mAddrEntriesSize = 0;
		// Raw addresses programmed in the array of uint64_t
		uint64_t *mAddrContents = 0;
		std::shared_ptr<Logger> mLogger;

	public:
		AddressList(uint64_t addrStart, uint16_t numSets, uint32_t setOffsetAddrIncr);
		AddressList(uint64_t addrStart, uint16_t numAddrIncr, uint64_t addrIncr);
		AddressList(uint64_t addrStart, uint16_t numSets, uint32_t setOffsetAddrIncr, 
				uint16_t numAddrIncr, uint64_t addrIncr);
		uint64_t& operator[](int index);
		ret_t GenerateAddressList();
		ret_t RebaseAddressList(int64_t baseAddr);
		uint64_t GetSizeRequirements();
		uint64_t GetEntrySize();
		uint64_t* GetListPtr(void);
		uint16_t GetNumSets();
		uint64_t GetSetOffsetAddrIncr();
		uint64_t GetNumAddrIncr();
		uint64_t GetSetAddrIncr();
		void Print();
		~AddressList();
};
