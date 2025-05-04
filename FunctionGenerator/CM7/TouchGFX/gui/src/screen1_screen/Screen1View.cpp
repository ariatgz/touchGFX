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

void Screen1View::SquareClicked(){

	presenter->SquareClicked();

}

void Screen1View::SineClicked(){

	presenter->SineClicked();

}

void Screen1View::TriAngleClicked(){

	presenter->TriAngleClicked();

}

void Screen1View::TriClicked(){

	presenter->TriClicked();

}
