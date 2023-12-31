/**

  Copyright (c) 2023, Intel Corporation 
  SPDX-License-Identifier: MIT

**/

#include "MulWrStream.h"
#include <iostream>
#include <sstream>
#include <thread>
#include <chrono>

#include <x86intrin.h>

ret_t MulWr64::run()
{
	return 0;
}

ret_t MulWr64::verify()
{
	return 0;
}

ret_t MulWr32::run()
{
	return 0;
}

ret_t MulWr32::verify()
{
	return 0;
}

inline void MulWrStreamNew::FlushStage()
{
	uint64_t addr;
	
	for (uint32_t idx = 0; idx < mpAddrList->GetEntrySize(); idx++) {
		addr = (*mpAddrList)[idx] + mOffset;
		asm volatile ("clflush (%0)" :: "r"(addr));
	}
}

inline void MulWrStreamNew::WriteStage()
{
	uint64_t addr;
	uint8_t writeType = (mParams & 0xF0) >> 4;

	if (writeType == 0) return;

	for (uint32_t idx = 0; idx < mpAddrList->GetEntrySize(); idx++) {
		std::stringstream ss;
		addr = (*mpAddrList)[idx] + mOffset;
		if (writeType == 1 && mSize == 4) {
			uint32_t pattern = mPattern & 0xFFFFFFFF;
			asm volatile ("movnti %0, (%1)" :
							: "r"(pattern), "r"(addr)
							: "memory");
		} else if (writeType == 2 && mSize == 8) {
			uint64_t pattern;
			pattern = (((mPattern << 32UL) & 0xFFFFFFFFFFFFFFFF) | (mPattern & 0xFFFFFFFF));
			asm volatile ("movq %0, (%1)"   :
							: "r"(pattern), "r"(addr)
							: "memory");
		} else if (writeType == 2 && mSize == 4) {
			uint32_t pattern = mPattern & 0xFFFFFFFF;
			asm volatile ("movl %0, (%1)"   :
							: "r"(pattern), "r"(addr)
							: "memory");
		} else if (writeType == 2 && mSize == 2) {
			uint16_t pattern = mPattern & 0xFFFF;
			asm volatile ("movw %0, (%1)"  :
							: "r"(pattern), "r"(addr)
							: "memory");
		} else if (writeType == 2 && mSize == 1) {
			uint8_t pattern = mPattern & 0xFF;
			asm volatile ("movb %0, (%1)" :
							: "r"(pattern), "r"(addr)
							: "memory");
		} else {
			ss << "Do nothing for 0x" << std::hex << addr <<  std::endl;
			mLogger->print(ss.str(), 2);
		}
		ss.str(std::string());
	}
}

inline ret_t MulWrStreamNew::ReadStage()
{
	uint64_t addr;
	uint8_t readType = (mParams & 0xF000) >> 12;

	if (readType == 0) return 0;

	for (uint32_t idx = 0; idx < mpAddrList->GetEntrySize(); idx++) {
		addr = (*mpAddrList)[idx] + mOffset;
		std::stringstream ss;
		if (readType == 2 && mSize == 8) {
			uint64_t readPattern;
			asm volatile ("movq (%1), %0"   : "=r"(readPattern)
							: "r"(addr)
							: "memory");
			uint64_t pattern = (((mPattern << 32UL) & 0xFFFFFFFFFFFFFFFF) | (mPattern & 0xFFFFFFFF));
			if (readPattern != pattern) {
				//throw std::runtime_error("Value mismatch.");
				ss << "Value mismatch in MOVQ. ReadPattern=0x"<< std::hex << readPattern << ", ExpectedPattern=0x" <<
					mPattern << std::endl;
				mLogger->report_failure(ss.str());
				return -1;
			}
		} else if (readType == 2 && mSize == 4) {
			uint32_t readPattern;
			asm volatile ("movl (%1), %0"   : "=r"(readPattern)
							: "r"(addr)
							: "memory");
			if (readPattern != (mPattern & 0xFFFFFFFF)) {
				ss << "Value mismatch in MOVL. ReadPattern=0x" << std::hex << readPattern << ", ExpectedPattern=0x" <<
					(mPattern & 0xFFFFFFFF) << std::endl;
				mLogger->report_failure(ss.str());
				return -1;
			}
		} else if (readType == 2 && mSize == 2) {
			uint16_t readPattern;
			asm volatile ("movw (%1), %0"   : "=r"(readPattern)
							: "r"(addr)
							: "memory");
			if (readPattern != (mPattern & 0xFFFF)) {
				ss << "Value mismatch in MOVW. ReadPattern=0x" << std::hex << readPattern << ", ExpectedPattern=0x" <<
					(mPattern & 0xFFFF) << std::endl;
				mLogger->report_failure(ss.str());
				return -1;
			}
		} else if (readType == 2 && mSize == 1) {
			uint8_t readPattern;
			asm volatile ("movb (%1), %0"   : "=r"(readPattern)
							: "r"(addr)
							: "memory");
			if (readPattern != (mPattern & 0xFF)) {
				ss << "Value mismatch in MOVB. ReadPattern=0x" << std::hex << readPattern << ", ExpectedPattern=0x" <<
				(uint16_t)(mPattern & 0xFF) << std::endl;
				mLogger->report_failure(ss.str());
				return -1;
			}
		} else {
			ss << "Do nothing for 0x" << std::hex << addr << std::endl;
			mLogger->print(ss.str(), 2);
			return -1;
		}
		ss.str(std::string());
	}
	return 0;
}

MulWrStreamNew::MulWrStreamNew(uint32_t params, uint64_t pattern, uint8_t offset, uint8_t size)
{
	mParams = params;
	mPattern = pattern;
	mOffset = offset;
	mSize = size;
}

uint8_t MulWrStreamNew::GetOffset()
{
	return mOffset;
}

uint8_t MulWrStreamNew::GetSize()
{
	return mSize;
}

ret_t MulWrStreamNew::run()
{
	ret_t retCode = 0;
	uint8_t flushType, writeType, readType;

	// CLFlush stage
	flushType = mParams & 0xF;
	if (flushType) {
		FlushStage();
	}
	// Write stage
	writeType = (mParams & 0xF0) >> 4;
	if (writeType != 0) {
		WriteStage();
	}

	// CLFlush stage
	flushType = (mParams & 0xF00) >> 8;
	if (flushType) {
		FlushStage();
	}

	// Read stage
	readType = (mParams & 0xF000) >> 12;
	if (readType) {
		retCode = ReadStage();
	}

	return retCode;
}

