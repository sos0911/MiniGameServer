#pragma once
#include <string>
#include <chrono>
#include "Packet.h"

class Player
{
public:
	std::string m_ip = "";
	u_short m_port;
	SOCKET m_fd;
	std::string m_name;
	// donghyun : -1이면 속해 있지 않고, 그 외의 경우는 해당 방 번호
	int m_roomNum = -1;

	int m_bufStartIdx = 0;
	char m_buf[PacketProtocol::BUF_MAXSIZE] = { 0 };

	// donghyun : player pos, rot
	float m_position[3] = { 0.0f };
	float m_rotation[3] = { 0.0f };

	// donghyun : 각 방에서의 player idx. 0이면 방에 들어가지 않은 상태
	unsigned short m_roomPlayerIdx = 0;
	// donghyun : hp 상태
	unsigned short m_heartCnt = 3;
	// donghyun : 게임 등수
	unsigned short m_rank = 0;
	// donghyun : 생존 시간
	long long m_surviveTime = 0;
	// donghyun : 가장 최근에 몬스터랑 충돌한 시각
	std::chrono::steady_clock::time_point m_latestCollisionTime;

	//donghyun : 맨 처음 클라 소켓 연결 요청 시 이름이 없을 때 사용하는 생성자
	Player(char ip[], u_short port, SOCKET fd, std::string name);
	Player(SOCKET clntFd);
	Player();
	Player(const Player& player);
	std::string getInfoStr();
	void decomposePacket(const char* assmeblePacket);
	bool checkCollide(const float* oppoPosVec, Packet::PacketID packetId);
};