/**

  Copyright (c) 2023, Intel Corporation 
  SPDX-License-Identifier: MIT

**/

#pragma once
#include <string>
#include "CxlTypes.h"
#include "utils/Logger.h"

/**
 * @class Cxl
 */
class Cxl
{
    private:
    	uint8_t type= 0xff;
    	uint32_t seg = 0, bus = 0, dev = 0, fun = 0;
    	uint64_t physical_address = 0, size = 0;
    	std::string config;
        std::shared_ptr<Logger> logger;

    protected:
    public:
        /**
         * @brief Constructor for the Cxl class which initializes with PCIe segment, bus, device, and function values.
         *
         * @param tseg The segment number of the PCIe device.
         * @param tbus The bus number of the PCIe device.
         * @param tdev The device number of the PCIe device.
         * @param tfun The function number of the PCIe device.
         */
        Cxl(uint32_t tseg, uint32_t tbus, uint32_t tdev, uint32_t tfun);

        /**
         * @brief Constructor for the Cxl class which initializes the `physical_address` and `size` member variables.
         *
         * @param phy_addr The physical address of the PCIe device.
         * @param phy_size The size of the PCIe device.
         */
        Cxl(uint64_t& phy_addr, uint64_t& phy_size);

        /**
         * @brief Setter function for the size of the PCIe device.
         *
         * @param phy_size The size to set.
         */
        void set_physical_size(uint64_t& phy_size);

        /**
         * @brief Setter function for the physical address of the PCIe device.
         *
         * @param phy_addr The physical address to set.
         */
        void set_physical_address(uint64_t& phy_addr);

        /**
         * @brief Getter function for the physical address of the PCIe device.
         *
         * @param phy_addr The physical address to set.
         */
        void get_physical_address(uint64_t& phy_addr);
};
