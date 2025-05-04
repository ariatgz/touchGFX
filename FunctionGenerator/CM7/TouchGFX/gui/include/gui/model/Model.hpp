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
    //Our virtual functions for buttons
     void SquareClicked();
     void SineClicked();
     void TriAngleClicked();
     void TriClicked();

protected:
    ModelListener* modelListener;
};

#endif // MODEL_HPP
