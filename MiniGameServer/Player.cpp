#include "pch.h"

#include "Player.h"
#include <algorithm>
#include <iterator>
#include <iostream>
#include <format>
#include "ServerManager.h"
#include "Packet.h"

Player::Player(char ip[], u_short port, SOCKET fd, std::string name) :
	m_ip(ip), m_port(port), m_fd(fd), m_name(name)
{
}

Player::Player(SOCKET clntFd) :
	m_ip(""), m_port(0), m_fd(clntFd), m_name(""), m_roomNum(-1) {}

Player::Player() :
	m_ip(""), m_port(0), m_fd(0), m_name(""), m_roomNum(-1) {}

Player::Player(const Player& player)
{
	this->m_ip = player.m_ip;
	this->m_port = player.m_port;
	this->m_fd = player.m_fd;
	this->m_name = player.m_name;
	this->m_roomNum = player.m_roomNum;
}

// donghyun : �÷��̾� ����Ʈ ��� �� ���
std::string Player::getInfoStr()
{
	return std::format("�̿���: {}              ������ : {} : {}", m_name, m_ip, m_port);
}

void Player::decomposePacket(const char packetChar[])
{
	// donghyun : ��Ŷ ���� �� struct ����
	PacketID packetID;
	std::memcpy(&packetID, &packetChar[1], sizeof(PacketID));
	
	switch (packetID)
	{
	case PacketID::LOGIN:
	{
		std::string loginID = "";
		std::memcpy(&loginID, &packetChar[2], packetChar[0] - sizeof(packetChar[0]) - sizeof(PacketID));
		Player* playerPtr = ServerManager::getInstance().findPlayerUsingfd(m_fd);
		if (!playerPtr)
		{
			LoginPacket loginPacket(false);
			NetworkManager::getInstance().sendPacket(m_fd, loginPacket, loginPacket.packetSize);
			return;
		}
		if (ServerManager::getInstance().findPlayerUsingName(loginID))
		{
			LoginPacket loginPacket(false);
			NetworkManager::getInstance().sendPacket(m_fd, loginPacket, loginPacket.packetSize);
			return;
		}
		playerPtr->m_name = loginID;
		LoginPacket loginPacket(true);
		NetworkManager::getInstance().sendPacket(m_fd, loginPacket, loginPacket.packetSize);
		break;
	}
	case PacketID::JOINROOM:
	{
		break;
	}
	case PacketID::PLAY:
	{
		break;
	}
	case PacketID::SKILL:
	{
		break;
	}
	}
}

