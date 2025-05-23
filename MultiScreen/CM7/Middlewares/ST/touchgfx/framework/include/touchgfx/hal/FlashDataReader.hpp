/******************************************************************************
* Copyright (c) 2018(-2025) STMicroelectronics.
* All rights reserved.
*
* This file is part of the TouchGFX 4.25.0 distribution.
*
* This software is licensed under terms that can be found in the LICENSE file in
* the root directory of this software component.
* If no LICENSE file comes with this software, it is provided AS-IS.
*
*******************************************************************************/

/**
 * @file touchgfx/hal/FlashDataReader.hpp
 *
 * Declares the touchgfx::FlashDataReader class.
 */
#ifndef TOUCHGFX_FLASHDATAREADER_HPP
#define TOUCHGFX_FLASHDATAREADER_HPP

#include <touchgfx/hal/Types.hpp>

namespace touchgfx
{
/**
 * This class is an abstract interface for a class reading data from a flash. The flash can be
 * any type, but is mostly used for flashes that are not memory mapped. Applications
 * must implement access to the flash through this interface.
 */
class FlashDataReader
{
public:
    /** Finalizes an instance of the FlashDataReader class. */
    virtual ~FlashDataReader()
    {
    }

    /**
     * Compute if an address is directly addressable by the MCU.
     *
     * Compute if an address is directly addressable by the MCU. The data is addressable it
     * should be read direct through a pointer and not through this interface.
     *
     * @param  address The address in the flash.
     *
     * @return True if the address is addressable by the MCU.
     */
    virtual bool addressIsAddressable(const void* address) = 0;

    /**
     * Copy data from flash to a buffer. This must be a synchrony method that does not
     * return until the copy is done.
     *
     * @param          src   Address of source data in the flash.
     * @param [in,out] dst   Address of destination buffer in RAM.
     * @param          bytes Number of bytes to copy.
     */
    virtual void copyData(const void* src, void* dst, uint32_t bytes) = 0;

    /**
     * Initiate a read operation from flash to a buffer. This can be an asynchrony operation
     * that is still running after this function returns. Buffers must be handled by the
     * subclass. LCD16bppSerialFlash will at most copy 4 bytes times the width of the
     * display.
     *
     * @param  src   Address of source data in the flash.
     * @param  bytes Number of bytes to copy.
     */
    virtual void startFlashLineRead(const void* src, uint32_t bytes) = 0;

    /**
     * Waits until the previous startFlashLineRead operation is complete.
     *
     * Waits until the previous startFlashLineRead operation is complete. If the
     * startFlashLineRead method is asynchrony, this method must wait until the previous
     * operation has completed.
     *
     * @return The address of a buffer containing the read data.
     */
    virtual const uint8_t* waitFlashReadComplete() = 0;
};

} // namespace touchgfx

#endif // TOUCHGFX_FLASHDATAREADER_HPP
