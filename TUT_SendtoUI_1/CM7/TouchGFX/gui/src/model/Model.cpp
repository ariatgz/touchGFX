#include <gui/model/Model.hpp>
#include <gui/model/ModelListener.hpp>
#ifndef SIMULATOR
#include "main.h"
extern "C"{

	extern ADC_HandleTypeDef hadc3;

}

long map(long x, long in_min,long in_max,long out_min,long out_max){
	return (x - in_min)*(out_max - out_min +1) / (in_max - in_min +1) + out_min;
}
#endif

Model::Model() : modelListener(0) , Button_state(false), ADC_VAL(500)
{

}

void Model::tick()
{
#ifndef SIMULATOR
	if(HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13)){
		Button_state = true;
	}else{
		Button_state = false;
	}

	HAL_ADC_Start(&hadc3);
	HAL_ADC_PollForConversion(&hadc3,10);

	uint16_t val = HAL_ADC_GetValue(&hadc3);
	HAL_ADC_Stop(&hadc3);

	ADC_VAL= map(val,0,4095,0,100);
#endif

	modelListener->setLight(Button_state);
	modelListener->setVolume(ADC_VAL);
}
