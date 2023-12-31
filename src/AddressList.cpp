/**

  Copyright (c) 2023, Intel Corporation 
  SPDX-License-Identifier: MIT

**/

#include <iostream>
#include <sstream>

#include "AddressList.h"


AddressList::AddressList(uint64_t addrStart, uint16_t numSets, uint32_t setOffsetAddrIncr) : AddressList(addrStart, numSets, setOffsetAddrIncr, 0x0, 0x0)
{
}

AddressList::AddressList(uint64_t addrStart, uint16_t numAddrIncr, uint64_t addrIncr) : AddressList(addrStart, 0x0, 0x0, numAddrIncr, addrIncr)
{
}

AddressList::AddressList(uint64_t addrStart, uint16_t numSets, uint32_t setOffsetAddrIncr,
				uint16_t numAddrIncr, uint64_t addrIncr)
{
	mLogger = Logger::build();
	try{	
		/* Check number of sets is bigger than 1 */
		if (numSets < 1){
			mLogger->report_failure("--num-sets parameter must be equal or higher to 1.");
			throw EXCEPTION_ID;
		}
		/* Check number of address increments is equal or bigger than 0 */
		else if (numAddrIncr < 0){
			mLogger->report_failure("--num-addr-incr parameter must be equal or higher to 0.");
			throw EXCEPTION_ID;
		}
		/* Check address increment can only be 0 if number of addres increments is 1 */
		else if (addrIncr < 1 && numAddrIncr > 1){
			mLogger->report_failure("--addr-incr parameter cannot be 0 when number of address increments is higher than 1.");
			throw EXCEPTION_ID;
		}
		/* Check for overlaps */
		else if (setOffsetAddrIncr < (numAddrIncr * addrIncr)) {
			std::stringstream ss;
			ss << "setOffsetAddrIncr :0x" << std::hex << setOffsetAddrIncr << "  numAddrIncr: 0x" << std::hex<< numAddrIncr << "\n    addrIncr: 0x" << addrIncr;
			mLogger->report_failure(ss.str());
	
			mLogger->report_failure("Overlap detected in AddressList.");
			throw EXCEPTION_ID;
		}
	}
	catch(...){
		mLogger->report_failure("Please fix the CXL hammer parameter which is mentioned above, exiting Bye!.");
		exit(1);
	}

	

	this->mAddrStart = addrStart;
	this->mNumSets = numSets;
	this->mSetOffsetAddrIncr = setOffsetAddrIncr;
	this->mNumAddrIncr = numAddrIncr;
	this->mAddrIncr = addrIncr;
}

ret_t AddressList::GenerateAddressList()
{
	//
	uint32_t idx = 0;

	uint64_t addrNumBlocks = mNumSets * mSetOffsetAddrIncr;
	// Add an extra block for buffer purposes
	if (addrNumBlocks) addrNumBlocks += 1;

	if (mNumAddrIncr != 0) {

		mAddrEntriesSize = (uint64_t)(mNumSets * mNumAddrIncr);

		// Allocate the content area to store the address contents
		mAddrContents = new uint64_t[(unsigned int)mAddrEntriesSize];

		uint16_t curSets = 0, curLoop = 0;
		for (uint64_t outAddr = mAddrStart; curSets < mNumSets; outAddr += mSetOffsetAddrIncr, curSets++) {
			curLoop = 0;
			for (uint64_t inAddr = outAddr; curLoop < mNumAddrIncr; inAddr += mAddrIncr, curLoop++) {
				mAddrContents[idx++] = inAddr;
			}
		}

	} else {
		mAddrEntriesSize = mNumSets;

		// Allocate the content area to store the address contents
		mAddrContents = new uint64_t[(unsigned int)mAddrEntriesSize];

		uint16_t curSets = 0; // , curLoop = 0;
		for (uint64_t outAddr = mAddrStart; curSets < mNumSets; outAddr += mSetOffsetAddrIncr, curSets++) {
			mAddrContents[idx++] = outAddr;
		}

	}
	// TODO: fix this , function can be void?
	return 0;
}

uint64_t& AddressList::operator[] (int index) {
	if (index < mAddrEntriesSize) {
		return mAddrContents[index];
	} else {
		mLogger->print("Array index accessed out-of-range at " + std::to_string(index) + " max index is " + std::to_string(mAddrEntriesSize), 1);
		throw std::invalid_argument("Array index out of bound.");
	}
}

ret_t AddressList::RebaseAddressList(int64_t baseAddr)
{
	for (uint32_t idx = 0; idx < mAddrEntriesSize; idx++) {
		mAddrContents[idx] += baseAddr;
	}

	return 0;
}

uint64_t AddressList::GetSizeRequirements()
{
	// Always return at least one $line size (0x40)
	return (0x40 + mAddrContents[GetEntrySize()-1] - mAddrStart);
}

uint64_t AddressList::GetEntrySize()
{
	return (mAddrEntriesSize);
}

uint64_t* AddressList::GetListPtr(void)
{
	return (mAddrContents);
}

uint16_t AddressList::GetNumSets(void)
{
	return (mNumSets);
}

uint64_t AddressList::GetSetOffsetAddrIncr(void)
{
	return (mSetOffsetAddrIncr);
}

uint64_t AddressList::GetNumAddrIncr(void)
{
	return (mNumAddrIncr);
}

uint64_t AddressList::GetSetAddrIncr(void)
{
	return (mAddrIncr);
}

void AddressList::Print()
{
	std::stringstream ss;
	ss << "+ AddressList Information\n";
	ss << "  - Num Sets        :   " << std::dec << mNumSets << '\n';
	ss << "  - SetOffset Incr  : 0x" << std::hex << mSetOffsetAddrIncr << '\n';
	ss << "  - Num Addr Incr   :   " << std::dec << mNumAddrIncr << '\n';
	ss << "  - Add Incr        : 0x" << std::hex << mAddrIncr << '\n';
	ss << "  + Address Sequence:";
	mLogger->print(ss.str(), 2);
	ss.str(std::string());


	for (uint32_t idx = 0; idx < mAddrEntriesSize; idx++) {
		ss << "    [" << std::dec << idx <<"]:\t0x" << std::hex << mAddrContents[idx]<< std::endl;
	}
	mLogger->print(ss.str(), 2);
}

AddressList::~AddressList()
{
}
