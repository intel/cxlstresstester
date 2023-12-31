/**

  Copyright (c) 2023, Intel Corporation 
  SPDX-License-Identifier: MIT

**/

#include <iostream>
#include <thread>
#include <chrono>
#include <vector>

#include <sched.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/io.h>
#include <sys/mman.h>
#include <string.h>
#include <sstream>

#include "DeviceTrafficGenerator.h"
#include "algo/MulWrStream.h"

extern "C" {
#include <pci/pci.h>
}

#define DEV_CAP_ERRORLOG1                                      0x40
#define DEV_CAP_ERRORLOG3                                      0x50
#define CONFIG_TEST_ADDR_INCRE_OFF                             0x10
#define CONFIG_TEST_PATTERN_OFF                                0x18
#define CONFIG_TEST_BYTEMASK_OFF                               0x20
#define CONFIG_TEST_PATTERN_PARAM_OFF                          0x28
#define CONFIG_ALGO_SETTING_OFF                                0x30
#define DEVICE_AFU_STATUS1_OFF                                 0x98
#define DEVICE_GENERATOR_LOGGER_ID                             50
#define TC1BF                            {"Self Checking Supported",\
                                          "Algorithm 1a Supported",\
                                          "Algorithm 1b Supported",\
                                          "Algorithm 2 Supported",\
                                          "RdCurr Supported ",\
                                          "RdOwn Supported ",\
                                          "RdShared Supported ",\
                                          "RdAny Supported ",\
                                          "RdOwnNoData Supported ",\
                                          "ItoMWr Supported ",\
                                          "MemWr Supported ",\
                                          "CLFlush Supported ",\
                                          "CleanEvict Supported ",\
                                          "DirtyEvict Supported ",\
                                          "CleanEvictNoData Supported ",\
                                          "WoWrInv Supported ",\
                                          "WoWrInvF Supported ",\
                                          "WrInv Supported ",\
                                          "CacheFlushed Supported ",\
                                          "UnexpectedCompletion Supported ",\
                                          "CompletionTimeoutRejection Supported ",\
                                          "Reserved",\
                                          "Reserved",\
                                          "Reserved",\
                                          "ConfigurationSize bit1",\
                                          "ConfigurationSize bit2",\
                                          "ConfigurationSize bit3",\
                                          "ConfigurationSize bit4",\
                                          "ConfigurationSize bit5",\
                                          "ConfigurationSize bit6",\
                                          "ConfigurationSize bit7",\
                                          "ConfigurationSize bit8"\
                                         }
#define ACCESS_ERROR	                                           ((void *) -1)

void DeviceTrafficGenerator::iterate_register(uint32_t raw_register, std::vector<std::string>& bit_fields){
    uint64_t bit = 0;
    while(raw_register!=0x0){
        if (raw_register&0x1){
            mLogger->log_action(bit_fields[bit], DEVICE_GENERATOR_LOGGER_ID, this->mBus);
        }
        raw_register = (raw_register >> 1);
        ++bit;
    }
}

DeviceTrafficGenerator::DeviceTrafficGenerator()
{
	mSeg = mBus = mDev = mFunc = 0;
	mOffset = 0;
	mSize = 0;
}

