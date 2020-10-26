#ifndef __MODULETRANSITION_H__
#define __MODULETRANSITION_H__

#include "Module.h"
#include "SDL\include\SDL_rect.h"

class Transition : public Module {
public:
    //Constructor
    Transition();
    //Destructor
    ~Transition();
    // Called when the module is activated
    // Enables the blending mode for transparency
    bool Start();
    // Called at the middle of the application loop
    // Updates the fade logic
    bool Update();
    // Called at the end of the application loop
    // Performs the render call of a black rectangle with transparency
    bool PostUpdate();
    // Called from another module
    // Starts the fade process which has two steps, fade_out and fade_in
    // After the first step, the modules should be switched
	bool TransitionStep(Module* toDisable, Module* toEnable, bool onlyFadeIn = false, float frames = 60);

private:

    enum Transition_Step {
        NONE,
        TO_BLACK,
        FROM_BLACK
    } currentStep = Transition_Step::NONE;

    // A frame count system to handle the fade time and ratio
    Uint32 frameCount = 0;
    Uint32 maxFadeFrames = 0;

    // The rectangle of the screen, used to render the black rectangle
    SDL_Rect screenRect;

    // The modules that should be switched after the first step
    Module* moduleToEnable = nullptr;
    Module* moduleToDisable = nullptr;
};

#endif //__MODULEFADETOBLACK_H__