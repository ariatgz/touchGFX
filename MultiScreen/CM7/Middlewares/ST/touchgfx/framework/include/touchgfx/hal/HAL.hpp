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
 * @file touchgfx/hal/HAL.hpp
 *
 * Declares the touchgfx::HAL class.
 */
#ifndef TOUCHGFX_HAL_HPP
#define TOUCHGFX_HAL_HPP

#include <platform/core/MCUInstrumentation.hpp>
#include <platform/driver/button/ButtonController.hpp>
#include <platform/driver/touch/TouchController.hpp>
#include <touchgfx/Bitmap.hpp>
#include <touchgfx/Drawable.hpp>
#include <touchgfx/hal/BlitOp.hpp>
#include <touchgfx/hal/DMA.hpp>
#include <touchgfx/hal/FrameBufferAllocator.hpp>
#include <touchgfx/hal/Gestures.hpp>
#include <touchgfx/hal/Types.hpp>
#include <touchgfx/lcd/LCD.hpp>

namespace touchgfx
{
class FlashDataReader;
class UIEventListener;

/**
 * Hardware Abstraction Layer.
 *
 * Contains functions that are specific to the hardware platform the code is running on.
 */
class HAL
{
public:
    /**
     * Initializes a new instance of the HAL class.
     *
     * @param [in] dmaInterface Reference to the DMA interface.
     * @param [in] display      Reference to the LCD.
     * @param [in] touchCtrl    Reference to the touch controller.
     * @param width             The width of the LCD display, in pixels.
     * @param height            The height of the LCD display, in pixels.
     */
    HAL(DMA_Interface& dmaInterface, LCD& display, TouchController& touchCtrl, uint16_t width, uint16_t height)
        : dma(dmaInterface),
          lcdRef(display),
          touchController(touchCtrl),
          mcuInstrumentation(0),
          buttonController(0),
          frameBufferAllocator(0),
          gestures(),
          nativeDisplayOrientation(ORIENTATION_LANDSCAPE),
          taskDelayFunc(0),
          frameBuffer0(0),
          frameBuffer1(0),
          frameBuffer2(0),
          refreshStrategy(REFRESH_STRATEGY_DEFAULT),
          fingerSize(1),
          lockDMAToPorch(false),
          frameBufferUpdatedThisFrame(false),
          auxiliaryLCD(0),
          partialFrameBufferRect(),
          useAuxiliaryLCD(false),
          listener(0),
          lastX(0),
          lastY(0),
          touchSampleRate(1),
          mcuLoadPct(0),
          vSyncCnt(0),
          vSyncForFrame(1),
          vSyncCompensationEnabled(false),
          clientDirty(false),
          swapRequested(false),
          lastTouched(false),
          updateMCULoad(0),
          cc_begin(0),
          requestedOrientation(ORIENTATION_LANDSCAPE),
          displayOrientationChangeRequested(false),
          useDMAAcceleration(true),
          lastRenderMethod(HARDWARE),
          isFrontPorchEntered(false),
          numberOfBlocks(4),
          maxDrawingHeight(height / numberOfBlocks), // Best guess based on testing (but not ideal in all cases)
          minDrawingHeight(maxDrawingHeight / 3),    // Best guess based on testing (but not ideal in all cases)
          maxBlockLines(20)
    {
        instance = this;
        FRAME_BUFFER_WIDTH = DISPLAY_WIDTH = width;
        FRAME_BUFFER_HEIGHT = DISPLAY_HEIGHT = height;
        DISPLAY_ROTATION = rotate0;
        nativeDisplayOrientation = ((width >= height) ? ORIENTATION_LANDSCAPE : ORIENTATION_PORTRAIT);
    }

    /** Finalizes an instance of the HAL class. */
    virtual ~HAL()
    {
    }

    /**
     * Gets the HAL instance.
     *
     * @return The HAL instance.
     */
    static HAL* getInstance()
    {
        return instance;
    }

    /**
     * Sets the desired display orientation (landscape or portrait). If desired orientation
     * is different from the native orientation of the display, a rotation is automatically
     * applied. The rotation does not incur any performance cost.
     *
     * @param  orientation The desired display orientation.
     *
     * @note A screen transition must occur before this takes effect!
     */
    virtual void setDisplayOrientation(DisplayOrientation orientation)
    {
        requestedOrientation = orientation;
        displayOrientationChangeRequested = true;
    }

    /**
     * Gets the current display orientation. Will be equal to the native orientation of the
     * display unless setDisplayOrientation has been explicitly called earlier.
     *
     * @return The current display orientation.
     */
    DisplayOrientation getDisplayOrientation() const
    {
        if (DISPLAY_ROTATION == rotate0)
        {
            return nativeDisplayOrientation;
        }
        return (nativeDisplayOrientation == ORIENTATION_LANDSCAPE ? ORIENTATION_PORTRAIT : ORIENTATION_LANDSCAPE);
    }

    /**
     * Sets framebuffer size. By default the display size and the framebuffer size are the same, but
     * in some hardware configurations, the hardware may have a width of e.g. 832 pixels even though
     * the display is only 800 pixels wide. First set the display width and height using
     * touchgfx_generic_init() and the update the framebuffer size using setFrameBufferSize().
     *
     * @param   width   The width of the framebuffer.
     * @param   height  The height of the framebuffer.
     *
     * @see touchgfx_generic_init
     */
    virtual void setFrameBufferSize(uint16_t width, uint16_t height)
    {
        assert(width >= DISPLAY_WIDTH && height >= DISPLAY_HEIGHT && "Framebuffer cannot be smaller than display");
        FRAME_BUFFER_WIDTH = width;
        FRAME_BUFFER_HEIGHT = height;
    }

    /** Notify the framework that a DMA interrupt has occurred. */
    void signalDMAInterrupt()
    {
        dma.signalDMAInterrupt();
    }

    /**
     * This function initializes the HAL, DMA, TouchController, and interrupts.
     *
     * @see configureInterrupts
     */
    virtual void initialize();

    /**
     * Main event loop. Will wait for VSYNC signal, and then process next frame. Call this
     * function from your GUI task.
     *
     * @note This function never returns!
     */
    virtual void taskEntry();

    /**
     * This function is called whenever the framework has performed a complete draw.
     *
     *  On some platforms, a local framebuffer needs to be pushed to the display through a
     *  SPI channel or similar. Implement that functionality here. This function is called
     *  whenever the framework has performed a complete draw.
     */
    virtual void flushFrameBuffer();