DeviceTrafficGenerator::DeviceTrafficGenerator(std::shared_ptr<AddressList> addrList,
		uint32_t pattern, uint16_t patternSize, uint16_t patternparam,
		uint16_t segment, uint16_t bus, uint16_t dev, uint16_t func, uint16_t protocol_id)
{
	std::stringstream ss;
	uint64_t barAddr;

	mpAddrList = std::move(addrList);
	mPattern = pattern;
	mPatternSize = patternSize;
	mOffset = 0;
	mSize = 0;
	mLoops = 0;
	mPatternParameter = 0;
    mProtocol = protocol_id;

	//need to remove the below line(s)
	ss << "Setting device bus 0x" << std::hex << bus <<".";
	mLogger->print(ss.str(), DEVICE_GENERATOR_LOGGER_ID);
	ss.str(std::string());

	mSeg = segment;
	mBus = bus;
	mDev = dev;
	// Override function as always 0
	mFunc = 0;

	mLogger->print("Searching for DVSEC Capability ID 0xA (Test capability)...", DEVICE_GENERATOR_LOGGER_ID);
	uint32_t testOffset = GetDvsecCapabilityOffset(0, mBus, 0, 0, 0xA);
	try{
       	if (testOffset == -1) {
    		mLogger->report_failure("Bus number is not found. Erring ...");
    		throw std::runtime_error("Bus number is not found");
    	} else if(testOffset == 0) {
    		mLogger->report_failure("DVSEC Test Capability is not found. Erring ...");
    		throw std::runtime_error("DVSEC Test Capability is not found");
    	}
    }
    catch(...){
    ss <<"Exiting...."<<std::endl;
	ss.str(std::string());
    exit(1);
    }

    ss << "Found DVSEC capability at testOffset 0x" << std::hex << testOffset;
    mLogger->print(ss.str(), DEVICE_GENERATOR_LOGGER_ID);
    ss.str(std::string());

    uint32_t CXLTestCapability1 = ConfigRead(0, mBus, 0, 0, testOffset + 0xC);
    ss << "CXL Test Capability 1: 0x" << std::hex << CXLTestCapability1;
    mLogger->log_action(ss.str(), DEVICE_GENERATOR_LOGGER_ID, this->mBus);
    ss.str(std::string());

    std::vector<std::string> test_capability_1_bit_fields = TC1BF;
    iterate_register(CXLTestCapability1, test_capability_1_bit_fields);

    uint32_t ConfigurationSize = (CXLTestCapability1 & 0xFF000000) >> 24;
    ss << "ConfigurationSize: 0x" << std::dec << ConfigurationSize;
    mLogger->log_action(ss.str(), DEVICE_GENERATOR_LOGGER_ID, this->mBus);
    ss.str(std::string());

    uint64_t CXLTestConfigurationBaseLow = ConfigRead(0, mBus, 0, 0, testOffset + 0x14);
    ss << "CXLTestConfigurationBaseLow: 0x" << std::hex << CXLTestConfigurationBaseLow;
    mLogger->log_action(ss.str(), DEVICE_GENERATOR_LOGGER_ID, this->mBus);
    ss.str(std::string());

    uint64_t CXLTestConfigurationBaseHigh = ConfigRead(0, mBus, 0, 0, testOffset + 0x18);
    ss << "CXLTestConfigurationBaseHigh: 0x" << std::hex << CXLTestConfigurationBaseHigh;
    mLogger->log_action(ss.str(), DEVICE_GENERATOR_LOGGER_ID, this->mBus);
    ss.str(std::string());

	if (((CXLTestConfigurationBaseLow >> 1) & 0xFFF) == 0x00) {
		barAddr = CXLTestConfigurationBaseLow;
	} else {
        barAddr = (CXLTestConfigurationBaseHigh << 32);
        barAddr |= CXLTestConfigurationBaseLow & ~(0xFFF);
	}

	mVirtAddr = (void *)MapPhyMemToVirtMem( barAddr, ConfigurationSize);
	uint64_t unAlOffset = (CXLTestConfigurationBaseLow & 0xFF0);
	mVirtAddr = (void *)((uint64_t)mVirtAddr + unAlOffset);
}

unsigned int DeviceTrafficGenerator::ConfigRead(uint32_t domain,uint32_t bus, uint32_t dev, uint32_t func, uint32_t offset)
{
    std::stringstream ss;
    unsigned long val;
    struct pci_access *AllDevices;
    struct pci_dev *PcieDeviceConfigSpace;
    AllDevices = pci_alloc();                /* Create a pointer to PCIe devices */
    pci_init(AllDevices);                    /* Initialize the pointer */
    pci_scan_bus(AllDevices);                /* Scan the PCIe Bus and populate the pointer */
    PcieDeviceConfigSpace = pci_get_dev(AllDevices,domain,bus,dev,func);   /* Get the pointer to specific B:D:F */
    
    if (!PcieDeviceConfigSpace)
    {
        ss << "Error! Couldn't locate the bus :"<< std::hex<<bus;
        ss.str (std::string());
        return -1;
    }
    val = pci_read_long(PcieDeviceConfigSpace,offset); /* Read value at specific offset */

    return val;
}

