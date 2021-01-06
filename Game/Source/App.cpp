#include "App.h"
#include "Window.h"
#include "Input.h"
#include "Render.h"
#include "Textures.h"
#include "Audio.h"
#include "EntityManager.h"
#include "GuiManager.h"
#include "ModuleFonts.h"

#include "Scene.h"
#include "Map.h"
#include "TitleScreen.h"
#include "Transition.h"
#include "Logo Screen.h"
#include "Player.h"
#include "Collisions.h"
#include "EnemyHandler.h"
#include "PathFinding.h"

#include "Defs.h"
#include "Log.h"

#include <iostream>
#include <sstream>

// Constructor
App::App(int argc, char* args[]) : argc(argc), args(args)
{

	PERF_START(ptimer);


	input = new Input();
	win = new Window();
	render = new Render();
	tex = new Textures();
	audio = new Audio();
	entityManager = new EntityManager();
	guiManager = new GuiManager();
	fonts = new ModuleFonts();

	scene = new Scene();
	map = new Map();
	titleScreen = new TitleScreen();
	transition = new Transition();
	logoScreen = new LogoScreen();
	collisions = new Collisions();
	//player = new Player();
	//enemies = new EnemyHandler();
	pathfinding = new PathFinding();

	//Todo lo que tiene que ver con enemies esta comentado

	// Ordered for awake / Start / Update
	// Reverse order of CleanUp
	
	//Dont forget to declare new ones higher up -Luce TO: Luce, you doofus


	AddModule(input);
	AddModule(win);
	AddModule(tex);
	AddModule(audio);
	AddModule(guiManager);
	AddModule(entityManager);
	AddModule(fonts);

	AddModule(pathfinding);
	AddModule(logoScreen);
	AddModule(titleScreen);
	AddModule(scene);
	AddModule(map);
	//AddModule(player);
	//AddModule(enemies);
	AddModule(collisions);
	AddModule(transition);

	// render last to swap buffer
	AddModule(render);



	PERF_PEEK(ptimer);
}

// Destructor
App::~App()
{
	// release modules
	ListItem<Module*>* item = modules.end;

	while(item != NULL)
	{
		RELEASE(item->data);
		item = item->prev;
	}
	modules.Clear();
}

void App::AddModule(Module* module)
{
	module->Init();
	modules.Add(module);
}

// Called before render is available
bool App::Awake()
{

	PERF_START(ptimer);

	pugi::xml_document configFile;
	pugi::xml_node config;
	pugi::xml_node configApp;

	bool ret = false;

	config = LoadConfig(configFile);

	if (config.empty() == false)
	{
		ret = true;
		configApp = config.child("app");

		title.Create(configApp.child("title").child_value());
		organization.Create(configApp.child("organization").child_value());


		// Read from config file your framerate cap
		int cap = configApp.attribute("framerate_cap").as_int(-1); // -1 = No cap

		if (cap > 0) cappedMs = 1000 / cap;
	}

	if(ret == true)
	{
		ListItem<Module*>* item;
		item = modules.start;
		while(item != NULL && ret == true)
		{
			ret = item->data->Awake(config.child(item->data->name.GetString()));
			item = item->next;
		}
	}

	PERF_PEEK(ptimer);

	return ret;
}

// Called before the first frame
bool App::Start()
{

	PERF_START(ptimer);

	bool ret = true;
	ListItem<Module*>* item;
	item = modules.start;

	while(item != NULL && ret == true)
	{
		if (item->data->active == true)
		{
			ret = item->data->Start();
		}
		item = item->next;
	}


	PERF_PEEK(ptimer);

	return ret;
}

// Called each loop iteration
bool App::Update()
{
	bool ret = true;
	PrepareUpdate();

	if(input->GetWindowEvent(WE_QUIT) == true)
		ret = false;

	if(ret == true)
		ret = PreUpdate();

	if(ret == true)
		ret = DoUpdate();

	if(ret == true)
		ret = PostUpdate();

	FinishUpdate();
	return ret;
}

//  Load config from XML file
pugi::xml_node App::LoadConfig(pugi::xml_document& configFile) const
{
	pugi::xml_node ret;
	pugi::xml_parse_result result = configFile.load_file("config.xml");

	if (result == NULL) LOG("Could not load xml file: %s. pugi error: %s", "config.xml", result.description());
	else ret = configFile.child("config");

	return ret;
}

// ---------------------------------------------
void App::PrepareUpdate() 
{
	frameCount++;
	lastSecFrameCount++;

	// Calculate the dt: differential time since last frame
	dt = frameTime.ReadSec();

	// We start the timer after read because we want to know how much time it took from the last frame to the new one
	PERF_START(frameTime);

}

