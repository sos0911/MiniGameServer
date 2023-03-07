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
	int curPlayTime;

	Room();
	Room(const std::string& in_roomName, int in_maxPartCnt, Player& in_player);
	// donghyun : ��������ڴ� ����Ʈ�� �д� (map insert �� ����)
};