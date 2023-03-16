#include "pch.h"

#include "Room.h"
#include <chrono>
#include <ctime>

#include <chrono>
#include <iostream>

#include "ServerManager.h"

Room::Room()
{
	maxPartCnt = -1;
	curPartCnt = 0;
	roomNum = -1;
	curPlayTime = ServerProtocol::ROOM_TIMER_STARTTIME;

	std::fill(&latestSpawnInfoArr[0][0][0], &latestSpawnInfoArr[0][0][0] + sizeof(latestSpawnInfoArr) / sizeof(std::chrono::steady_clock::time_point), std::chrono::high_resolution_clock::now());
	isGameStart = false;
}

Room::Room(int in_maxPartCnt, Player& in_player)
{
	// donghyun : ���� �ð� ĳ��
	std::string time_str = ServerManager::getInstance().getCurTime();
	openTime = time_str;
	roomName = "room_" + std::to_string(ServerManager::getInstance().getLastRoomNum());
	
	roomPartInfo[in_player.m_name] = { &in_player, time_str };
	
	maxPartCnt = in_maxPartCnt;
	// donghyun : �ڱ� �ڽ�
	curPartCnt = 1;
	ServerManager::getInstance().increaseLastRoomNum();
	roomNum = ServerManager::getInstance().getLastRoomNum();

	curPlayTime = ServerProtocol::ROOM_TIMER_STARTTIME;

	std::fill(&latestSpawnInfoArr[0][0][0], &latestSpawnInfoArr[0][0][0] + sizeof(latestSpawnInfoArr) / sizeof(std::chrono::steady_clock::time_point), std::chrono::high_resolution_clock::now());
	isGameStart = false;
}
