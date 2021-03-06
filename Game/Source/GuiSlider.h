#ifndef __GUISLIDER_H__
#define __GUISLIDER_H__

#include "GuiControl.h"

#include "Point.h"
#include "SString.h"

class GuiSlider : public GuiControl
{
public:

	GuiSlider(uint32 id, SDL_Rect bounds, SDL_Rect sliderBounds, const char *text);
	virtual ~GuiSlider();

	bool Update(float dt);
	bool Draw();

	int value;

private:

	// GuiSlider specific properties
	// Maybe some animation properties for state change?
	SDL_Rect slider;
	SDL_Rect sliderButton;
	SDL_Rect sliderButtonSelected;
	SDL_Rect sliderButtonPressed;

	//SDL_Rect sliderBounds;

	int minValue;
	int maxValue;

	bool clickPlay = true;
	bool hoverPlay = true;
};

#endif // __GUISLIDER_H__
