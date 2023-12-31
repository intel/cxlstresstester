/**

  Copyright (c) 2023, Intel Corporation 
  SPDX-License-Identifier: MIT

**/

#pragma once
#include <iostream>

#include "IAlgorithm.h"


/**
 * @class MulWrStream
 */
class MulWrStream : public IAlgorithm
{
};

/**
 * @class MulWr64
 */
class MulWr64 : public MulWrStream
{
	private:
	public:
		uint64_t iterations = 0;
		ret_t run();
		ret_t verify();

		/**
		 * @return uint64_t 0x8
		 */
		uint64_t get_operation_size(void) { return 0x8;}
};

/**
 * @class MulWr32
 */
class MulWr32 : public MulWrStream
{
	private:
	protected:
	public:
		ret_t run();
		ret_t verify();

		/**
		 * @return uint64_t 0x4
		 */
		uint64_t get_operation_size(void) { return 0x4; }
};

template <typename op>
/**
 * @class MulWr
 */
class MulWr : public MulWrStream
{
	private:
		op operation_size = 0x4;

	public:
		ret_t run();
		ret_t verify();
};

/**
 * @class MulWrStreamNew
 */
class MulWrStreamNew : public IAlgorithm
{
	private:
		uint32_t mParams;
		uint64_t mPattern;
		uint8_t mSize;
		uint8_t mOffset;

	public:
		/**
		 * @brief Constructs a new MulWrStreamNew object using passed values.
		 *
		 * @param params The size to initialize mParams to.
		 * @param pattern The size to initialize mPattern to.
		 * @param offset The size to initialize mOffset to.
		 * @param size The size to initialize mSize to.
		 */
		MulWrStreamNew(uint32_t params, uint64_t pattern, uint8_t offset, uint8_t size);

		/**
		 * @brief 
		 * 
		 * @return ret_t 
		 */
		ret_t run(void);

		/**
		 * @return uint64_t 0x0
		 */
		ret_t verify(void) {return 0x0; }

		/**
		 * @return 0x4
		 */
		uint64_t get_operation_size(void) { return 0x4; }

		/**
		 * @brief Runs write stage (further description needed)
		 */
		void WriteStage(void);

		/**
		 * @brief Runs read stage (further description needed)
		 */
		ret_t ReadStage(void);

		/**
		 * @brief Runs flush stage (further description needed)
		 */
		void FlushStage(void);

		/**
		 * @return uint8_t mOffset
		 */
		uint8_t GetOffset(void);

		/**
		 * @return uint8_t mSize
		 */
		uint8_t GetSize(void);
};
