/* Engine.h
Copyright (c) 2014 by Michael Zahniser

Endless Sky is free software: you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later version.

Endless Sky is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details.
*/

#ifndef ENGINE_H_
#define ENGINE_H_

#include "AI.h"
#include "AsteroidField.h"
#include "DrawList.h"
#include "EscortDisplay.h"
#include "Information.h"
#include "Point.h"
#include "Projectile.h"
#include "Radar.h"
#include "Ship.h"
#include "ShipEvent.h"

#include <condition_variable>
#include <list>
#include <map>
#include <memory>
#include <thread>
#include <vector>

class Government;
class Outfit;
class PlayerInfo;
class StellarObject;



// Class representing the game engine: its job is to track all of the objects in
// the game, and to move them, step by step. All the motion and collision
// calculations are handled in a separate thread so that the graphics thread is
// free to just work on drawing things; this means that the drawn state of the
// game is always one step (1/60 second) behind what is being calculated. This
// lag is too small to be detectable and means that the game can better handle
// situations where there are many objects on screen at once.
class Engine {
public:
	Engine(PlayerInfo &player);
	~Engine();
	
	// Place all the player's ships, and "enter" the system the player is in.
	void Place();
	
	// Wait for the previous calculations (if any) to be done.
	void Wait();
	// Perform all the work that can only be done while the calculation thread
	// is paused (for thread safety reasons).
	void Step(bool isActive);
	// Begin the next step of calculations.
	void Go();
	
	// Get any special events that happened in this step.
	const std::list<ShipEvent> &Events() const;
	
	// Draw a frame.
	void Draw() const;
	
	// Select the object the player clicked on.
	void Click(const Point &point);
	
	
private:
	void EnterSystem();
	
	void ThreadEntryPoint();
	void CalculateStep();
	void AddSprites(const Ship &ship, const Point &position, const Point &velocity);
	void AddSprites(const Ship &ship, const Point &position, const Point &velocity, const Point &unit, double cloak);
	
	void DoGrudge(const std::shared_ptr<Ship> &target, const Government *attacker);
	
	
private:
	class Target {
	public:
		Point center;
		Angle angle;
		double radius;
		int type;
	};
	
	class Status {
	public:
		Status(const Point &position, double shields, double hull, double radius, bool isEnemy);
		
		Point position;
		double shields;
		double hull;
		double radius;
		bool isEnemy;
	};
	
	class Label {
	public:
		Label(const Point &position, const StellarObject &object);
		
		Point position;
		double radius;
		std::string name;
		std::string government;
		Color color;
		int hostility = 0;
	};
	
	
private:
	PlayerInfo &player;
	
	AI ai;
	
	std::thread calcThread;
	std::condition_variable condition;
	std::mutex swapMutex;
	
	Point center;
	bool calcTickTock = false;
	bool drawTickTock = false;
	bool terminate = false;
	bool wasActive = false;
	DrawList draw[2];
	Radar radar[2];
	// Viewport position and velocity.
	Point position;
	Point velocity;
	// Other information to display.
	Information info;
	std::vector<Target> targets;
	Point targetAngle;
	Point targetUnit;
	EscortDisplay escorts;
	std::vector<Status> statuses;
	std::vector<Label> labels;
	std::vector<std::pair<const Outfit *, int>> ammo;
	
	int step = 0;
	
	std::list<std::shared_ptr<Ship>> ships;
	std::list<Projectile> projectiles;
	std::list<Effect> effects;
	// Keep track of which ships we have not seen for long enough that it is
	// time to stop tracking their movements.
	std::map<std::list<Ship>::iterator, int> forget;
	
	std::list<ShipEvent> eventQueue;
	std::list<ShipEvent> events;
	// Keep track of who has asked for help in fighting whom.
	std::map<const Government *, std::weak_ptr<const Ship>> grudge;
	
	AsteroidField asteroids;
	double flash = 0.;
	bool doFlash = false;
	bool doEnter = false;
	
	bool doClick = false;
	Command clickCommands;
	Point clickPoint;
	
	double load = 0.;
	int loadCount = 0;
	double loadSum = 0.;
};



#endif
