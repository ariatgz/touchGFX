#include <gui/screen2_screen/Screen2View.hpp>

Screen2View::Screen2View()
{

}

void Screen2View::setupScreen()
{
    Screen2ViewBase::setupScreen();
}

void Screen2View::tearDownScreen()
{
    Screen2ViewBase::tearDownScreen();
}

void Screen2View::SliderUpdated(int value){

    Unicode::snprintf(textArea1Buffer, TEXTAREA1_SIZE, "%d", value);
    textArea1.invalidate();
    presenter->setSliderValue(value);


}


void Screen2View::setSlider(int val){

	slider1.setValue(val);

}