    /**
     * This function is called whenever the framework has performed a partial draw.
     *
     * @param  rect The area of the screen that has been drawn, expressed in absolute coordinates.
     *
     * @see flushFrameBuffer
     */
    virtual void flushFrameBuffer(const Rect& rect);

    /** Allow the DMA to start transfers. Front Porch Entry is a good place to call this. */
    virtual void allowDMATransfers();

    /**
     * Has to be called from within the LCD IRQ routine when the Front Porch Entry is
     * reached.
     */
    void frontPorchEntered()
    {
        allowDMATransfers();
        isFrontPorchEntered = true;
    }

    /**
     * @brief Get the front porch entered flag.
     *
     * @return True if the front porch has been entered, false otherwise.
     */
    bool getFrontPorchEntered() const
    {
        return isFrontPorchEntered;
    }

    /** This function blocks until the DMA queue (containing BlitOps) is empty. */
    virtual void flushDMA();

    /**
     * This function can be used to explicitly submit any GPU2D operations that
     * might be queued in the command list. Can be called if e.g. the task is
     * about to sleep, to ensure GPU2D operations are running in the background.
     *
     * Only implemented on systems with the GPU2D IP.
     */
    virtual void submitGPU2D()
    {
    }

    /**
     * Waits for the framebuffer to become available for use (i.e. not
     * used by DMA transfers). Calls the InvalidateCache virtual if
     * previous operation was hardware based.
     *
     * @return A pointer to the beginning of the currently used framebuffer.
     *
     * @note Function blocks until framebuffer is available. Client code MUST call
     *       unlockFrameBuffer() when framebuffer operation has completed.
     */
    virtual uint16_t* lockFrameBuffer();

    /**
     * A list of rendering methods.
     *
     * @see setRenderingMethod
     */
    enum RenderingMethod
    {
        SOFTWARE, ///< Transition to this method will invalidate the D-Cache, if enabled
        HARDWARE  ///< Transition to this method will flush the D-Cache, if enabled
    };

    /**
     * Locks the framebuffer and sets rendering method for correct
     * cache management.
     *
     * @param method The rendering method to be used.
     *
     * @return A pointer to the beginning of the currently used framebuffer.
     */
    uint16_t* lockFrameBufferForRenderingMethod(RenderingMethod method);

    /**
     * Unlocks the framebuffer (MUST be called exactly once for each call to
     * lockFrameBuffer()).
     */
    virtual void unlockFrameBuffer();

    /**
     * Locks and unlocks the frame buffer.
     *
     * This method is used to wait for any hardware based drawing is
     * complete when you don't need the frame buffer pointer for
     * drawing.
     */
    void lockUnlockFrameBuffer()
    {
        lockFrameBuffer();
        unlockFrameBuffer();
    }

    /**
     * Gets the framebuffer address used by the TFT controller.
     *
     * @return The address of the framebuffer currently being displayed on the TFT.
     */
    virtual uint16_t* getTFTFrameBuffer() const = 0;

    /**
     * Gets a reference to the LCD.
     *
     * @return A reference to the LCD.
     */
    static LCD& lcd()
    {
        if (instance->useAuxiliaryLCD && instance->auxiliaryLCD)
        {
            return *instance->auxiliaryLCD;
        }
        return instance->lcdRef;
    }

    /**
     * Function to set whether the DMA transfers are locked to the TFT update cycle. If
     * locked, DMA transfer will not begin until the TFT controller has finished updating
     * the display. If not locked, DMA transfers will begin as soon as possible. Default is
     * false (DMA is not locked with TFT).
     *
     * Enabling the lock will in most cases decrease rendering performance. But for single-
     * buffering, it is recommended to enable the lock, to avoid tearing on the display, i.e.
     * lock the DMA to avoid modifying the framebuffer while the display is being updated.
     *
     * @param  enableLock True to lock DMA transfers to the front porch signal. Default
     *                    disabled, which will normally yield performance improvement.
     */
    void lockDMAToFrontPorch(bool enableLock)
    {
        lockDMAToPorch = enableLock;
    }

    /**
     * This function performs a platform-specific memcpy, if supported by the hardware.
     *
     * @param [out] dest     Pointer to destination memory.
     * @param [in]  src      Pointer to source memory.
     * @param       numBytes Number of bytes to copy.
     *
     * @return true if the copy succeeded, false if copy was not performed.
     */
    virtual bool blockCopy(void* RESTRICT dest, const void* RESTRICT src, uint32_t numBytes);

    /**
     * Function for obtaining the blit capabilities of the concrete HAL implementation. As
     * default, will return whatever blitcaps are reported by the associated DMA object.
     *
     * DMA operations can be disabled by calling enableDMAAcceleration(bool).
     *
     * @return a bitmask of the supported blitcaps.
     *
     * @see enableDMAAcceleration
     */
    virtual BlitOperations getBlitCaps()
    {
        if (useDMAAcceleration)
        {
            return dma.getBlitCaps();
        }
        return static_cast<BlitOperations>(0);
    }

    /**
     * Sets a flag to allow use of DMA operations to speed up drawing operations.
     *
     * @param  enable True to enable, false to disable.
     *
     * @see getBlitCaps
     */
    void enableDMAAcceleration(const bool enable)
    {
        useDMAAcceleration = enable;
    }

    /**
     * Blits a 2D source-array to the framebuffer performing alpha-blending as specified.
     *
     * @param  pSrc                 The source-array pointer (points to first value to copy)
     * @param  pClut                The CLUT pointer (points to CLUT header data which include
     *                              the type and size of this CLUT followed by colors data)
     * @param  x                    The destination x coordinate on the framebuffer.
     * @param  y                    The destination y coordinate on the framebuffer.
     * @param  width                The width desired area of the source 2D array.
     * @param  height               The height of desired area of the source 2D array.
     * @param  srcWidth             The distance (in elements) from first value of first line, to
     *                              first value of second line (the source 2D array width)
     * @param  alpha                The alpha value to use for blending (255 = solid, no blending)
     * @param  hasTransparentPixels If true, this data copy contains transparent pixels and
     *                              require hardware support for that to be enabled.
     * @param  dstWidth             The distance (in elements) from first value of first line, to
     *                              first value of second line (the destination 2D array width)
     * @param  srcFormat            The source buffer color format (default is the framebuffer
     *                              format)
     * @param  dstFormat            The destination buffer color format (default is the
     *                              framebuffer format)
     * @param  replaceBgAlpha       Replace the background buffer per pixel alpha value
     *                              with 255 = solid.
     *
     * @note Alpha=255 is assumed "solid" and shall be used if HAL does not support
     *       BLIT_OP_COPY_WITH_ALPHA.
     */
    virtual void blitCopy(const uint16_t* pSrc, const uint8_t* pClut, uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t srcWidth, uint8_t alpha, bool hasTransparentPixels, uint16_t dstWidth, Bitmap::BitmapFormat srcFormat, Bitmap::BitmapFormat dstFormat, bool replaceBgAlpha);

