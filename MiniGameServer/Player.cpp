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

// donghyun : �÷��̾� ����Ʈ ��� �� ���
std::string Player::getInfoStr()
{
	return std::format("�̿���: {}              ������ : {} : {}", m_name, m_ip, m_port);
}

void Player::decomposePacket(const char* packetChar)
{
	// donghyun : ��Ŷ ���� �� struct ����
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
			// donghyun : �ڱ� �ڽŹۿ� ���� �� �� ��
			ServerManager::getInstance().createRoom(m_fd, "5", "test");
		}
		else
		{
			// donghyun : �׽�Ʈ������ �� �濡 ���� ����
			ServerManager::getInstance().joinRoom(2, m_fd);
			// donghyun : �ش� ���� ���� �ֱ⸶�� Ÿ�̸� ������Ű�� �� ����Ʈ�� �߰�
			ServerManager::getInstance().addRoomTimerList(2);
			// donghyun : 2���� á�� �� ���� ���� ��Ŷ ��ε�ĳ����
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

		std::cout << "player position : " << m_position[0] << " : " << m_position[1] << " : " << m_position[2] << '\n';
		std::cout << "monster position : " << pmColliderRequestPacket.monsterPos[0] << " : " << pmColliderRequestPacket.monsterPos[1] << " : " << pmColliderRequestPacket.monsterPos[2] << '\n';

		float dirVec[3] = { 0.0f, 0.0f, 0.0f };
		bool IsCollided = checkCollide(pmColliderRequestPacket.monsterPos);
		if (IsCollided)
		{
			// donghyun : �� ���� direction vector ���
			for (int i = 0; i < 3; ++i)
			{
				dirVec[i] = m_position[i] - pmColliderRequestPacket.monsterPos[i];
			}
		}

		Packet::CollideResultPacket collideResultPacket(IsCollided, dirVec);
		NetworkManager::getInstance().sendPacket(m_fd, collideResultPacket, collideResultPacket.packetSize);
		break;
	}
	case Packet::PacketID::PPCOLLIDEREQUEST:
	{
		Packet::PPColliderRequestPacket ppColliderRequestPacket = *(Packet::PPColliderRequestPacket*)(packetChar);

		float dirVec[3] = { 0.0f, 0.0f, 0.0f };

		Player* oppoPlayer = ServerManager::getInstance().findPlayerUsingInfoMapIdx(ppColliderRequestPacket.oppoPlayerIdx);
		if (!oppoPlayer)
		{
			return;
		}

		bool IsCollided = checkCollide(oppoPlayer->m_position);
		if (IsCollided)
		{
			// donghyun : �� ���� direction vector ���
			for (int i = 0; i < 3; ++i)
			{
				dirVec[i] = m_position[i] - oppoPlayer->m_position[i];
			}
		}

		Packet::CollideResultPacket pCollideResultPacket(IsCollided, dirVec);
		NetworkManager::getInstance().sendPacket(m_fd, pCollideResultPacket, pCollideResultPacket.packetSize);

		for (int i = 0; i < 3; ++i)
		{
			dirVec[i] = -dirVec[i];
		}

		Packet::CollideResultPacket oCollideResultPacket(IsCollided, dirVec);
		NetworkManager::getInstance().sendPacket(oppoPlayer->m_fd, oCollideResultPacket, oCollideResultPacket.packetSize);

		break;
	}
	default:
	{
		break;
	}
	}
}

// donghyun : true�� �浹
bool Player::checkCollide(const float* oppoPosVec)
{
	float sqaureDist = 0.0f;
	for (int i = 0; i < 3; ++i)
	{
		sqaureDist += pow(m_position[i] - oppoPosVec[i], 2);
	}
	float testValue = pow((ServerProtocol::PLAYER_COLLIDER_RADIUS * 2.0f), 2);
	return testValue >= sqaureDist;
}

