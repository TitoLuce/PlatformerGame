#ifndef __COLLISIONS_H__
#define __COLLISIONS_H__

#define MAX_COLLIDERS 75

#include "Module.h"

#include "SDL/include/SDL.h"

class Collider
{
public:
	enum Type
	{
		NONE = -1,
		AIR,
		SOLID,
		PAIN,
		COIN,
		BOX,
		CHECKPOINT,
		//SMOLSOLID,
		//END,
		/*HALFPAINDOWN,
		HALFPAINUP,
		HALFPAINLEFT,
		HALFPAINRIGHT,
		UPPERSOLIDPAIN,
		RIGHTSOLIDPAIN,
		LEFTSOLIDPAIN,
		LOWERSOLIDPAIN,*/
		PLAYER,
		ENEMY,
		ATTACK,
		MAX
	};

	Collider(SDL_Rect _rect, Type _type, Module* _listener = nullptr) : rect(_rect), type(_type), listener(_listener) {}

	void SetPos(int _x, int _y, int _w, int _h);

	bool Intersects(const SDL_Rect& r) const;

	SDL_Rect rect;
	bool pendingToDelete = false;
	Type type;
	Module* listener = nullptr;
};

class Collisions : public Module
{
public:
	Collisions();

	void Init();

	// Destructor
	virtual ~Collisions();

	// Called before player is available
	bool Awake(pugi::xml_node&);

	// Called before the first frame
	bool Start();

	// Called each loop iteration
	bool PreUpdate();
	bool Update(float dt);
	bool PostUpdate();

	// Called before quitting
	bool CleanUp();

	bool Load(pugi::xml_node&);
	bool Save(pugi::xml_node&);

	// Adds a new collider to the list
	Collider* AddCollider(SDL_Rect rect, Collider::Type type, Module* listener = nullptr);

private:
	// All existing colliders in the scene
	Collider* colliders[MAX_COLLIDERS] = { nullptr };


	// The collision matrix. Defines the interaction for two collider types
	// If set two false, collider 1 will ignore collider 2
	bool matrix[Collider::Type::MAX][Collider::Type::MAX];
};

#endif // !__COLLISIONS_H__
