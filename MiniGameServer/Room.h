#pragma once
#include <string>
#include <vector>
#include <chrono>
#include <iomanip>
#include <map>
#include <chrono>
#include "Packet.h"

class Player;

class Room
{
public:
	std::string openTime = "", roomName = "";
	// donghyun : key : 플레이어 이름
	// donghyun : value : <player 정보, 참여 시각(string)>
	std::map<std::string, std::pair<Player*, std::string>> roomPartInfo;

	int maxPartCnt, curPartCnt, roomNum;
	// donghyun : 현재 게임이 진행된 시간 (초 단위)
	short curPlayTime;

	// donghyun : 게임이 시작된 시간
	std::chrono::steady_clock::time_point gameStartTime;
	// donghyun : 게임의 현재 마지막 등수
	unsigned short lastRankNum = ServerProtocol::ROOM_MAXPARTCNT;
	// donghyun : spawn packet 중복 방지를 위한 타임맵
	std::chrono::steady_clock::time_point latestSpawnInfoArr[2][ServerProtocol::GAMEMAP_SIZE + 1][2];
	// donghyun : 게임이 시작되었는가?
	bool isGameStart = false;

	Room();
	Room(int in_maxPartCnt, Player& in_player);
	// donghyun : 복사생성자는 디폴트로 둔다 (map insert 시 사용됨)
};