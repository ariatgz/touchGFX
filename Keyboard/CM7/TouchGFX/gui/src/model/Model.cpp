#include <gui/model/Model.hpp>
#include <gui/model/ModelListener.hpp>

#include"main.h"
#include "string.h"
#include "cmsis_os2.h"
extern "C"{

 extern UART_HandleTypeDef huart1;
 osMessageQueueId_t uart_dataHandle;
 uart_d data_q;

}


Model::Model() : modelListener(0)
{


}

void Model::tick()
{

}

void Model::sendData(char *data){

	HAL_UART_Transmit(&huart1,(uint8_t *)data,strlen(data),100);


}