int DeviceTrafficGenerator::GetDvsecCapabilityOffset(uint32_t mDomain, uint32_t mBusNum, uint32_t mDevice, uint32_t mFuncNum,uint8_t CapabilityID)
{
    std::stringstream ss;
    uint32_t i = 0x0;
    uint32_t NextPtr = 0x100;
    uint32_t RegVal = 0;
    /* ****** Check if DVSEC Capability Exists and return Capability offset ****** */

    while(NextPtr != 0x000)
    {
        i = NextPtr;
        uint32_t t = ConfigRead(mDomain,mBusNum,mDevice,mFuncNum,i);        
        if(t==0xffffffff)
        {
            ss <<"Config read failure! exiting "<<std::endl;
			ss.str (std::string());
            return -1;
        }
             
        ss << "t=0x" << std::hex << t;
        //mLogger->print(ss.str(), DEVICE_GENERATOR_LOGGER_ID);
        ss.str(std::string());
        if((ConfigRead(mDomain,mBusNum,mDevice,mFuncNum,i) & 0xFFFFF) == 0x10023)
        {
            RegVal = ConfigRead(mDomain,mBusNum,mDevice,mFuncNum,(i+8));
            ss << "RegVal=0x" << std::hex << RegVal;
            //mLogger->print(ss.str(), DEVICE_GENERATOR_LOGGER_ID);
            ss.str(std::string());
            if ((RegVal & 0xFF) == CapabilityID)
               return i;
        }
        NextPtr = ConfigRead(mDomain,mBusNum,mDevice,mFuncNum,i);
        NextPtr = NextPtr >> 20;
    }

    return 0;
}

unsigned long long int DeviceTrafficGenerator::MapPhyMemToVirtMem(uint64_t PhysicalAddrCopy, off_t MemorySpanCopy)
{
    std::stringstream ss;
    uint64_t LogicalAddrCopy;
    int pagesize,fd;
    
    /* Check for appropriate IO permissions */
    if (iopl(3)) {
        mLogger->print("Cannot get I/O permissions (try running as root)\n", DEVICE_GENERATOR_LOGGER_ID);
        return -1;
    }

    /* Get the number of bytes in a page */
    pagesize = getpagesize();
    ss.str(std::string());

    /* Open system memory as a read/write file */
    fd = open("/dev/mem", O_RDWR);
    if(fd == (unsigned long long) ACCESS_ERROR)
    {
        perror("Open System memory as read/write failed!.\n");
        return -3;
    }

    /*
     * Map from the /dev/mem file, offset by the Physical Address
     * Map for memoryspan + pagesize bytes
     * logical address points at this processes memory space mapped to the
     * shared memory
     */
    ss << "Mapping 0x" << std::hex << (MemorySpanCopy + pagesize) << " bytes to physical address 0x" << PhysicalAddrCopy;
    mLogger->log_action(ss.str(), DEVICE_GENERATOR_LOGGER_ID, this->mBus);
    ss.str(std::string());

    LogicalAddrCopy = (unsigned long long) mmap(NULL, (MemorySpanCopy + pagesize),
            PROT_READ|PROT_WRITE, MAP_SHARED, fd, (off_t)PhysicalAddrCopy);

    /* Check if the memory map worked */
    if(LogicalAddrCopy == (unsigned long long)MAP_FAILED)
    {
        perror("Mapping Virtual Memory to Physical Memory Failed.\n");
        close(fd);
        return -3;
    }
    else {
        ss << "Virtual Address: 0x" << std::hex << LogicalAddrCopy;
        mLogger->log_action(ss.str(), DEVICE_GENERATOR_LOGGER_ID, this->mBus);
        ss.str(std::string());
    }
    close(fd);

    return LogicalAddrCopy;
}

unsigned long long DeviceTrafficGenerator::GetPhysAddress(void *vaddr)
{
    unsigned long long addr;
    static int pagemap_fd=-1;
    int pid = getpid();
    std::stringstream filedescstream;
    filedescstream << "/proc/";
    filedescstream << std::dec << pid;
    filedescstream << "/pagemap";
    std::string filedesc;
    filedescstream >> filedesc;

    if (pagemap_fd==-1)
    {
        pagemap_fd = open(filedesc.c_str(), O_RDONLY);
        if(pagemap_fd == (unsigned long long) ACCESS_ERROR)
        {
            perror("Open file read failed!.\n");
            return -3;
        }        
    }
    int n = pread(pagemap_fd, &addr, 8, ((unsigned long long)vaddr / 4096) * 8);
    if (n != 8)
        return 0;
    if (!(addr & (1ULL<<63)))
        return 0;
    addr &= (1ULL<<54)-1;
    addr <<= 12;
    return addr + ((unsigned long long)vaddr  & (4096-1));
}

