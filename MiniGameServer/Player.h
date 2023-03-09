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
	// donghyun : -1�̸� ���� ���� �ʰ�, �� ���� ���� �ش� �� ��ȣ
	int m_roomNum = -1;

	int m_bufStartIdx = 0;
	char m_buf[PacketProtocol::BUF_MAXSIZE] = { 0 };

	// donghyun : player pos, rot
	float m_position[3] = { 0.0f };
	float m_rotation[3] = { 0.0f };

	// donghyun : �� �濡���� player idx
	unsigned short m_infoMapIdx = -1;

	//donghyun : �� ó�� Ŭ�� ���� ���� ��û �� �̸��� ���� �� ����ϴ� ������
	Player(char ip[], u_short port, SOCKET fd, std::string name);
	Player(SOCKET clntFd);
	Player();
	Player(const Player& player);
	std::string getInfoStr();
	void decomposePacket(const char* assmeblePacket);
	bool checkCollide(const float* oppoPosVec);
};