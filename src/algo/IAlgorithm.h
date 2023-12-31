/**

  Copyright (c) 2023, Intel Corporation 
  SPDX-License-Identifier: MIT

**/

#pragma once

#include "cxl/CxlTypes.h"
#include "utils/Logger.h"
#include "AddressList.h"

/**
 * @class IAlgorithm
 * @brief A base interface class for algorithms.
*/
class IAlgorithm
{
    private:
	protected:
		IAlgorithm();

		/**
		 * @brief A variable to store the pattern for the algorithm.
		 */
		uint64_t mPattern = 0xFFFFFFFF;

		/**
		 * @brief A shared pointer to the address list used by the algorithm.
		 */
		std::shared_ptr<AddressList> mpAddrList;
		std::shared_ptr<Logger> mLogger;
	public:
		/**
		 * @brief Sets mpAddrList to point to the passed pointer to an AddressList.
		 * 
		 * @param pAddrList A shared pointer to the address list.
		 */
		void setAddressList(std::shared_ptr<AddressList> pAddrList);
		virtual uint64_t get_operation_size(void)=0;
		virtual ret_t run()=0;
		virtual ret_t verify()=0;
};
