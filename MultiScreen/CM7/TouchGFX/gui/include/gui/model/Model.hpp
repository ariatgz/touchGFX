#ifndef MODEL_HPP
#define MODEL_HPP

class ModelListener;

class Model
{
public:
    Model();

    void bind(ModelListener* listener)
    {
        modelListener = listener;
    }

    void tick();
    void saveToggleButton(bool state);
    bool getBtnState();
    void setSliderValue(int value);
    int getSliderValue();
protected:
    ModelListener* modelListener;
    bool btnState;
    int Slider_val;
};

#endif // MODEL_HPP