    /**
     * Blits a 2D source-array to the framebuffer performing alpha-blending as specified.
     *
     * @param  pSrc                 The source-array pointer (points to first value to copy)
     * @param  x                    The destination x coordinate on the framebuffer.
     * @param  y                    The destination y coordinate on the framebuffer.
     * @param  width                The width desired area of the source 2D array.
     * @param  height               The height of desired area of the source 2D array.
     * @param  srcWidth             The distance (in elements) from first value of first line, to
     *                              first value of second line (the source 2D array width)
     * @param  alpha                The alpha value to use for blending (255 = solid, no blending)
     * @param  hasTransparentPixels If true, this data copy contains transparent pixels and
     *                              require hardware support for that to be enabled.
     * @param  dstWidth             The distance (in elements) from first value of first line, to
     *                              first value of second line (the destination 2D array width)
     * @param  srcFormat            The source buffer color format (default is the framebuffer
     *                              format)
     * @param  dstFormat            The destination buffer color format (default is the
     *                              framebuffer format)
     * @param  replaceBgAlpha       Replace the background buffer per pixel alpha value
     *                              with 255 = solid.
     *
     * @note Alpha=255 is assumed "solid" and shall be used if HAL does not support
     *       BLIT_OP_COPY_WITH_ALPHA.
     */
    virtual void blitCopy(const uint16_t* pSrc, uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t srcWidth, uint8_t alpha, bool hasTransparentPixels, uint16_t dstWidth, Bitmap::BitmapFormat srcFormat, Bitmap::BitmapFormat dstFormat, bool replaceBgAlpha);

    /**
     * Blits a 2D source-array to the framebuffer performing alpha-blending as specified using
     * the default lcd format.
     *
     * @param  pSrc                 The source-array pointer (points to first value to copy)
     * @param  x                    The destination x coordinate on the framebuffer.
     * @param  y                    The destination y coordinate on the framebuffer.
     * @param  width                The width desired area of the source 2D array.
     * @param  height               The height of desired area of the source 2D array.
     * @param  srcWidth             The distance (in elements) from first value of first line, to
     *                              first value of second line (the source 2D array width)
     * @param  alpha                The alpha value to use for blending (255 = solid, no blending)
     * @param  hasTransparentPixels If true, this data copy contains transparent pixels and
     *                              require hardware support for that to be enabled.
     * @param  replaceBgAlpha       Replace the background buffer per pixel alpha value
     *                              with 255 = solid.
     *
     * @note Alpha=255 is assumed "solid" and shall be used if HAL does not support
     *       BLIT_OP_COPY_WITH_ALPHA.
     */
    virtual void blitCopy(const uint16_t* pSrc, uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t srcWidth, uint8_t alpha, bool hasTransparentPixels, bool replaceBgAlpha);

    /**
     * Blits a 2D source-array to the framebuffer using 16-bit copy
     * without conversion. This operation can be used to perform
     * hardware accelerated copies to the framebuffer even when the
     * image (and framebuffer) format is not 16-bit.
     *
     * All parameters (e.g. x) must correspond to their 16-bit
     * values. I.e. the 10th bytes corresponds to x=5.
     *
     * @param pSrc     Pointer to the source data (points to first value to copy)
     * @param x        The destination x coordinate in the framebuffer with 16-bit pixels.
     * @param y        The destination y coordinate in the framebuffer with 16-bit pixels.
     * @param width    The width of the area to copy in 16-bit pixels.
     * @param height   The height of the area to copy
     * @param srcWidth The width of the source bitmap (stride) in 16-bit pixels.
     * @param dstWidth The width of the framebuffer in 16-bit pixels.
     */
    virtual void blitCopyWord(const uint16_t* pSrc, uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t srcWidth, uint16_t dstWidth);

    /**
     * Fills a part of the framebuffer using 16-bit fill without
     * conversion. This operation can be used to perform hardware
     * accelerated fills in the framebuffer even when the framebuffer
     * format is not 16-bit.
     *
     * All parameters (e.g. x) must correspond to their 16-bit
     * values. I.e. the 10th bytes corresponds to x=5.
     *
     * @param colorValue The 16-bit value to fill in the framebuffer.
     * @param x          The destination x coordinate in the framebuffer with 16-bit pixels.
     * @param y          The destination y coordinate in the framebuffer with 16-bit pixels.
     * @param width      The width of the area to copy in 16-bit pixels.
     * @param height     The height of the area to copy
     * @param dstWidth   The width of the framebuffer in 16-bit pixels.
     */
    virtual void blitFillWord(uint16_t colorValue, uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t dstWidth);

    /**
     * Blits a 2D source-array to the framebuffer performing per-pixel alpha blending.
     *
     * @param  pSrc                 The source-array pointer (points to first value to copy)
     * @param  x                    The destination x coordinate on the framebuffer.
     * @param  y                    The destination y coordinate on the framebuffer.
     * @param  width                The width desired area of the source 2D array.
     * @param  height               The height of desired area of the source 2D array.
     * @param  srcWidth             The distance (in elements) from first value of first line, to first
     *                              value of second line (the source 2D array width)
     * @param  alpha                The alpha value to use for blending. This is applied on every pixel,
     *                              in addition to the per-pixel alpha value (255 = solid, no blending)
     * @param  replaceBgAlpha       Replace the background buffer per pixel alpha value with 255 = solid.
     *
     */
    virtual void blitCopyARGB8888(const uint16_t* pSrc, uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t srcWidth, uint8_t alpha, bool replaceBgAlpha);

