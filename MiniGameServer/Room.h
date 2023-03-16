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
	// donghyun : key : �÷��̾� �̸�
	// donghyun : value : <player ����, ���� �ð�(string)>
	std::map<std::string, std::pair<Player*, std::string>> roomPartInfo;

	int maxPartCnt, curPartCnt, roomNum;
	// donghyun : ���� ������ ����� �ð� (�� ����)
	short curPlayTime;

	// donghyun : ������ ���۵� �ð�
	std::chrono::steady_clock::time_point gameStartTime;
	// donghyun : ������ ���� ������ ���
	unsigned short lastRankNum = ServerProtocol::ROOM_MAXPARTCNT;
	// donghyun : spawn packet �ߺ� ������ ���� Ÿ�Ӹ�
	std::chrono::steady_clock::time_point latestSpawnInfoArr[2][ServerProtocol::GAMEMAP_SIZE + 1][2];
	// donghyun : ������ ���۵Ǿ��°�?
	bool isGameStart = false;

	Room();
	Room(int in_maxPartCnt, Player& in_player);
	// donghyun : ��������ڴ� ����Ʈ�� �д� (map insert �� ����)
};