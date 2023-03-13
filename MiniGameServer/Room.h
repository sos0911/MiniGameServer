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
	unsigned short curPlayTime;

	// donghyun : 게임이 시작된 시간
	std::chrono::steady_clock::time_point gameStartTime;
	// donghyun : 게임의 현재 마지막 등수
	unsigned short lastRankNum = ServerProtocol::ROOM_MAXPARTCNT;

	Room();
	Room(int in_maxPartCnt, Player& in_player);
	// donghyun : 복사생성자는 디폴트로 둔다 (map insert 시 사용됨)
};