    /**
     * Blits a 4bpp or 8bpp glyph - maybe use the same method and supply additional color
     * mode arg.
     *
     * @param  pSrc                 The source-array pointer (points to first value to copy)
     * @param  x                    The destination x coordinate on the framebuffer.
     * @param  y                    The destination y coordinate on the framebuffer.
     * @param  width                The width desired area of the source 2D array.
     * @param  height               The height of desired area of the source 2D array.
     * @param  srcWidth             The distance (in elements) from first value of first line, to first
     *                              value of second line (the source 2D array width)
     * @param  color                Color of the text.
     * @param  alpha                The alpha value to use for blending (255 = solid, no blending)
     * @param  operation            The operation type to use for blit copy.
     * @param  replaceBgAlpha       Replace the background buffer per pixel alpha value
     *                              with 255 = solid.
     */
    virtual void blitCopyGlyph(const uint8_t* pSrc, uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t srcWidth, colortype color, uint8_t alpha, BlitOperations operation, bool replaceBgAlpha);

    /**
     * Blits a color value to the framebuffer performing alpha-blending as specified.
     *
     * @param  color                The desired fill-color.
     * @param  x                    The destination x coordinate on the framebuffer.
     * @param  y                    The destination y coordinate on the framebuffer.
     * @param  width                The width desired area of the source 2D array.
     * @param  height               The height of desired area of the source 2D array.
     * @param  alpha                The alpha value to use for blending (255 = solid, no blending)
     * @param  dstWidth             The distance (in elements) from first value of first line, to first value
     *                              of second line (the destination 2D array width)
     * @param  dstFormat            The destination buffer color format (default is the framebuffer format)
     * @param  replaceBgAlpha       Replace the background buffer per pixel alpha value
     *                              with 255 = solid.
     *
     * @note Alpha=255 is assumed "solid" and shall be used if HAL does not support
     *       BLIT_OP_FILL_WITH_ALPHA.
     */
    virtual void blitFill(colortype color, uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t alpha, uint16_t dstWidth, Bitmap::BitmapFormat dstFormat, bool replaceBgAlpha);

    /**
     * Copies a region of the currently displayed framebuffer to memory. Used for e.g.
     * BlockTransition and for displaying pre-rendered drawables
     * e.g. in animations where redrawing the drawable is not necessary.
     *
     * @param  region               The displayed framebuffer region to copy.
     *
     * @return A pointer to the memory address containing the copy of the framebuffer.
     *
     * @note Requires double framebuffer to be enabled.
     */
    virtual uint16_t* copyFromTFTToClientBuffer(Rect region);

    /**
     * Blits a color value to the framebuffer performing alpha-blending as specified.
     *
     * @param  color                The desired fill-color.
     * @param  x                    The destination x coordinate on the framebuffer.
     * @param  y                    The destination y coordinate on the framebuffer.
     * @param  width                The width desired area of the source 2D array.
     * @param  height               The height of desired area of the source 2D array.
     * @param  alpha                The alpha value to use for blending (255 = solid, no blending)
     * @param  replaceBgAlpha       Replace the background buffer per pixel alpha value
     *                              with 255 = solid.
     *
     * @note Alpha=255 is assumed "solid" and shall be used if HAL does not support
     *       BLIT_OP_FILL_WITH_ALPHA.
     */
    virtual void blitFill(colortype color, uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t alpha, bool replaceBgAlpha);

    /**
     * Registers an event handler implementation with the underlying event system. The
     * actual HAL implementation decides whether or not multiple UIEventListener instances
     * are allowed (including execution order).
     *
     * @param [in] listener The listener to register.
     */
    virtual void registerEventListener(UIEventListener& listener);

    /**
     * Copies a region of the currently displayed framebuffer to memory. Used for e.g.
     * SlideTransition and for displaying pre-rendered drawables
     * e.g. in animations where redrawing the drawable is not necessary.
     *
     * @param  meAbs The framebuffer region to copy.
     *
     * @return A pointer to the memory address containing the copy of the framebuffer.
     *
     * @note Requires animation storage to be present.
     */
    virtual uint16_t* copyFBRegionToMemory(Rect meAbs);

    /**
     * Copies a region of the currently displayed framebuffer to a buffer. Used for e.g.
     * SlideTransition and for displaying pre-rendered drawables e.g. in animations where
     * redrawing the drawable is not necessary. The buffer can e.g. be a dynamic bitmap.
     *
     * @param          meAbs  The framebuffer region to copy.
     * @param [in,out] dst    Address of the buffer to store the copy in.
     * @param          stride The width of the target buffer (row length).
     *
     * @return A pointer to the memory address containing the copy of the framebuffer.
     *
     * @note Requires animation storage to be present.
     */
    virtual uint16_t* copyFBRegionToMemory(Rect meAbs, uint16_t* dst, uint32_t stride);

    /**
     * Gets display width.
     *
     * @return The display width.
     */
    uint16_t getDisplayWidth() const
    {
        return DISPLAY_WIDTH;
    }

    /**
     * Gets display height.
     *
     * @return The display height.
     */
    uint16_t getDisplayHeight() const
    {
        return DISPLAY_HEIGHT;
    }

    /** Swaps the two framebuffers. */
    void swapFrameBuffers();

    /**
     * Returns the number of VSync interrupts between the current drawing operation and the
     * last drawing operation, i.e. the number of lost frames.
     *
     * @return Number of VSync since previous draw.
     */
    uint32_t getLCDRefreshCount()
    {
        return vSyncForFrame;
    }

    /**
     * Enables or disables compensation for lost frames. See knowledge base article.
     *
     * @param  enabled true to enable, false to disable.
     */
    void setFrameRateCompensation(bool enabled)
    {
        vSyncCompensationEnabled = enabled;
    }

    /**
     * Called by the VSync interrupt.
     *
     * Called by the VSync interrupt for counting of LCD refreshes.
     */
    void vSync()
    {
        vSyncCnt++;
        isFrontPorchEntered = false;
    }

    /**
     * Get current VSync count.
     *
     * @return The current VSync count.
     */
    uint8_t getVSyncCount() const
    {
        return vSyncCnt;
    }

    /**
     * Has to be called from within the LCD IRQ rutine when the Back Porch Exit is reached.
     */
    virtual void backPorchExited()
    {
        swapFrameBuffers();
        tick();
    }

