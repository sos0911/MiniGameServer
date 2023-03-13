#pragma once
#include <string>
#include <vector>
#include <chrono>
#include <iomanip>
#include <map>

class Player;

class Room
{
public:
	std::string openTime = "", roomName = "";
	// donghyun : key : �÷��̾� �̸�
	// donghyun : value : <player ����, ���� �ð�(string)>
	std::map<std::string, std::pair<Player*, std::string>> roomPartInfo;

	int maxPartCnt, curPartCnt, roomNum;
	std::chrono::steady_clock::time_point gameStartTime;
	// donghyun : ���� ������ ����� �ð� (�� ����)
	unsigned short curPlayTime;

	//// donghyun : �渶���� spawn thread
	//std::jthread m_timerThread;

	Room();
	Room(int in_maxPartCnt, Player& in_player);
	// donghyun : ��������ڴ� ����Ʈ�� �д� (map insert �� ����)
};