ret_t DeviceTrafficGenerator::configure()
{
	uint64_t ByteMask=0x0, AndByteMask=0x0, OrByteMask=0x0, Register7Data;
	uint64_t startAddress1=0x0UL;
    std::stringstream ss;


    mLogger->log_action("Start CCV AFU configuration.", DEVICE_GENERATOR_LOGGER_ID, this->mBus);

	mNumSets = mpAddrList->GetNumSets();
	mSetOffsetAddrIncr = (mpAddrList->GetSetOffsetAddrIncr() >> 6);     // Right shifting set offset by 6 bits as per spec
    //CCV AFU doesnt count first write, need to substract one to match exact writes from CPUS
	mNumAddrIncr = (mpAddrList->GetNumAddrIncr()-1);
	mAddrIncr = (mpAddrList->GetSetAddrIncr() >> 6 );                   // Right shifting set offset by 6 bits as per spec

    ss << std::hex << "mNumSets: " << mNumSets << ", mSetOffsetAddrIncr: 0x" << mSetOffsetAddrIncr << ", mNumAddrIncr: 0x" << mNumAddrIncr << ", mAddrIncr: 0x" << mAddrIncr;
    //mLogger->print(ss.str(), DEVICE_GENERATOR_LOGGER_ID);
    ss.str(std::string());

    if (mStartAddressCacheAligned) {
    	startAddress1 = (*mpAddrList)[0]; // Should be always aligned to 64-bytes
    	for (int idx = mOffset; idx < (mOffset + mSize); idx++) {
    		ByteMask |= (0x1ULL << idx);
    	}
    }
	else {
		startAddress1 = (*mpAddrList)[0] + mOffset;
		if ((mOffset % 4) == 0) {

		}
		else if ((mOffset % 2) == 0) {
			if (mPatternSize == 4) {
                mLogger->report_failure("PatternSize is invalid. mPatternSize=" + std::to_string(mPatternSize) + " not matching with mOffset=" + std::to_string(mOffset));
				throw std::runtime_error("PatternSize mismatch with Offset.");
			}
		}
		else { // mOffset is aligned with 1 byte
			if (mPatternSize != 1) {
                mLogger->report_failure("PatternSize is invalid. mPatternSize=" + std::to_string(mPatternSize) + " not matching with mOffset=" + std::to_string(mPatternSize));
				throw std::runtime_error("PatternSize mismatch with Offset.");
			}
		}

		for (int idx = 0; idx < mOffset; idx++) {
			AndByteMask |= (0x1ULL << idx);
		}

		for (int idx = mOffset; idx < mOffset + mSize; idx++) {
			OrByteMask |= (0x1ULL << idx);
		}

		ByteMask &= ~(AndByteMask);
		ByteMask |= OrByteMask;
	}

	uint8_t WriteSemanticsOpcode = mAlgoParams & 0xF;
	uint8_t VerifySemanticsOpcode = (mAlgoParams & 0xF0) >> 4;
	uint64_t Register1, Register3, Register4, Register5, Register6, Register7;
	
	Register1 = startAddress1;
	Register3 = ((uint64_t)mSetOffsetAddrIncr << 32) | mAddrIncr;   
	Register4 = mPattern;
	Register5 = ByteMask;
	Register6 = ((uint64_t)mPatternParameter << 3) | mPatternSize;
	Register7 = (1UL << 3) |
			(((mNumAddrIncr & 0xFF) << 8) & 0xFFFF) |
			(((mNumSets & 0xFF) << 16) & 0xFFFFFF) |
			(((mLoops & 0xFF) << 24) & 0xFFFFFFFF) |
			((((uint64_t)mProtocol & 0xF) << 33) & 0xFFFFFFFFFF) |
			((((uint64_t)WriteSemanticsOpcode & 0xF) << 36) & 0xFFFFFFFFFF) |
			((((uint64_t)VerifySemanticsOpcode & 0x7) << 44) & 0x3FFFFFFFFFFF);
	
    ss << std::endl << "| \tCCV AFU Registers:" << std::endl;
    ss << "| \tRegister1 (StartAddress1)  : 0x" << GetPhysAddress((void*)Register1) << std::endl;
    ss << "| \tRegister3 (Increment)      : 0x" << Register3 << std::endl;
    ss << "| \t- AddressIncrement: 0x" << mAddrIncr << " (0x"<< (mAddrIncr<<6) << ")" << std::endl;
    ss << "| \t- SetOffset: 0x" << mSetOffsetAddrIncr << " (0x"<< (mSetOffsetAddrIncr<<6) << ")" << std::endl;
    ss << "| \tRegister4 (Pattern)        : 0x" << Register4 << std::endl;
    ss << "| \t- Pattern1: 0x" << ((0xFFFFFFFF)&mPattern) << std::endl;
    //ss << "| \t- Pattern2: 0x" << ((0xFFFFFFFF)&(mPattern>>32)) << std::endl;
    ss << "| \tRegister5 (ByteMask)       : 0x" << Register5 << std::endl;
    ss << "| \t- ByteMask: 0x" << ByteMask << " (cache-line offset: 0x" << mOffset << ")" << std::endl;
    ss << "| \tRegister6 (PatternSize)    : 0x" << Register6 << std::endl;
    ss << "| \t- PatternSize: 0x" << mPatternSize << std::endl;
    ss << "| \t- PatternParameter: 0x" << mPatternParameter << std::endl;
    ss << "| \tRegister7 (AlgorithmConf)  : 0x" << Register7 << std::endl;
    ss << "| \t- Algorithm: Algorithm1a" << std::endl;
    ss << "| \t- SelfChecking: 0x1" << std::endl;
    ss << "| \t- NumberOfAddrIncrements: 0x" << mNumAddrIncr << std::endl;
    ss << "| \t- NumberOfSets: 0x" << mNumSets << std::endl;
    ss << "| \t- NumberOfLoops: 0x" << mLoops << std::endl;
    ss << "| \t- Protocol ID: 0x" << mProtocol << std::endl;
    ss << "| \t- WriteSemanticsCache: 0x" << (uint64_t)WriteSemanticsOpcode << std::endl;
    ss << "| \t- VerifyReadSemanticsCache: 0x" << (uint64_t)VerifySemanticsOpcode;
    mLogger->log_action(ss.str(), DEVICE_GENERATOR_LOGGER_ID, this->mBus);
    ss.str(std::string());
	
	
	*(uint64_t*)(mVirtAddr) = GetPhysAddress((void*)Register1);
	*(uint64_t*)((char*)mVirtAddr + CONFIG_TEST_ADDR_INCRE_OFF) = Register3;
	*(uint64_t*)((char*)mVirtAddr + CONFIG_TEST_PATTERN_OFF) = Register4;
	*(uint64_t*)((char*)mVirtAddr + CONFIG_TEST_BYTEMASK_OFF) = Register5;
	*(uint64_t*)((char*)mVirtAddr + CONFIG_TEST_PATTERN_PARAM_OFF) = Register6;
	*(uint64_t*)((char*)mVirtAddr + CONFIG_ALGO_SETTING_OFF) = Register7;
    // Clear errors
    *(uint64_t*)((char*)mVirtAddr + DEV_CAP_ERRORLOG3) = 0x0;

    mLogger->log_action("CCV AFU configuration completed.", DEVICE_GENERATOR_LOGGER_ID, this->mBus);
	return 0;
}