    /**
     * Configures the interrupts relevant for TouchGFX. This primarily entails setting the
     * interrupt priorities for the DMA and LCD interrupts.
     */
    virtual void configureInterrupts() = 0;

    /** Enables the DMA and LCD interrupts. */
    virtual void enableInterrupts() = 0;

    /** Disables the DMA and LCD interrupts. */
    virtual void disableInterrupts() = 0;

    /**
     * Configure the LCD controller to fire interrupts at VSYNC. Called automatically once
     * TouchGFX initialization has completed.
     */
    virtual void enableLCDControllerInterrupt() = 0;

    /**
     * Sample external key event.
     *
     * @param [out] key Output parameter that will be set to the key value if a keypress was
     *                  detected.
     *
     * @return True if a keypress was detected and the "key" parameter is set to a value.
     */
    virtual bool sampleKey(uint8_t& key)
    {
        (void)key; // Unused variable
        return false;
    }

    /**
     * Configure the threshold for reporting drag events. A touch input movement must exceed
     * this value in either axis in order to report a drag. Default value is 0.
     *
     * @param  value New threshold value.
     *
     * @note Use if touch controller is not completely accurate to avoid "false" drags.
     */
    void setDragThreshold(uint8_t value)
    {
        gestures.setDragThreshold(value);
    }

    /**
     * Get the Gesture class attached to the HAL instance.
     *
     * @return A pointer to the Gestures object.
     */
    Gestures* getGestures()
    {
        return &gestures;
    }

    static uint16_t DISPLAY_WIDTH;           ///< The width of the LCD display in pixels.
    static uint16_t DISPLAY_HEIGHT;          ///< The height of the LCD display in pixels.
    static DisplayRotation DISPLAY_ROTATION; ///< The rotation from display to framebuffer.
    static uint16_t FRAME_BUFFER_WIDTH;      ///< The width of the framebuffer in pixels.
    static uint16_t FRAME_BUFFER_HEIGHT;     ///< The height of the framebuffer in pixels.
    static bool USE_DOUBLE_BUFFERING;        ///< Is double buffering enabled?
    static bool USE_ANIMATION_STORAGE;       ///< Is animation storage enabled?

    /**
     * Sets framebuffer start addresses.
     *
     * Sets individual framebuffer start addresses.
     *
     * @param [in] frameBuffer      Buffer for framebuffer data, must be non-null.
     * @param [in] doubleBuffer     If non-null, buffer for double buffer data. If null double
     *                              buffering is disabled.
     * @param [in] animationStorage If non-null, the animation storage. If null animation storage
     *                              is disabled.
     *
     * @see setAnimationStorage
     */
    virtual void setFrameBufferStartAddresses(void* frameBuffer, void* doubleBuffer, void* animationStorage)
    {
        assert(frameBuffer != 0 && "A framebuffer address must be set");
        frameBuffer0 = reinterpret_cast<uint16_t*>(frameBuffer);
        frameBuffer1 = reinterpret_cast<uint16_t*>(doubleBuffer);
        USE_DOUBLE_BUFFERING = doubleBuffer != 0;
        setAnimationStorage(animationStorage);
    }

    /**
     * Sets animation storage address.
     *
     * @param [in] animationStorage If non-null, the animation storage. If null animation storage
     *                              is disabled.
     *
     * @see setFrameBufferStartAddresses
     */
    virtual void setAnimationStorage(void* animationStorage)
    {
        frameBuffer2 = reinterpret_cast<uint16_t*>(animationStorage);
        USE_ANIMATION_STORAGE = animationStorage != 0;
    }

    /**
     * Configures a partial framebuffer as current framebuffer. This method uses the
     * assigned FrameBufferAllocator to allocate block of compatible dimensions. The height
     * of the allocated block is returned.
     *
     * @param  x      The absolute x coordinate of the block on the screen.
     * @param  y      The absolute y coordinate of the block on the screen.
     * @param  width  The width of the block.
     * @param  height The height of the block requested.
     *
     * @return The height of the block allocated.
     */
    virtual uint16_t configurePartialFrameBuffer(const uint16_t x, const uint16_t y, const uint16_t width, const uint16_t height);

    /**
     * Adjusts the framebuffer pointer to match the current logical block. This function
     * is only used on LTDC systems and when the partial framebuffer strategy is enabled.
     *
     * @param  currentBlock The current block on the screen.
     *
     */
    virtual void configurePartialFrameBufferLTDC(int16_t currentBlock);

    /**
     * This function causes the task to wait on a semaphore until the LTDC has
     * progressed the specified number of lines.
     *
     * Only applicable on LTDC systems and when the partial framebuffer strategy is
     * enabled.
     *
     * @param numberOfLines Number of lines to wait.
     */
    virtual void waitForLTDCLines(uint16_t numberOfLines)
    {
    }

    /**
     * Sets the number of ticks between each touch screen sample.
     *
     * @param  sampleRateInTicks Sample rate. Default is 1 (every tick).
     */
    void setTouchSampleRate(int8_t sampleRateInTicks)
    {
        if (sampleRateInTicks > 0)
        {
            touchSampleRate = sampleRateInTicks;
        }
    }

    /**
     * Gets the number of ticks between each touch screen sample.
     *
     * @return Number of ticks between each touch screen sample.
     */
    int8_t getTouchSampleRate() const
    {
        return touchSampleRate;
    }

    /**
     * Register if MCU is active by measuring cpu cycles. If user wishes to track MCU load,
     * this method should be called whenever the OS Idle task is scheduled in or out. This
     * method makes calls to a concrete implementation of GPIO functionality and a concrete
     * implementation of cpu cycles.
     *
     * @param  active If true, MCU is registered as being active, inactive otherwise.
     */
    void setMCUActive(bool active);

    /**
     * Gets the current cycle counter.
     *
     * @return the cycle counter.
     */
    uint32_t getCPUCycles();

    /**
     * Stores a pointer to an instance of an MCU specific instrumentation class.
     *
     * @param [in] mcuInstr pointer to MCU instrumentation.
     */
    void setMCUInstrumentation(MCUInstrumentation* mcuInstr)
    {
        mcuInstrumentation = mcuInstr;
    }

    /**
     * This method sets a flag that determines if generic HAL should calculate MCU load
     * based on concrete MCU instrumentation.
     *
     * @param  enabled If true, set flag to update MCU load.
     */
    void enableMCULoadCalculation(bool enabled)
    {
        updateMCULoad = enabled;
    }

