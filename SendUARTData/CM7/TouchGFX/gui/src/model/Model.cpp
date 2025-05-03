#include <gui/model/Model.hpp>
#include <gui/model/ModelListener.hpp>

#ifndef SIMULATION

#include "main.h"
#include "cmsis_os2.h"
#include "string.h"

extern "C"{
extern osMessageQueueId_t uartQueueHandle;
 uart_data *newdata;


}


#endif

Model::Model() : modelListener(0)
{

}

void Model::tick()
{
#ifndef SIMULATION

	if(osMessageQueueGetCount(uartQueueHandle)>0){
		if(osMessageQueueGet(uartQueueHandle, &newdata, 0, 0) == osOK){

			strncpy(Rdata, newdata->Data,newdata->size);
			modelListener->GetUart(Rdata);

		}

	}
#endif

}
