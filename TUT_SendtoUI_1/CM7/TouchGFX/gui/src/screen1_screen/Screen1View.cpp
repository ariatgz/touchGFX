#include <gui/screen1_screen/Screen1View.hpp>

Screen1View::Screen1View()
{

}

void Screen1View::setupScreen()
{
    Screen1ViewBase::setupScreen();
}

void Screen1View::tearDownScreen()
{
    Screen1ViewBase::tearDownScreen();
}

void Screen1View::setLight (bool state){

	LightON.setVisible(state);
	LightON.invalidate();
}

void Screen1View::setVolume(int ADC_VAL){

    Unicode::snprintf(textArea1Buffer, TEXTAREA1_SIZE, "%d", ADC_VAL);
    textArea1.invalidate();

}
