#include "App.h"
#include "Input.h"
#include "Textures.h"
#include "Audio.h"
#include "Render.h"
#include "Window.h"
#include "Scene.h"
#include "Map.h"
#include "Player.h"

#include "Defs.h"
#include "Log.h"

Scene::Scene() : Module()
{
	name.Create("scene");
}

// Destructor
Scene::~Scene()
{}

// Called before render is available
bool Scene::Awake()
{
	LOG("Loading Scene");
	bool ret = true;

	return ret;
}

// Called before the first frame
bool Scene::Start()
{
	
	//app->audio->PlayMusic("Assets/audio/music/music_spy.ogg");

	app->render->camera.x = -(1600 - app->render->camera.w/2);
	app->render->camera.y = -(5120 + app->render->camera.h/2);

	app->map->Enable();
	app->map->Load("Level_1.tmx");
	app->player->Enable();


	return true;
}

// Called each loop iteration
bool Scene::PreUpdate()
{
	return true;
}

// Called each loop iteration
bool Scene::Update(float dt)
{
	if((app->render->camera.x + app->player->playerRect.x) < (app->map->data.tileWidth * 10))
	{
		app->render->camera.x += 5;
	}
	if ((app->player->playerRect.w + app->render->camera.x + app->player->playerRect.x) > (app->render->camera.w - app->map->data.tileWidth * 10))
	{
		app->render->camera.x -= 5;
	}
	if ((app->render->camera.y + app->player->playerRect.y) < (app->map->data.tileHeight * 6))
	{
		app->render->camera.y += 5;
	}
	if ((app->player->playerRect.h + app->render->camera.y + app->player->playerRect.y) > (app->render->camera.h - app->map->data.tileHeight * 6))
	{
		app->render->camera.y -= 5;
	}

	if (app->input->GetKey(SDL_SCANCODE_F5) == KEY_DOWN)
		app->SaveRequest();

	if (app->input->GetKey(SDL_SCANCODE_F6) == KEY_DOWN)
		app->LoadRequest();


	app->map->Draw();

	return true;
}

// Called each loop iteration
bool Scene::PostUpdate()
{
	bool ret = true;

	if(app->input->GetKey(SDL_SCANCODE_ESCAPE) == KEY_DOWN) ret = false;

	return ret;
}

// Called before quitting
bool Scene::CleanUp()
{
	LOG("Freeing scene");



	app->map->Disable();

	return true;
}


void Scene::Init()
{
	active = false;
}