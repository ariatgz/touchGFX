#include <gui/model/Model.hpp>
#include <gui/model/ModelListener.hpp>

#ifndef SIMULATION
#include "cmsis_os2.h"
#include "main.h"

extern "C"{


 extern osMessageQueueId_t ADCQueueHandle;
 extern osMessageQueueId_t ButtonQueueHandle;

}

#endif

Model::Model() : modelListener(0), ADC_Value(30), Button_state(false)
{

}

void Model::tick()
{

#ifndef SIMULATION

	if(osMessageQueueGetCount(ADCQueueHandle)>0){

		if(osMessageQueueGet(ADCQueueHandle, &ADC_Value, 0, 0) == osOK){

				modelListener->setVolume(ADC_Value);

		}

	}

	static int button;
	if(osMessageQueueGet(ButtonQueueHandle, &button, 0, 0)  == osOK){

		if(button) Button_state = true;
		else Button_state = false;

		modelListener->setButton(Button_state);
	}

#endif
}
