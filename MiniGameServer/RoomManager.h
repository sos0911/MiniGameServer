#pragma once
#include <map>
#include "Singleton.h"

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