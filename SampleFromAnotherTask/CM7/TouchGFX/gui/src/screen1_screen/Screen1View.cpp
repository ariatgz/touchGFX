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
void Screen1View::setVolume(int val){

	gauge1.setValue(val);
	gauge1.invalidate();

}
void Screen1View::setButton(bool btnState){

	if(!(animatedImage1.isAnimatedImageRunning())){
		if(btnState == true){
			animatedImage1.startAnimation(false, false, true);
		}
	}else if((animatedImage1.isAnimatedImageRunning())){
		if(btnState == false){
			animatedImage1.pauseAnimation();
		}
	}

}
