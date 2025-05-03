#ifndef MODELLISTENER_HPP
#define MODELLISTENER_HPP

#include <gui/model/Model.hpp>

class ModelListener
{
public:
    ModelListener() : model(0) {}
    
    virtual ~ModelListener() {}

    void bind(Model* m)
    {
        model = m;
    }

    virtual void setVolume(int val){};
    virtual void setButton(bool btnState){};
protected:
    Model* model;
};

#endif // MODELLISTENER_HPP
