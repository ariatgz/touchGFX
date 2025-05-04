#include <gui/model/Model.hpp>
#include <gui/model/ModelListener.hpp>

#ifndef SIMULATION


#include "main.h"
extern "C"{

	extern UART_HandleTypeDef huart2;
	extern UART_HandleTypeDef huart1;

}

#endif

Model::Model() : modelListener(0)
{

}

void Model::tick()
{

}

void Model::SquareClicked(){


	HAL_UART_Transmit(&huart1, (uint8_t *)"SQ", 2, HAL_MAX_DELAY);
	HAL_GPIO_WritePin(GPIOI, GPIO_PIN_12, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOI, GPIO_PIN_13, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOI, GPIO_PIN_14, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOI, GPIO_PIN_15, GPIO_PIN_SET);

}

void Model::SineClicked(){

	HAL_UART_Transmit(&huart1,(uint8_t *) "SI", 2, HAL_MAX_DELAY);
	HAL_GPIO_WritePin(GPIOI, GPIO_PIN_12, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOI, GPIO_PIN_13, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOI, GPIO_PIN_14, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOI, GPIO_PIN_15, GPIO_PIN_SET);
}

void Model::TriAngleClicked(){

	HAL_UART_Transmit(&huart1,(uint8_t *) "TR", 2, HAL_MAX_DELAY);

	HAL_GPIO_WritePin(GPIOI, GPIO_PIN_12, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOI, GPIO_PIN_13, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOI, GPIO_PIN_14, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOI, GPIO_PIN_15, GPIO_PIN_SET);
}

void Model::TriClicked(){
	HAL_UART_Transmit(&huart1,(uint8_t *) "TN", 2, HAL_MAX_DELAY);
	HAL_GPIO_WritePin(GPIOI, GPIO_PIN_12, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOI, GPIO_PIN_13, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOI, GPIO_PIN_14, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOI, GPIO_PIN_15, GPIO_PIN_RESET);
}
