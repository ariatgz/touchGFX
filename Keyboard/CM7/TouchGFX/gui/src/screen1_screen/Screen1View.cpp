#include <gui/screen1_screen/Screen1View.hpp>
#include "stdio.h"
#include <cstring>

Screen1View::Screen1View()
{
	keyboard.setPosition(80,16,320,240);
	add(keyboard);
	keyboard.setVisible(false);
	keyboard.invalidate();

}

void Screen1View::setupScreen()
{
    Screen1ViewBase::setupScreen();
}

void Screen1View::tearDownScreen()
{
    Screen1ViewBase::tearDownScreen();
}

void Screen1View::NameClicked(){

	keyboard.setVisible(true);
	keyboard.invalidate();
	OKButton.setVisible(true);
	OKButton.invalidate();
	 buttonWithSend.setVisible(false);
	 buttonWithSend.invalidate();
	ExitButton.setVisible(true);
	ExitButton.invalidate();

	namemod = 1;
	agemod = 0;

}


void Screen1View::AgeClicked(){
	keyboard.setVisible(true);
		keyboard.invalidate();
		OKButton.setVisible(true);
		OKButton.invalidate();

		 buttonWithSend.setVisible(false);
		 buttonWithSend.invalidate();

		ExitButton.setVisible(true);
		ExitButton.invalidate();

		namemod = 0;
		agemod = 1;

}

void Screen1View::OkClicked(){
	keyboard.setVisible(false);
				keyboard.invalidate();
				OKButton.setVisible(false);
				OKButton.invalidate();

				ExitButton.setVisible(false);
				ExitButton.invalidate();

				buttonWithSend.setVisible(true);
					 buttonWithSend.invalidate();

				if (namemod){

					Unicode::strncpy(textAreaNameBuffer, keyboard.getBuffer(), TEXTAREANAME_SIZE);

					textAreaName.invalidate();
				}
				if (agemod){

					Unicode::strncpy(textAreaAgeBuffer, keyboard.getBuffer(), TEXTAREAAGE_SIZE);
					textAreaAge.invalidate();
			}

				keyboard.clearBuffer();

}

void Screen1View::ExitClicked(){

	keyboard.setVisible(false);
			keyboard.invalidate();
			OKButton.setVisible(false);
			OKButton.invalidate();

			ExitButton.setVisible(false);
			ExitButton.invalidate();

			buttonWithSend.setVisible(true);
				 buttonWithSend.invalidate();

			namemod = agemod = 0;

}

void Screen1View::SendClicked(){

		int i=0;

		do
		{

			namebuf[i] = (char) textAreaNameBuffer[i];
			i++;
		} while(textAreaNameBuffer[i] != 0);

		i=0;
		do
				{

					agebuf[i] = (char) textAreaAgeBuffer[i];
					i++;
				} while(textAreaAgeBuffer[i] != 0);

			sprintf(dataBuffer,"Name: %s, Age:%s \r\n",namebuf,agebuf);

			presenter->sendData(dataBuffer);

			memset(namebuf,'\0',TEXTAREANAME_SIZE);
			memset(agebuf,'\0',TEXTAREANAME_SIZE);

}

