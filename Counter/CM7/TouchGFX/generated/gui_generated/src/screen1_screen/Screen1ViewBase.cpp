/*********************************************************************************/
/********** THIS FILE IS GENERATED BY TOUCHGFX DESIGNER, DO NOT MODIFY ***********/
/*********************************************************************************/
#include <gui_generated/screen1_screen/Screen1ViewBase.hpp>
#include <touchgfx/Color.hpp>
#include <images/BitmapDatabase.hpp>
#include <texts/TextKeysAndLanguages.hpp>

Screen1ViewBase::Screen1ViewBase() :
    flexButtonCallback(this, &Screen1ViewBase::flexButtonCallbackHandler)
{
    __background.setPosition(0, 0, 800, 480);
    __background.setColor(touchgfx::Color::getColorFromRGB(0, 0, 0));
    add(__background);

    image1.setXY(-132, -100);
    image1.setBitmap(touchgfx::Bitmap(BITMAP_ALTERNATE_THEME_IMAGES_BACKGROUNDS_1024X600_LINES_ID));
    add(image1);

    scalableImage1.setBitmap(touchgfx::Bitmap(BITMAP_ALTERNATE_THEME_IMAGES_TEXTURES_SEAMLESS_TEXTURE_1_ID));
    scalableImage1.setPosition(239, 138, 322, 125);
    scalableImage1.setScalingAlgorithm(touchgfx::ScalableImage::NEAREST_NEIGHBOR);
    add(scalableImage1);

    textArea1.setPosition(308, 190, 184, 43);
    textArea1.setColor(touchgfx::Color::getColorFromRGB(255, 250, 250));
    textArea1.setLinespacing(0);
    Unicode::snprintf(textArea1Buffer, TEXTAREA1_SIZE, "%s", touchgfx::TypedText(T_WC).getText());
    textArea1.setWildcard(textArea1Buffer);
    textArea1.setTypedText(touchgfx::TypedText(T___SINGLEUSE_XI3E));
    add(textArea1);

    flexButton1.setBitmaps(Bitmap(BITMAP_UP_ARROW_BUTTON_ID), Bitmap(BITMAP_UP_ARROW_BUTTON_ID));
    flexButton1.setBitmapXY(0, 0);
    flexButton1.setAction(flexButtonCallback);
    flexButton1.setPosition(239, 314, 76, 76);
    add(flexButton1);

    flexButton2.setBitmaps(Bitmap(BITMAP_DOWN_ARROW_ID), Bitmap(BITMAP_DOWN_ARROW_ID));
    flexButton2.setBitmapXY(0, 0);
    flexButton2.setAction(flexButtonCallback);
    flexButton2.setPosition(484, 314, 77, 76);
    add(flexButton2);
}

Screen1ViewBase::~Screen1ViewBase()
{

}

void Screen1ViewBase::setupScreen()
{

}

void Screen1ViewBase::flexButtonCallbackHandler(const touchgfx::AbstractButtonContainer& src)
{
    if (&src == &flexButton1)
    {
        //Interaction1
        //When flexButton1 clicked call virtual function
        //Call Up_Clicked
        Up_Clicked();
    }
    if (&src == &flexButton2)
    {
        //Interaction2
        //When flexButton2 clicked call virtual function
        //Call DownClicked
        DownClicked();
    }
}