    /**
     * Gets the current MCU load.
     *
     * @return mcuLoadPct the MCU Load in %.
     */
    uint8_t getMCULoadPct() const
    {
        return mcuLoadPct;
    }

    /**
     * Stores a pointer to an instance of a specific implementation of a ButtonController.
     *
     * @param [in] btnCtrl pointer to button controller.
     */
    void setButtonController(ButtonController* btnCtrl)
    {
        buttonController = btnCtrl;
    }

    /**
     * Gets the associated ButtonController.
     *
     * @return A pointer to the ButtonController, or zero if no ButtonController has been
     *         set.
     */
    ButtonController* getButtonController() const
    {
        return buttonController;
    }

    /**
     * Sets a framebuffer allocator. The framebuffer allocator is only used in partial
     * framebuffer mode.
     *
     * @param [in] allocator pointer to a framebuffer allocator object.
     */
    void setFrameBufferAllocator(FrameBufferAllocator* allocator)
    {
        frameBufferAllocator = allocator;
    }

    /**
     * Gets the framebuffer allocator.
     *
     * @return The framebuffer allocator.
     */
    FrameBufferAllocator* getFrameBufferAllocator()
    {
        return frameBufferAllocator;
    }

    /**
     * Gets the flash data reader. This method must be implemented in
     * subclasses that uses a FlashDataReader object.
     *
     * @return the FlashDataReader.
     */
    virtual FlashDataReader* getFlashDataReader() const
    {
        return 0;
    }

    /**
     * Sets the finger size in pixels. Setting the finger size to a size of more than 1
     * pixel will emulate a finger of width and height of 2*(fingersize-1)+1. This can be
     * especially useful when trying to interact with small elements on a high ppi display.
     * The finger size will influence which element is chosen as the point of interaction,
     * when clicking, dragging, ... the display. A number of samples will be drawn from
     * within the finger area and a best matching drawable will be chosen. The best matching
     * algorithm will consider the size of the drawable and the distance from the touch
     * point.
     *
     * @param [in] size the size of the finger.
     */
    void setFingerSize(uint8_t size)
    {
        fingerSize = size;
    }

    /**
     * Gets the finger size in pixels.
     *
     * @return The size of the finger in pixels, 1 is the default value.
     */
    uint8_t getFingerSize() const
    {
        return fingerSize;
    }

    /**
     * Gets the optional framebuffer used for animation storage.
     *
     * @return The address or 0 if unused.
     */
    uint16_t* getAnimationStorage() const
    {
        return frameBuffer2;
    }

    /**
     * A list of available frame refresh strategies.
     *
     * @see setFrameRefreshStrategy
     */
    enum FrameRefreshStrategy
    {
        REFRESH_STRATEGY_DEFAULT,                      ///< If not explicitly set, this strategy is used.
        REFRESH_STRATEGY_OPTIM_SINGLE_BUFFER_TFT_CTRL, ///< Strategy optimized for single framebuffer on systems with TFT controller.
        REFRESH_STRATEGY_PARTIAL_BUFFER_TFT_CTRL,      ///< Strategy using less than a full framebuffer on systems with TFT controller.
        REFRESH_STRATEGY_PARTIAL_FRAMEBUFFER           ///< Strategy using less than a full framebuffer.
    };

    /**
     * Set a specific strategy for handling timing and mechanism of framebuffer drawing.
     *
     * By setting a different frame refresh strategy, the internals of how TouchGFX
     * interacts with the framebuffer can be modified.
     *
     * Currently there are two strategies available. This will increase over time.
     * - REFRESH_STRATEGY_OPTIM_SINGLE_BUFFER_TFT_CTRL: this strategy is available
     * on targets that use single buffering on a TFT controller based system. It requires an
     * implementation of the getTFTCurrentLine() function as well as a task delay function
     * being registered. The implementation of this strategy is that TouchGFX will carefully
     * track the progress of the TFT controller, and draw parts of the framebuffer whenever
     * possible. The effect is that the risk of tearing is much reduced compared to the
     * default single buffer strategy of only drawing in porch areas. It does have a
     * drawback of slightly increased MCU load. But in many cases employing this strategy
     * will make it possible to avoid external RAM, by using just a single framebuffer in
     * internal RAM and still avoid tearing.
     * - REFRESH_STRATEGY_DEFAULT: This is a general strategy that works for all target
     * configurations.
     *
     * Recommendation: Try using REFRESH_STRATEGY_OPTIM_SINGLE_BUFFER_TFT_CTRL if you're on
     * a TFT controller based system (ie. non-8080) and you have a desire to avoid external
     * RAM. Otherwise stick to REFRESH_STRATEGY_DEFAULT.
     *
     * @param  s The desired strategy to use.
     *
     * @return true if the desired strategy will be used, false otherwise.
     */
    bool setFrameRefreshStrategy(FrameRefreshStrategy s)
    {
        if (s == REFRESH_STRATEGY_DEFAULT || s == REFRESH_STRATEGY_PARTIAL_FRAMEBUFFER)
        {
            refreshStrategy = s;
            return true;
        }
        if (s == REFRESH_STRATEGY_OPTIM_SINGLE_BUFFER_TFT_CTRL)
        {
            // Perform sanity checks. This strategy requires
            //   - task delay function
            //   - a TFT controller (+ an impl of getTFTCurrentLine())
            //   - single buffering
            if (taskDelayFunc != 0 && getTFTCurrentLine() != 0xFFFF && !USE_DOUBLE_BUFFERING)
            {
                refreshStrategy = s;
                return true;
            }
            return false;
        }
        if (s == REFRESH_STRATEGY_PARTIAL_BUFFER_TFT_CTRL)
        {
            // Perform sanity checks. This strategy requires
            //   - a TFT controller (+ an impl of getTFTCurrentLine())
            //   - single buffering
            if (getTFTCurrentLine() != 0xFFFF && !USE_DOUBLE_BUFFERING)
            {
                refreshStrategy = s;
                return true;
            }
            return false;
        }
        // Unknown strategy
        return false;
    }

    /**
     * Used internally by TouchGFX core to manage the timing and process of drawing into the
     * framebuffer.
     *
     * @return Current frame refresh strategy.
     *
     * @see setFrameRefreshStrategy
     */
    FrameRefreshStrategy getFrameRefreshStrategy() const
    {
        return refreshStrategy;
    }

