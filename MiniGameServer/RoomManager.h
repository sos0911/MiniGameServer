#pragma once
#include <map>
#include "Singleton.h"

class map;

class RoomManager : public Singleton<RoomManager>
{
private:
	friend class Singleton;
	RoomManager();
	RoomManager(const RoomManager& ref) = delete;
	RoomManager& operator=(const RoomManager& ref) = delete;
	virtual ~RoomManager() {}

public:

};