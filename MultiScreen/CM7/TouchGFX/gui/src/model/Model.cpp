#include <gui/model/Model.hpp>
#include <gui/model/ModelListener.hpp>

Model::Model() : modelListener(0), btnState(false),Slider_val(10)
{

}

void Model::tick()
{

}

void Model::saveToggleButton(bool state){

	btnState = state;


}

bool Model::getBtnState(){

	return btnState;

}


void Model::setSliderValue(int value){

	Slider_val = value;

}

int Model::getSliderValue(){

	return Slider_val;

}
