#ifndef SCREEN1VIEW_HPP
#define SCREEN1VIEW_HPP

#include <gui_generated/screen1_screen/Screen1ViewBase.hpp>
#include <gui/screen1_screen/Screen1Presenter.hpp>
#include <gui/common/CustomKeyboard.hpp>

class Screen1View : public Screen1ViewBase
{
public:
    Screen1View();
    virtual ~Screen1View() {}
    virtual void setupScreen();
    virtual void tearDownScreen();
    virtual void NameClicked();

       virtual void AgeClicked();

       virtual void OkClicked();

       virtual void ExitClicked();
       virtual void SendClicked();



protected:

    CustomKeyboard keyboard;
    int namemod;
    int agemod;
    char dataBuffer[50];
    char namebuf[TEXTAREANAME_SIZE];
    char agebuf[TEXTAREANAME_SIZE];

};

#endif // SCREEN1VIEW_HPP
