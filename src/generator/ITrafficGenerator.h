/**

  Copyright (c) 2023, Intel Corporation 
  SPDX-License-Identifier: MIT

**/

#pragma once
#include <atomic>

#include "cxl/CxlTypes.h"
#include "utils/Logger.h"

//TODO: make state machine same for cpu/afu
enum TrafficGeneratorState {    TrafficGeneratorStateReset=0, 
				TrafficGeneratorStateStart=1,
				TrafficGeneratorStateExecuting=2,
				TrafficGeneratorStateStop=3,
				TrafficGeneratorStateStopError=4
			   };

class ITrafficGenerator
{
	protected:
		uint64_t mLoops = 0;
		std::atomic<TrafficGeneratorState> mState = TrafficGeneratorStateReset;
		std::atomic<bool> mActive = false;
		std::shared_ptr<Logger> mLogger;

	public:
		ITrafficGenerator();
		virtual ret_t configure() = 0;
		virtual ret_t start() = 0;
		virtual ret_t stop() = 0;
		virtual ret_t check() = 0;
		virtual ret_t task() = 0;
		virtual void print() = 0;
		virtual void dump() = 0;
};