    /**
     * Registers a function capable of delaying GUI task execution
     *
     * In order to make use of the HAL::taskDelay function, a delay function must be
     * registered by calling this function. Usually the delay function would be
     * OSWrappers::taskDelay.
     *
     * @param [in] delayF A pointer to a function returning void with an uint16_t parameter
     *                    specifying number of milliseconds to delay.
     *
     * @note The task delay capability is only used when the frame refresh strategy
     *       REFRESH_STRATEGY_OPTIM_SINGLE_BUFFER_TFT_CTRL is selected. Otherwise it is
     *       not necessary to register a delay function.
     */
    void registerTaskDelayFunction(void (*delayF)(uint16_t))
    {
        taskDelayFunc = delayF;
    }

    /**
     * Delay GUI task execution by number of milliseconds
     *
     * This function requires the presence of a task delay function. If a task delay
     * function has not been registered, it returns immediately. Otherwise it returns when
     * number of milliseconds has passed.
     *
     * @param  ms Number of milliseconds to wait.
     *
     * @see registerTaskDelayFunction
     */
    virtual void taskDelay(uint16_t ms)
    {
        if (taskDelayFunc)
        {
            taskDelayFunc(ms);
        }
    }

    /**
     * Get the current line (Y) of the TFT controller
     *
     * This function is used to obtain the progress of the TFT controller. More specifically,
     * the line (or Y-value) currently being transferred.
     *
     * Note: The value must be adjusted to account for vertical back porch before returning,
     * such that the value is always within the range of [0;
     * actual display height in pixels[
     *
     * It is used for the REFRESH_STRATEGY_OPTIM_SINGLE_BUFFER_TFT_CTRL frame refresh
     * strategy in order to synchronize framebuffer drawing with TFT controller progress. If
     * this strategy is used, the concrete HAL subclass must provide an override of this
     * function that returns correct line value. If this strategy is not used, then the
     * getTFTCurrentLine function is never called and can be disregarded.
     *
     * @return In this default implementation, 0xFFFF is returned to signify "not
     *         implemented".
     */
    virtual uint16_t getTFTCurrentLine()
    {
        return 0xFFFFu;
    }

    /**
     * Function for obtaining the DMA type of the concrete DMA implementation. As default,
     * will return DMA_TYPE_GENERIC type value.
     *
     * @return a DMAType value of the concrete DMA implementation.
     */
    virtual DMAType getDMAType()
    {
        return dma.getDMAType();
    }

    /**
     * Render a Drawable and its widgets into a dynamic bitmap.
     *
     * @param [in,out] drawable A reference on the Drawable object to render.
     * @param          bitmapId Dynamic bitmap to be used as a rendertarget.
     */
    virtual void drawDrawableInDynamicBitmap(Drawable& drawable, BitmapId bitmapId);

    /**
     * Render a Drawable and its widgets into a dynamic bitmap. Only the specified Rect
     * region is updated.
     *
     * @param [in,out] drawable A reference on the Drawable object to render.
     * @param          bitmapId Dynamic bitmap to be used as a rendertarget.
     * @param          rect     Region to update.
     */
    virtual void drawDrawableInDynamicBitmap(Drawable& drawable, BitmapId bitmapId, const Rect& rect);

    /**
     * Set an auxiliary LCD class to be used for offscreen rendering.
     *
     * @param [in,out] auxLCD A pointer on the axiliary LCD class to use for offscreen rendering.
     */
    void setAuxiliaryLCD(LCD* auxLCD)
    {
        auxiliaryLCD = auxLCD;
    }

    /**
     * Get the auxiliary LCD class attached to the HAL instance if any.
     *
     * @return A pointer on the axiliary LCD class attached to the HAL instance.
     */
    LCD* getAuxiliaryLCD()
    {
        return auxiliaryLCD;
    }

    /**
     * Set current rendering method for cache maintenance.
     *
     * This function is used to keep track of previous rendering method and will determine if cache should be flush or invalidated depending on transition state.
     *
     * @param method The rendering method used.
     */
    void setRenderingMethod(RenderingMethod method);

    /**
     * Set number of blocks to be used for partial framebuffer strategy on LTDC systems.
     * Both minDrawingHeight and maxDrawingHeight will be updated according to the new
     * number of blocks.
     * Note: The number must be divisible by the framebuffer height.
     *
     * @see setMinDrawingHeight, setMaxDrawingHeight
     *
     * @param blocks The number of blocks to be used.
     */
    void setNumberOfBlocks(uint8_t blocks)
    {
        numberOfBlocks = blocks;
        maxDrawingHeight = FRAME_BUFFER_HEIGHT / numberOfBlocks;
        minDrawingHeight = maxDrawingHeight / 3;
    }

    /**
     * Get number of blocks used for partial framebuffer strategy on LTDC systems.
     *
     * @return The number of blocks used.
     */
    uint8_t getNumberOfBlocks() const
    {
        return numberOfBlocks;
    }

    /**
     * Set maximum drawing height used for partial framebuffer strategy on LTDC systems.
     *
     * @param maxHeight The maximum drawing height to be used.
     */
    void setMaxDrawingHeight(uint16_t maxHeight)
    {
        maxDrawingHeight = maxHeight;
    }

    /**
     * Get maximum drawing height used for partial framebuffer strategy on LTDC systems.
     *
     * @return The maximum drawing height used.
     */
    uint16_t getMaxDrawingHeight() const
    {
        return maxDrawingHeight;
    }

    /**
     * Set minimum drawing height used for partial framebuffer strategy on LTDC systems.
     *
     * @param minHeight The minimum drawing height to be used.
     */
    void setMinDrawingHeight(uint16_t minHeight)
    {
        minDrawingHeight = minHeight;
    }

    /**
     * Get minimum drawing height used for partial framebuffer strategy on LTDC systems.
     *
     * @return The minimum drawing height used.
     */
    uint16_t getMinDrawingHeight() const
    {
        return minDrawingHeight;
    }

    /**
     * Set maximum number of block lines used for partial framebuffer strategy on GRAM systems.
     *
     * @param blockLines The maximum number of block lines to be used.
     */
    void setMaxBlockLines(int16_t blockLines)
    {
        maxBlockLines = blockLines;
    }

