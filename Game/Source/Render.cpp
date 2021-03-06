#include "App.h"
#include "Window.h"
#include "Render.h"
#include "Player.h"
#include "Input.h"
#include "Scene.h"

#include "Defs.h"
#include "Log.h"

#define VSYNC true

Render::Render() : Module()
{
	name.Create("renderer");
	background.r = 0;
	background.g = 0;
	background.b = 0;
	background.a = 0;
}

// Destructor
Render::~Render() {}

// Called before render is available
bool Render::Awake(pugi::xml_node& config)
{
	LOG("Create SDL rendering context");
	bool ret = true;

	Uint32 flags = SDL_RENDERER_ACCELERATED;

	if(config.child("vsync").attribute("value").as_bool(true) == true)
	{
		flags |= SDL_RENDERER_PRESENTVSYNC;
		LOG("Using vsync");
	}

	renderer = SDL_CreateRenderer(app->win->window, -1, flags);

	if(renderer == NULL)
	{
		LOG("Could not create the renderer! SDL_Error: %s\n", SDL_GetError());
		ret = false;
	}
	else
	{
		camera.w = app->win->screenSurface->w;
		camera.h = app->win->screenSurface->h;
		camera.x = 0;
		camera.y = 0;
	}

	return ret;
}

// Called before the first frame
bool Render::Start()
{
	LOG("render start");
	// back background
	SDL_RenderGetViewport(renderer, &viewport);
	drawLayerColliders = false;

	return true;
}

// Called each loop iteration
bool Render::PreUpdate()
{
	SDL_RenderClear(renderer);
	return true;
}

bool Render::Update(float dt)
{
	if (app->input->GetKey(SDL_SCANCODE_F9) == KEY_DOWN)
	{

		drawLayerColliders = !drawLayerColliders;

	}

	if (app->input->GetKey(SDL_SCANCODE_F8) == KEY_DOWN)
	{

		drawButtonsColliders = !drawButtonsColliders;

	}
	return true;
}

bool Render::PostUpdate()
{
	SDL_SetRenderDrawColor(renderer, background.r, background.g, background.g, background.a);
	SDL_RenderPresent(renderer);
	return true;
}

// Called before quitting
bool Render::CleanUp()
{
	LOG("Destroying SDL render");
	SDL_DestroyRenderer(renderer);
	return true;
}

void Render::SetBackgroundColor(SDL_Color color) { background = color; }

void Render::SetViewPort(const SDL_Rect& rect) { SDL_RenderSetViewport(renderer, &rect); }

void Render::ResetViewPort() { SDL_RenderSetViewport(renderer, &viewport); }

// Blit to screen
bool Render::DrawTexture(SDL_Texture* texture, int x, int y, const SDL_Rect* section, bool invert,  float speed, double angle, int pivotX, int pivotY) const
{
	bool ret = true;
	uint scale = app->win->GetScale();

	SDL_Rect rect;
	rect.x = (int)(camera.x * speed) + x * scale;
	rect.y = (int)(camera.y * speed) + y * scale;

	if (section != NULL)
	{
		rect.w = section->w;
		rect.h = section->h;
	}
	else { SDL_QueryTexture(texture, NULL, NULL, &rect.w, &rect.h); }

	rect.w *= scale;
	rect.h *= scale;

	SDL_Point* p = NULL;
	SDL_Point pivot;

	if(pivotX != INT_MAX && pivotY != INT_MAX)
	{
		pivot.x = pivotX;
		pivot.y = pivotY;
		p = &pivot;
	}

	SDL_RendererFlip flip = SDL_FLIP_NONE;
	if (invert) { flip = SDL_FLIP_HORIZONTAL; }

	if(SDL_RenderCopyEx(renderer, texture, section, &rect, angle, p, flip) != 0)
	{
		LOG("Cannot blit to screen. SDL_RenderCopy error: %s", SDL_GetError());
		ret = false;
	}
	return ret;
}

bool Render::DrawRectangle(const SDL_Rect& rect, Uint8 r, Uint8 g, Uint8 b, Uint8 a, bool filled, bool use_camera) const
{
	bool ret = true;
	uint scale = app->win->GetScale();

	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(renderer, r, g, b, a);

	SDL_Rect rec(rect);
	if(use_camera)
	{
		rec.x = (int)(camera.x + rect.x * scale);
		rec.y = (int)(camera.y + rect.y * scale);
		rec.w *= scale;
		rec.h *= scale;
	}

	int result = (filled) ? SDL_RenderFillRect(renderer, &rec) : SDL_RenderDrawRect(renderer, &rec);

	if(result != 0)
	{
		LOG("Cannot draw quad to screen. SDL_RenderFillRect error: %s", SDL_GetError());
		ret = false;
	}
	return ret;
}

