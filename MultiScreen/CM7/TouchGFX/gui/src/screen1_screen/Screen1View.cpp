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

void Screen1View::TogglePressed(){

Unicode::snprintf(textArea1Buffer, TEXTAREA1_SIZE, "%d",toggleButton1.getState()? 1:0);
textArea1.invalidate();
presenter->saveToggleButton(toggleButton1.getState());
}


void Screen1View::setToggleState(bool state){

	toggleButton1.forceState(state);
	Unicode::snprintf(textArea1Buffer, TEXTAREA1_SIZE, "%d",state?1:0);
}