// ---------------------------------------------
void App::FinishUpdate()
{
	// This is a good place to call Load / Save functions
	if (loadRequest)
	{
		loadRequest = !loadRequest;
		LoadGame();
	}
	else if (saveRequest)
	{
		saveRequest = !saveRequest;
		SaveGame();
	}


	// Framerate calculations------------------------------------------
	// To know how many frames have passed in the last second
	if (lastSecFrameTime.Read() > 1000)
	{
		lastSecFrameTime.Start();
		prevLastSecFrameCount = lastSecFrameCount;
		lastSecFrameCount = 0;
	}

	// Amount of seconds since startup
	float secondsSinceStartup = 0.0f;
	secondsSinceStartup = startupTime.ReadSec();

	// Amount of time since game start (use a low resolution timer)
	uint32 lastFrameMs = 0;
	lastFrameMs = frameTime.Read(); // Time from the prepare update until now (whole update method)

	// Average FPS for the whole game life (since start)
	float averageFps = 0.0f;
	averageFps = float(frameCount) / startupTime.ReadSec();

	// Amount of frames during the last update
	uint32 framesOnLastUpdate = 0;
	framesOnLastUpdate = prevLastSecFrameCount;

	static char title[256];

	sprintf_s(title, 256, "Av.FPS: %.2f Last Frame Ms: %02u Last sec frames: %i Last dt: %.3f Time since startup: %.3f Frame Count: %I64u ",
		averageFps, lastFrameMs, framesOnLastUpdate, dt, secondsSinceStartup, frameCount);
	app->win->SetTitle(title);

	// Use SDL_Delay to make sure you get your capped framerate
	PERF_START(ptimer);
	if (cappedMs > lastFrameMs)
	{
		SDL_Delay(cappedMs - lastFrameMs);
	}

	// Measure accurately the amount of time SDL_Delay() actually waits compared to what was expected
	PERF_PEEK(ptimer);
}

// Call modules before each loop iteration
bool App::PreUpdate()
{
	bool ret = true;
	ListItem<Module*>* item;
	item = modules.start;
	Module* pModule = NULL;

	for(item = modules.start; item != NULL && ret == true; item = item->next)
	{
		pModule = item->data;
		if (pModule->active == false) { continue; }
		ret = item->data->PreUpdate();
	}
	return ret;
}

// Call modules on each loop iteration
bool App::DoUpdate()
{
	bool ret = true;
	ListItem<Module*>* item;
	item = modules.start;
	Module* pModule = NULL;

	for(item = modules.start; item != NULL && ret == true; item = item->next)
	{
		pModule = item->data;
		if (pModule->active == false) { continue; }
		ret = item->data->Update(dt);
	}
	return ret;
}

// Call modules after each loop iteration
bool App::PostUpdate()
{
	bool ret = true;
	ListItem<Module*>* item;
	Module* pModule = NULL;

	for(item = modules.start; item != NULL && ret == true; item = item->next)
	{
		pModule = item->data;
		if (pModule->active == false) { continue; }
		ret = item->data->PostUpdate();
	}
	return ret;
}

// Called before quitting
bool App::CleanUp()
{
	bool ret = true;
	ListItem<Module*>* item;
	item = modules.end;

	while(item != NULL && ret == true)
	{
		ret = item->data->CleanUp();
		item = item->prev;
	}
	return ret;
}

int App::GetArgc() const { return argc; }

const char* App::GetArgv(int index) const
{
	if(index < argc)
		return args[index];
	else
		return NULL;
}

const char* App::GetTitle() const { return title.GetString(); }

const char* App::GetOrganization() const { return organization.GetString(); }

void App::LoadRequest() { loadRequest = true; }

void App::SaveRequest() { saveRequest = true; }

bool App::LoadGame()
{
	bool ret = true;

	pugi::xml_parse_result result = saveFile.load_file("savegame.xml");
	if (result == NULL)
	{
		LOG("Failed to load xml file savegame.xml. pugi error: %s", result.description());
		ret = false;
	}
	else
	{
		save = saveFile.child("save_state");
		ListItem<Module*>* item;
		item = modules.start;
		while (item != NULL && ret == true)
		{
			ret = item->data->LoadState(save.child(item->data->name.GetString()));
			item = item->next;
		}
	}
	return ret;
}

bool App::SaveGame()
{
	bool ret = true;
	pugi::xml_document newSaveFile;
	pugi::xml_node saveState = newSaveFile.append_child("save_state");

	ListItem<Module*>* item;
	item = modules.start;

	while (item != NULL && ret == true)
	{
		ret = item->data->SaveState(saveState.append_child(item->data->name.GetString()));
		item = item->next;
	}
	newSaveFile.save_file("savegame.xml");
	return ret;
}