    /**
     * Get maximum number of block lines used for partial framebuffer strategy on GRAM systems.
     *
     * @return The maximum number of block lines.
     */
    int16_t getMaxBlockLines() const
    {
        return maxBlockLines;
    }

    /**
     * Called by the framework if it detects a draw operation took too long to complete
     * when using the partial LTDC frame refresh strategy.
     * This occurrence will lead to tearing on the display.
     *
     * This empty function can be overridden to e.g. output this error condition
     * on a pin.
     */
    virtual void partialLTDCDeadlineMissed()
    {
    }

protected:
    /** This function is called at each timer tick, depending on platform implementation. */
    virtual void tick();

    /**
     * Called when beginning to rendering a frame.
     *
     * @return true if rendering can begin, false otherwise.
     */
    virtual bool beginFrame();

    /** Called when a rendering pass is completed. */
    virtual void endFrame();

    /**
     * Sets the framebuffer address used by the TFT controller.
     *
     * @param [in] address New framebuffer address.
     */
    virtual void setTFTFrameBuffer(uint16_t* address) = 0;

    /**
     * Gets client framebuffer.
     *
     * @return The address of the framebuffer currently used by the framework to draw in.
     */
    uint16_t* getClientFrameBuffer()
    {
        if (USE_DOUBLE_BUFFERING && getTFTFrameBuffer() == frameBuffer0)
        {
            return frameBuffer1;
        }
        return frameBuffer0;
    }

    /**
     * Called by the touch driver to indicate a touch.
     *
     * @param  x The x coordinate of the touch.
     * @param  y The y coordinate of the touch.
     */
    virtual void touch(int32_t x, int32_t y);

    /** Called by the touch driver to indicate that no touch is currently detected. */
    virtual void noTouch();

    /** Perform the actual display orientation change. */
    virtual void performDisplayOrientationChange()
    {
        if (requestedOrientation != nativeDisplayOrientation)
        {
            if (DISPLAY_ROTATION == rotate0)
            {
                const uint16_t tmp = DISPLAY_HEIGHT;
                DISPLAY_HEIGHT = DISPLAY_WIDTH;
                DISPLAY_WIDTH = tmp;
                DISPLAY_ROTATION = rotate90;
            }
        }
        else if (DISPLAY_ROTATION != rotate0)
        {
            const uint16_t tmp = DISPLAY_HEIGHT;
            DISPLAY_HEIGHT = DISPLAY_WIDTH;
            DISPLAY_WIDTH = tmp;
            DISPLAY_ROTATION = rotate0;
        }
    }

    /**
     * Invalidate D-Cache.
     *
     * Called by setRenderingMethod when changing rendering method
     * from Hardware to Software indicating the cache should be invalidated.
     */
    virtual void InvalidateCache()
    {
    }

    /**
     * Flush D-Cache.
     *
     * Called by setRenderingMethod when changing rendering method
     * from Software to Hardware indicating the cache should be invalidated.
     */
    virtual void FlushCache()
    {
    }

    /**
     * Invalidate texture cache(s).
     *
     * Called on every tick before drawing into the framebuffer to enable
     * invalidation of any caches that may be incoherent with main memory.
     */
    virtual void InvalidateTextureCache()
    {
    }

    DMA_Interface& dma;                          ///< A reference to the DMA interface.
    LCD& lcdRef;                                 ///< A reference to the LCD.
    TouchController& touchController;            ///< A reference to the touch controller.
    MCUInstrumentation* mcuInstrumentation;      ///< A reference to an optional MCU instrumentation.
    ButtonController* buttonController;          ///< A reference to an optional ButtonController.
    FrameBufferAllocator* frameBufferAllocator;  ///< A reference to an optional FrameBufferAllocator.
    static bool isDrawing;                       ///< True if currently in the process of rendering a screen.
    Gestures gestures;                           ///< Class for low-level interpretation of touch events.
    DisplayOrientation nativeDisplayOrientation; ///< Contains the native display orientation. If desired orientation is different, apply rotation.
    void (*taskDelayFunc)(uint16_t);             ///< Pointer to a function that can delay GUI task for a number of milliseconds.
    uint16_t* frameBuffer0;                      ///< Pointer to the first framebuffer.
    uint16_t* frameBuffer1;                      ///< Pointer to the second framebuffer.
    uint16_t* frameBuffer2;                      ///< Pointer to the optional third framebuffer used for animation storage.
    FrameRefreshStrategy refreshStrategy;        ///< The selected display refresh strategy.
    uint8_t fingerSize;                          ///< The radius of the finger in pixels
    bool lockDMAToPorch;                         ///< Whether or not to lock DMA transfers with TFT porch signal.
    bool frameBufferUpdatedThisFrame;            ///< True if something was drawn in the current frame.
    LCD* auxiliaryLCD;                           ///< Auxiliary LCD class used to render Drawables into dynamic bitmaps.
    Rect partialFrameBufferRect;                 ///< The region of the screen covered by the partial framebuffer.
    bool useAuxiliaryLCD;                        ///< True if using another LCD than the hardware framebuffer

private:
    UIEventListener* listener;
    static HAL* instance;
    int32_t lastX;
    int32_t lastY;
    int8_t touchSampleRate;
    uint8_t mcuLoadPct;
    uint8_t vSyncCnt;
    uint8_t vSyncForFrame;
    bool vSyncCompensationEnabled;
    bool clientDirty;
    bool swapRequested;
    bool lastTouched;
    bool updateMCULoad;
    uint32_t cc_begin;
    DisplayOrientation requestedOrientation;
    bool displayOrientationChangeRequested;
    bool useDMAAcceleration;
    RenderingMethod lastRenderMethod;
    bool isFrontPorchEntered;
    uint8_t numberOfBlocks;
    uint16_t maxDrawingHeight;
    uint16_t minDrawingHeight;
    int16_t maxBlockLines;

    uint16_t* getDstAddress(uint16_t x, uint16_t y, uint16_t* startAddress, uint16_t dstWidth, Bitmap::BitmapFormat dstFormat) const;
    uint16_t* getDstAddress(uint16_t x, uint16_t y, uint16_t* startAddress) const;
    uint8_t getBitDepth(Bitmap::BitmapFormat format) const;
};

} // namespace touchgfx

#endif // TOUCHGFX_HAL_HPP