ret_t DeviceTrafficGenerator::start()
{
    mLogger->log_action("Start.", DEVICE_GENERATOR_LOGGER_ID, this->mBus);
	*(uint64_t*)((char*)mVirtAddr + 0x30) |= 0x1;
    mLogger->log_action("Device is running.", DEVICE_GENERATOR_LOGGER_ID, this->mBus);
	return 0;
}

ret_t DeviceTrafficGenerator::stop()
{
    mLogger->log_action("Stopping.", DEVICE_GENERATOR_LOGGER_ID, this->mBus);
	*(uint64_t*)((char*)mVirtAddr + 0x30) &= (0xFFFFFFFFFFFFFFF8);
	return 0;
}

void DeviceTrafficGenerator::dump()
{
    std::stringstream ss;
	uint64_t RegisterData;
	for (int idx=0; idx<168; idx+=8)
	{
		RegisterData = *(uint64_t*)((char*)mVirtAddr + idx);
		std::stringstream ss;
		ss << "Register at offset: 0x" << std::hex << idx << " Data: 0x" << RegisterData << std::endl;
	}
	mLogger->log_action(ss.str(), DEVICE_GENERATOR_LOGGER_ID, this->mBus);
}

ret_t DeviceTrafficGenerator::check()
{
	int64_t Register9, Register10, Register11;
	int64_t expectedPattern, actualPattern;
	int64_t errorLog3, errorLog1, deviceStatusReg1;
    uint64_t LoopNum, ByteOffst;
	std::stringstream ss;

	mLogger->log_action("Verifying results...", DEVICE_GENERATOR_LOGGER_ID, this->mBus);

	errorLog3 = *(uint64_t*)((char*)mVirtAddr + DEV_CAP_ERRORLOG3);
	//mLogger->print("Error Log3: 0x" + std::to_string(errorLog3), 1);

	if (((errorLog3 >> 16) & 0x1) != 0)
	{
		mLogger->print("Data miscompare observed.", 1000);
        ByteOffst = ((uint64_t)errorLog3) & 0xFF;
        LoopNum = ((uint64_t)errorLog3 >> 8) & 0xFF;
		errorLog1 = *(uint64_t*)((char*)mVirtAddr + DEV_CAP_ERRORLOG1);
		expectedPattern = (errorLog1 & 0xFFFFFFFF);

		actualPattern = ((uint64_t)errorLog1 >> 32) & 0xFFFFFFFF;

		ss << std::endl << std::hex << "| - Expected pattern : 0x" << expectedPattern << std::endl;
		ss << "| - Actual pattern : 0x" << actualPattern << std::endl;
        ss << "| - ByteOffset: 0x" << ByteOffst << std::endl;
        ss << std::dec <<"| - LoopNum: " << LoopNum;
		mLogger->print(ss.str(), 1000);
		return -1;
	}
	else {
		deviceStatusReg1 = *(uint64_t*)((char*)mVirtAddr + 0x98);
		ss << std::hex << "Device Status Register 1: 0x" << deviceStatusReg1 << std::endl;
		uint16_t loopsDone = ((deviceStatusReg1 >> 20) & 0xFF);

		if (((mLoops == 0) && (loopsDone != 0)) || ((mLoops != 0) && (loopsDone == mLoops))) {
			ss << "****** PASS ******" << std::endl;
			//mLogger->print(ss.str(), 3);
		} else {
			ss << std::hex << "****** WARNING - FAIL (LOOPS NOT MET) ****** Ran for 0x" << loopsDone << std::endl;
            mLogger->print(ss.str(), 3);
			return 0;
		}

		return 0;
	}

	return -1;
}

