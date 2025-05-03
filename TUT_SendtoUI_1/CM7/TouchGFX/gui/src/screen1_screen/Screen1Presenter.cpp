#include <gui/screen1_screen/Screen1View.hpp>
#include <gui/screen1_screen/Screen1Presenter.hpp>

Screen1Presenter::Screen1Presenter(Screen1View& v)
    : view(v)
{

}

void Screen1Presenter::activate()
{

}

void Screen1Presenter::deactivate()
{

}

void Screen1Presenter::setLight (bool state){


	view.setLight(state);


}

void Screen1Presenter::setVolume(int ADC_VAL){

	view.setVolume(ADC_VAL);

};
