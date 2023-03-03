#pragma once
#include "Singleton.h"

class PlayerManager : public Singleton<PlayerManager>
{
private:
	friend class Singleton;
	PlayerManager();
	PlayerManager(const PlayerManager& ref) = delete;
	PlayerManager& operator=(const PlayerManager& ref) = delete;
	virtual ~PlayerManager() {}

public:


};