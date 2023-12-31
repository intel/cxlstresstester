/**

  Copyright (c) 2023, Intel Corporation 
  SPDX-License-Identifier: MIT

**/

#pragma once
#include "ITrafficGenerator.h"
#include "algo/IAlgorithm.h"
#include "utils/Logger.h"
#include "AddressList.h"

#include <pthread.h>

/**
 * @class CpuTrafficGenerator
 */
class CpuTrafficGenerator : public ITrafficGenerator
{
	private:
		std::shared_ptr<AddressList> mpAddrList;
		std::shared_ptr<IAlgorithm> mpAlgo;
		const uint32_t mAnyApicId = 0xFFFFFFFF;
		uint32_t mApicId = mAnyApicId;
		ret_t mErrCode = 0;

	public:
		/**
		 * @brief Default constructor for CpuTrafficGenerator class.
		 */
		CpuTrafficGenerator();

		/**
		 * @brief Constructor for CpuTrafficGenerator class that sets the address list for the object.
		 * @param addrList A shared pointer to an AddressList object.
		 */
		CpuTrafficGenerator(std::shared_ptr<AddressList> addrList);

		/**
		 * @brief 
		 * 
		 * @return 0
		 */
		virtual ret_t configure();

		/**
		 * @brief Sets mState to TrafficGeneratorState::TrafficGeneratorStateStart
		 * 
		 * @return 0 
		 */
		virtual ret_t start();

		/**
		 * @brief Sets mState to TrafficGeneratorState::TrafficGeneratorStateStop
		 * 
		 * @return 0 
		 */
		virtual ret_t stop();

		/**
		 * @brief Getter function for the error code
		 */
		virtual ret_t check();

		/**
		 * @brief Run the CPU traffic generator task. The task function sets the thread affinity and waits for the state to change from
		 * TrafficGeneratorStateReset to TrafficGeneratorStateStart. Once the state is set to TrafficGeneratorStateStart, it enters the
		 * execution loop until the state changes to TrafficGeneratorStateStopError or TrafficGeneratorStateStop.
		 * 
		 * @return 0 if the task runs successfully, otherwise, an error code.
		 * 
		 * @throw std::runtime_error if the thread affinity is not set or the task is not executing on the set HW thread.
		 */
		virtual ret_t task();

		/**
		 * @brief Prints a line with the values of  mApicId and mLoops 
		 */
		virtual void print();


		virtual void dump();

		/**
		 * @brief Setter function for the address list
		 *
		 * @param addrList A shared pointer to an AddressList
		 */
		void setAddressList(std::shared_ptr<AddressList> addrList);

		/**
		 * @brief Setter function for the address list
		 *
		 * @param addrList A shared pointer to an AddressList
		 */
		void setAlgorithm(std::shared_ptr<IAlgorithm> algo);

		/**
		 * @brief Setter function for affinity
		 *
		 * @param apicid 
		 */
		void setAffinity(uint32_t apicid);
};
