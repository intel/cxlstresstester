/**

  Copyright (c) 2023, Intel Corporation 
  SPDX-License-Identifier: MIT

**/

#include <iostream>
#include <thread>
#include <chrono>
#include <sched.h>
#include "CpuTrafficGenerator.h"
#define CPU_GENERATOR_LOGGER_ID       54

CpuTrafficGenerator::CpuTrafficGenerator()
{
	mLoops = 0;
}

CpuTrafficGenerator::CpuTrafficGenerator(std::shared_ptr<AddressList> addrList)
{
	mLoops = 0;
	mpAddrList = std::move(addrList);
}

ret_t CpuTrafficGenerator::configure()
{
	return 0;
}

ret_t CpuTrafficGenerator::start()
{
	mState = TrafficGeneratorStateStart;
	mLogger->print("Start.", CPU_GENERATOR_LOGGER_ID);
	return 0;
}

ret_t CpuTrafficGenerator::stop()
{
	mState = TrafficGeneratorStateStop;
	mLogger->print("Stopping.", CPU_GENERATOR_LOGGER_ID);
	return 0;
}

ret_t CpuTrafficGenerator::check()
{
	return mErrCode;
}

void CpuTrafficGenerator::print()
{
	mLogger->print("cpu id: " + std::to_string(mApicId) + ", loops: " + std::to_string(mLoops), CPU_GENERATOR_LOGGER_ID);
}

void CpuTrafficGenerator::dump()
{
	return;
}

ret_t CpuTrafficGenerator::task()
{
	cpu_set_t cpu;
	CPU_ZERO(&cpu);
	CPU_SET(mApicId, &cpu);

	// Convert the C++ style thread ID to native pthread_t and send to 
	// pthread_setaffinity_np()
	auto tid = std::this_thread::get_id();
	auto handle = *reinterpret_cast<std::thread::native_handle_type*>(&tid);

	int ret = pthread_setaffinity_np(handle, sizeof(cpu_set_t), &cpu);
	if ((ret != 0) || (mApicId != sched_getcpu())) {
		throw std::runtime_error("Fail to set thread affinity or not executing on set HW thread.");
	}

	do {
		// mLogger->print("Thread " + std::to_string(sched_getcpu()) + " waiting to start.", CPU_GENERATOR_LOGGER_ID);
		std::this_thread::sleep_for(std::chrono::seconds(1));
	} while (mState == TrafficGeneratorStateReset);

	mLogger->print("Thread " + std::to_string(sched_getcpu()) + " running.", CPU_GENERATOR_LOGGER_ID);
	do {
	} while (mState != TrafficGeneratorStateStart);


	if (mState == TrafficGeneratorStateStart) {
		mState = TrafficGeneratorStateExecuting;
	} else {
		mLogger->report_failure("State machine error found. CPUID=" + std::to_string(sched_getcpu()));
		//return -1;
		return 0;
	}

	do {
		int ret = mpAlgo->run();
		if (ret == 0) {
			mLoops++;
		} else {
			mState = TrafficGeneratorStateStopError;
			break;
		}

	} while (mState == TrafficGeneratorStateExecuting);

	// Error condition
	if (mState == TrafficGeneratorStateStopError) {
		mLogger->report_failure("Error found during core threads verify stage.");
		mErrCode = -1;
		//return -1;
		return 0;
	} else if (mState == TrafficGeneratorStateStop) {
		mLogger->print("No error detected while running.", CPU_GENERATOR_LOGGER_ID);
		mErrCode = 0;
	} else {
		mLogger->report_failure("State machine error found.  mState=" + std::to_string(mState));
	}

	return 0;
}

void CpuTrafficGenerator::setAddressList(std::shared_ptr<AddressList> addrList)
{
	mpAddrList = addrList;
	 mpAlgo->setAddressList(std::move(addrList));
}

void CpuTrafficGenerator::setAlgorithm(std::shared_ptr<IAlgorithm> algo)
{
	mpAlgo = std::move(algo);
}

void CpuTrafficGenerator::setAffinity(uint32_t apicid)
{
	mApicId = apicid;
}
