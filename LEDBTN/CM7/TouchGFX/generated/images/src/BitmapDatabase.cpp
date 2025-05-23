// 4.25.0 0x4b6770f8
// Generated by imageconverter. Please, do not edit!

#include <images/BitmapDatabase.hpp>
#include <touchgfx/Bitmap.hpp>

extern const unsigned char image_alternate_theme_images_backgrounds_1024x600_poly[]; // BITMAP_ALTERNATE_THEME_IMAGES_BACKGROUNDS_1024X600_POLY_ID = 0, Size: 1024x600 pixels
extern const unsigned char image_light_theme_images_widgets_togglebutton_large_rounded_text_off_normal[]; // BITMAP_LIGHT_THEME_IMAGES_WIDGETS_TOGGLEBUTTON_LARGE_ROUNDED_TEXT_OFF_NORMAL_ID = 1, Size: 170x60 pixels
extern const unsigned char image_light_theme_images_widgets_togglebutton_large_rounded_text_on_action[]; // BITMAP_LIGHT_THEME_IMAGES_WIDGETS_TOGGLEBUTTON_LARGE_ROUNDED_TEXT_ON_ACTION_ID = 2, Size: 170x60 pixels

const touchgfx::Bitmap::BitmapData bitmap_database[] = {
    { image_alternate_theme_images_backgrounds_1024x600_poly, 0, 1024, 600, 0, 0, 1024, ((uint8_t)touchgfx::Bitmap::RGB888) >> 3, 600, ((uint8_t)touchgfx::Bitmap::RGB888) & 0x7 },
    { image_light_theme_images_widgets_togglebutton_large_rounded_text_off_normal, 0, 170, 60, 11, 10, 71, ((uint8_t)touchgfx::Bitmap::ARGB8888) >> 3, 40, ((uint8_t)touchgfx::Bitmap::ARGB8888) & 0x7 },
    { image_light_theme_images_widgets_togglebutton_large_rounded_text_on_action, 0, 170, 60, 88, 10, 71, ((uint8_t)touchgfx::Bitmap::ARGB8888) >> 3, 40, ((uint8_t)touchgfx::Bitmap::ARGB8888) & 0x7 }
};

namespace BitmapDatabase
{
const touchgfx::Bitmap::BitmapData* getInstance()
{
    return bitmap_database;
}

uint16_t getInstanceSize()
{
    return (uint16_t)(sizeof(bitmap_database) / sizeof(touchgfx::Bitmap::BitmapData));
}
} // namespace BitmapDatabase
