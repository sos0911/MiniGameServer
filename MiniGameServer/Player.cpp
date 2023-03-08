#include "pch.h"

#include "Player.h"
#include <algorithm>
#include <iterator>
#include <iostream>
#include <format>
#include "ServerManager.h"
#include "NetworkManager.h"
#include "Packet.h"

Player::Player(char ip[], u_short port, SOCKET fd, std::string name) :
	m_ip(ip), m_port(port), m_fd(fd), m_name(name)
{
	m_infoMapIdx = ServerManager::getInstance().getPlayerNum() + 1;
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

// donghyun : 플레이어 리스트 출력 시 사용
std::string Player::getInfoStr()
{
	return std::format("이용자: {}              접속지 : {} : {}", m_name, m_ip, m_port);
}

void Player::decomposePacket(const char* packetChar)
{
	// donghyun : 패킷 분해 후 struct 조립
	unsigned short packetSize = *(unsigned short*)packetChar;
	Packet::PacketID packetID = *(Packet::PacketID*)(packetChar + sizeof(unsigned short));
	//std::memcpy(&packetID, &packetChar[1], sizeof(Packet::PacketID));
	
	switch (packetID)
	{
	case Packet::PacketID::LOGINREQUEST:
	{
		Packet::LoginRequestPacket loginRequestPacket = *(Packet::LoginRequestPacket*)(packetChar);

		Player* playerPtr = ServerManager::getInstance().findPlayerUsingfd(m_fd);
		if (!playerPtr)
		{
			Packet::LoginResultPacket loginPacket(false);
			NetworkManager::getInstance().sendPacket(m_fd, loginPacket, loginPacket.packetSize);
			return;
		}
		if (ServerManager::getInstance().findPlayerUsingName(loginRequestPacket.LoginNickname))
		{
			Packet::LoginResultPacket loginPacket(false);
			NetworkManager::getInstance().sendPacket(m_fd, loginPacket, loginPacket.packetSize);
			return;
		}
		playerPtr->m_name = loginRequestPacket.LoginNickname;

		// donghyun : test code
		if (ServerManager::getInstance().getPlayerNum() == 1)
		{
			// donghyun : 자기 자신밖에 없을 시 방 팜
			ServerManager::getInstance().createRoom(m_fd, "5", "test");
		}
		else
		{
			// donghyun : 테스트용으로 한 방에 집어 넣음
			ServerManager::getInstance().joinRoom(2, m_fd);
			// donghyun : 해당 방을 일정 주기마다 타이머 증가시키는 방 리스트에 추가
			ServerManager::getInstance().addRoomTimerList(2);
			// donghyun : 2명이 찼을 때 게임 시작 패킷 브로드캐스팅
			ServerManager::getInstance().broadCastPacketInRoom(m_fd, 2, Packet::PacketID::GAMESTART);
		}

		Packet::LoginResultPacket loginPacket(true);
		NetworkManager::getInstance().sendPacket(m_fd, loginPacket, loginPacket.packetSize);
		break;
	}
	case Packet::PacketID::MAKEROOMREQUEST:
	{
		break;
	}
	case Packet::PacketID::JOINROOMREQUEST:
	{
		break;
	}
	case Packet::PacketID::UPDATE:
	{
		/*struct UpdatePacket
		{
			unsigned short packetSize;
			PacketID packetID;
			unsigned short playerIdx;
			float posVec[3];
			float rotVec[3];
		};*/

		Packet::UpdatePacket updatePacket = *(Packet::UpdatePacket*)(packetChar);
		Player* playerPtr = ServerManager::getInstance().findPlayerUsingfd(m_fd);
		if (!playerPtr)
		{
			return;
		}
		
		// donghyun : pos, rot update
		for (int i = 0; i < 3; i++)
		{
			playerPtr->m_position[i] = updatePacket.posVec[i];
			playerPtr->m_rotation[i] = updatePacket.rotVec[i];
		}

		int roomNum = ServerManager::getInstance().getChatRoomNum(m_fd);
		if(roomNum == -2)
		{
			return;
		}
	
		ServerManager::getInstance().broadCastPacketInRoom(playerPtr->m_fd, roomNum, Packet::PacketID::PLAY);
		break;
	}
	case Packet::PacketID::PMCOLLIDEREQUEST:
	{
		Packet::PMColliderRequestPacket pmColliderRequestPacket = *(Packet::PMColliderRequestPacket*)(packetChar);

		float dirVec[3] = { 0.0f, 0.0f, 0.0f };
		bool IsCollided = checkCollide(pmColliderRequestPacket.monsterPos);
		if (IsCollided)
		{
			// donghyun : 힘 받을 direction vector 계산
			for (int i = 0; i < 3; ++i)
			{
				dirVec[i] = m_position[i] - pmColliderRequestPacket.monsterPos[i];
			}
		}

		Packet::PMCollideResultPacket pmCollideResultPacket(IsCollided, dirVec);
		NetworkManager::getInstance().sendPacket(m_fd, pmCollideResultPacket, pmCollideResultPacket.packetSize);
		break;
	}
	default:
	{
		break;
	}
	}
}

// donghyun : true면 충돌
bool Player::checkCollide(const float* oppoVec)
{
	float sqaureDist = 0.0f;
	for (int i = 0; i < 3; ++i)
	{
		sqaureDist += pow(m_position[i] - oppoVec[i], 2);
	}
	return pow((ServerProtocol::PLAYER_COLLIDER_RADIUS * 2.0f), 2) >= sqaureDist;
}