ret_t DeviceTrafficGenerator::task()
{
    return 0;
}

void DeviceTrafficGenerator::print()
{
	uint64_t deviceStatusReg1 = *(uint64_t*)((char*)mVirtAddr + DEVICE_AFU_STATUS1_OFF);
	mLogger->print("bus: " + std::to_string(mBus) + ", loops: " + std::to_string((deviceStatusReg1 >> 20) & 0xFF), DEVICE_GENERATOR_LOGGER_ID);
}


void DeviceTrafficGenerator::setAddressList(std::shared_ptr<AddressList> addrList)
{
	mpAddrList = std::move(addrList);
}

void DeviceTrafficGenerator::setAlgorithm(std::shared_ptr<IAlgorithm> algo)
{
	mpAlgo = std::move(algo);
}


void DeviceTrafficGenerator::setOffset(uint16_t offset)
{
    mOffset = offset;
}

void DeviceTrafficGenerator::setSize(uint16_t size)
{
    mSize = size;
}

void DeviceTrafficGenerator::setLoops(uint16_t loops)
{
    mLoops = loops;
}

void DeviceTrafficGenerator::setPatternParam(uint16_t setpatternparam)
{
    mPatternParameter = setpatternparam;
}

void DeviceTrafficGenerator::setAlgoParams(uint16_t algoparams)
{
    mAlgoParams = algoparams;
}

void DeviceTrafficGenerator::setStartAddressCacheAligned(uint8_t StartAddressCacheAligned)
{
    mStartAddressCacheAligned = StartAddressCacheAligned;
}

void DeviceTrafficGenerator::setProtocol(uint16_t protocol)
{
    mProtocol = protocol;
}
