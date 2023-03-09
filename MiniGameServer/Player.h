#pragma once
#include <string>
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

	// donghyun : 각 방에서의 player idx
	unsigned short m_infoMapIdx = -1;

	//donghyun : 맨 처음 클라 소켓 연결 요청 시 이름이 없을 때 사용하는 생성자
	Player(char ip[], u_short port, SOCKET fd, std::string name);
	Player(SOCKET clntFd);
	Player();
	Player(const Player& player);
	std::string getInfoStr();
	void decomposePacket(const char* assmeblePacket);
	bool checkCollide(const float* oppoPosVec);
};