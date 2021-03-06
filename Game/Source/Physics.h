#ifndef __PHYSICS_H__
#define __PHYSICS_H__

#include "Point.h"
#include "App.h"
#include "Collisions.h"
#include "Map.h"

#include "SDL/include/SDL.h"


class Collider;


struct Physics
{
public:
	
	void UpdatePhysics(iPoint& pos, float deltaTime)
	{
		if (axisX)
		{
			pos.x = pos.x + speed.x * deltaTime;
		}
		if (axisY)
		{
			//Euler
			//y = y + sy * dt;

			// Verlet
			pos.y = pos.y + speed.y * deltaTime + (gravity * deltaTime * deltaTime * 0.5);


			speed.y = speed.y + gravity * deltaTime;
		}
	}


	// To know what direction the velocity is going
	void CheckDirection()
	{
		if (speed.y >= 0.0f)
		{
			positiveSpeedY = true;
		}
		else if (speed.y < 0.0f)
		{
			positiveSpeedY = false;
		}
	}



	// Collisions
	void ResolveCollisions(SDL_Rect& currentFrame, iPoint nextFrame, bool goingLeft)
	{
		iPoint tiledPos(currentFrame.x / 64, currentFrame.y / 64);
		iPoint correctedPos;
		iPoint checkedPos;
		//LOG("past: %d,%d current: %d,%d\n", playerRect.x, playerRect.y, nextFrame.x, nextFrame.y);

		// X axis
		if (!goingLeft) { // right
			tiledPos.x = (currentFrame.x + currentFrame.w) / 64;
			int i = 0;
			while ((app->map->GetTileProperty(tiledPos.x + i, tiledPos.y, "Collider") == Collider::Type::AIR || app->map->GetTileProperty(tiledPos.x + i,  tiledPos.y,  "Collider") == Collider::Type::BOX || app->map->GetTileProperty(tiledPos.x+i, tiledPos.y, "Collider") == Collider::Type::CHECKPOINT) && i < 5)
			{
				i++;
			}
			i--;
			correctedPos.x = MIN(nextFrame.x - currentFrame.x, (tiledPos.x + i) * 64 - currentFrame.x);
		}
		else { // left
			int i = 0;
			while ((app->map->GetTileProperty(tiledPos.x - i, tiledPos.y, "Collider") == Collider::Type::AIR || app->map->GetTileProperty(tiledPos.x - i, tiledPos.y, "Collider") == Collider::Type::BOX || app->map->GetTileProperty(tiledPos.x - i, tiledPos.y, "Collider") == Collider::Type::CHECKPOINT) && i < 5)
			{
				i++;
			}
			i--;
			correctedPos.x = -MIN(currentFrame.x - nextFrame.x, currentFrame.x - (tiledPos.x - i) * 64);
		}

		// Y axis

		if (positiveSpeedY) {
			tiledPos.y = (currentFrame.y + currentFrame.h) / 64;
			int i = 0;
			while ((app->map->GetTileProperty(tiledPos.x, tiledPos.y + i, "Collider") == Collider::Type::AIR || app->map->GetTileProperty(tiledPos.x, tiledPos.y + i, "Collider") == Collider::Type::BOX || app->map->GetTileProperty(tiledPos.x, tiledPos.y + i, "Collider") == Collider::Type::CHECKPOINT) && i < 5)
			{
				i++;
			}
			i--;
			correctedPos.y = MIN(nextFrame.y - currentFrame.y, (tiledPos.y + i) * 64 - currentFrame.y);
		}
		else {
			int i = 0;
			while (app->map->GetTileProperty(tiledPos.x, tiledPos.y - i, "Collider") == Collider::Type::AIR || app->map->GetTileProperty(tiledPos.x, tiledPos.y - i, "Collider") == Collider::Type::BOX && i < 5)
			{
				i++;
			}
			i--;
			correctedPos.y = -MIN(currentFrame.y - nextFrame.y, currentFrame.y - (tiledPos.y - i) * 64);
		}

		currentFrame.x += correctedPos.x;
		currentFrame.y += correctedPos.y;


		if (app->map->GetTileProperty(currentFrame.x / 64 + 1, currentFrame.y / 64, "Collider") == Collider::Type::SOLID)
		{
			speed.x = 0.0f;
			currentFrame.x -= correctedPos.x;
			
		}
		else if (app->map->GetTileProperty(currentFrame.x / 64, currentFrame.y / 64, "Collider") == Collider::Type::SOLID)
		{
			speed.x = 0.0f;
			currentFrame.x -= correctedPos.x;
			
		}
		else if (app->map->GetTileProperty(currentFrame.x / 64, currentFrame.y / 64 + 1, "Collider") == Collider::Type::SOLID) {
			speed.y = 0.0f;
		}
		else if (app->map->GetTileProperty(currentFrame.x / 64, currentFrame.y / 64, "Collider") == Collider::Type::SOLID ) {
			speed.y = 0.0f;
		}


	}

	bool axisY;
	bool axisX;
	bool positiveSpeedY;
	bool verlet = true;
	fPoint speed;
	float gravity = 950.0f;

};

#endif // !__PHYSICS_H__