bool Render::DrawLine(int x1, int y1, int x2, int y2, Uint8 r, Uint8 g, Uint8 b, Uint8 a, bool use_camera) const
{
	bool ret = true;
	uint scale = app->win->GetScale();

	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(renderer, r, g, b, a);

	int result = -1;

	if(use_camera)
		result = SDL_RenderDrawLine(renderer, camera.x + x1 * scale, camera.y + y1 * scale, camera.x + x2 * scale, camera.y + y2 * scale);
	else
		result = SDL_RenderDrawLine(renderer, x1 * scale, y1 * scale, x2 * scale, y2 * scale);

	if(result != 0)
	{
		LOG("Cannot draw quad to screen. SDL_RenderFillRect error: %s", SDL_GetError());
		ret = false;
	}

	return ret;
}

bool Render::DrawCircle(int x, int y, int radius, Uint8 r, Uint8 g, Uint8 b, Uint8 a, bool use_camera) const
{
	bool ret = true;
	uint scale = app->win->GetScale();

	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(renderer, r, g, b, a);

	int result = -1;
	SDL_Point points[360];

	float factor = (float)M_PI / 180.0f;

	for(uint i = 0; i < 360; ++i)
	{
		points[i].x = (int)(x + radius * cos(i * factor));
		points[i].y = (int)(y + radius * sin(i * factor));
	}

	result = SDL_RenderDrawPoints(renderer, points, 360);

	if(result != 0)
	{
		LOG("Cannot draw quad to screen. SDL_RenderFillRect error: %s", SDL_GetError());
		ret = false;
	}

	return ret;
}

bool Render::LoadState(pugi::xml_node& data)
{
	LOG("Loading render info");
	bool ret = true;

	app->scene->player->entityRect.x = data.child("player").attribute("x").as_int();
	app->scene->player->entityRect.y = data.child("player").attribute("y").as_int();
	app->scene->seconds = data.child("time").attribute("seconds").as_float();
	app->scene->minutes = data.child("time").attribute("minutes").as_float();
	app->scene->fly->entityRect.x = data.child("fly").attribute("x").as_int();
	app->scene->fly->entityRect.y = data.child("fly").attribute("y").as_int();
	app->scene->fly->heDed = data.child("fly").attribute("ded").as_bool();
	app->scene->slime->entityRect.x = data.child("slime").attribute("x").as_int();
	app->scene->slime->entityRect.y = data.child("slime").attribute("y").as_int();
	app->scene->player->heDed = data.child("slime").attribute("ded").as_bool();
	app->scene->coins = data.child("coins").attribute("amount").as_int();
	app->scene->score = data.child("score").attribute("amount").as_int();

	return ret;
}

// Save Game State
bool Render::SaveState(pugi::xml_node& data)
{
	LOG("Saving render info");
	bool ret = true;

	pugi::xml_node ply = data.append_child("player");
	pugi::xml_node time = data.append_child("time");
	pugi::xml_node fly = data.append_child("fly");
	pugi::xml_node slime = data.append_child("slime");
	pugi::xml_node coins = data.append_child("coins");
	pugi::xml_node score = data.append_child("score");


	ply.append_attribute("x").set_value(app->scene->player->entityRect.x);
	ply.append_attribute("y").set_value(app->scene->player->entityRect.y);
	time.append_attribute("seconds").set_value(app->scene->seconds);
	time.append_attribute("minutes").set_value(app->scene->minutes);
	fly.append_attribute("x").set_value(app->scene->fly->entityRect.x);
	fly.append_attribute("y").set_value(app->scene->fly->entityRect.y);
	fly.append_attribute("ded").set_value(app->scene->fly->heDed);
	slime.append_attribute("x").set_value(app->scene->slime->entityRect.x);
	slime.append_attribute("y").set_value(app->scene->slime->entityRect.y);
	slime.append_attribute("ded").set_value(app->scene->slime->heDed);
	coins.append_attribute("amount").set_value(app->scene->coins);
	score.append_attribute("amount").set_value(app->scene->score);

	return ret;
}
