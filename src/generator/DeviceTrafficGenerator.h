/**

  Copyright (c) 2023, Intel Corporation 
  SPDX-License-Identifier: MIT

**/

#pragma once
#include <vector>

#include "ITrafficGenerator.h"
#include "algo/IAlgorithm.h"
#include "AddressList.h"

class DeviceTrafficGenerator : public ITrafficGenerator
{
	private:
		std::shared_ptr<AddressList> mpAddrList;
		std::shared_ptr<IAlgorithm> mpAlgo;
		uint32_t mSeg = 0, mBus = 0, mDev = 0, mFunc = 0;
		uint16_t mNumSets = 0, mNumAddrIncr = 0;
		uint32_t mSetOffsetAddrIncr = 0;
		uint64_t mAddrIncr = 0;
		uint32_t mPattern = 0;
		uint16_t mPatternSize = 0;
		uint16_t mPatternParameter = 0;
		// Need to move to proper places
		uint16_t mSize = 0, mOffset = 0;
		uint16_t mAlgoParams = 0;
		//
		uint16_t mLoops = 0;
		uint16_t mProtocol = 0;
		uint8_t mStartAddressCacheAligned = 0;
		void *mVirtAddr  = (void *) 0;
		void iterate_register(uint32_t raw_register, std::vector<std::string>& bit_fields);
		unsigned long long int MapPhyMemToVirtMem(uint64_t PhysicalAddrCopy, off_t MemorySpanCopy);
		unsigned long long GetPhysAddress(void *vaddr);
		unsigned int ConfigRead(uint32_t domain,uint32_t bus, uint32_t dev, 
				uint32_t func, uint32_t offset);
		int GetDvsecCapabilityOffset(uint32_t mDomain, uint32_t mBusNum, uint32_t mDevice, 
				uint32_t mFuncNum,uint8_t CapabilityID);

	public:
		DeviceTrafficGenerator();
		DeviceTrafficGenerator(std::shared_ptr<AddressList> addrList, 
				uint32_t pattern, uint16_t patternSize, uint16_t patternparam, 
				uint16_t segment, uint16_t bus, uint16_t dev, uint16_t func, uint16_t protocol);
		//
		virtual ret_t configure();
		virtual ret_t start();
		virtual ret_t stop();
		virtual ret_t check();
		virtual ret_t task();
		virtual void print();
		virtual void dump();
		//
		void setAddressList(std::shared_ptr<AddressList> addrList);
		void setAlgorithm(std::shared_ptr<IAlgorithm> algo);
		//
		void setOffset(uint16_t offset);
		void setSize(uint16_t size);
		void setLoops(uint16_t loops);
		void setPatternParam(uint16_t setpatternparam);
		void setAlgoParams(uint16_t algoparams);
		void setStartAddressCacheAligned(uint8_t StartAddressCacheAligned);
		void setProtocol(uint16_t protocol);
};
