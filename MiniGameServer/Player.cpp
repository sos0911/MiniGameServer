#include "pch.h"

#include "Player.h"
#include <algorithm>
#include <iterator>
#include <iostream>
#include <format>
#include "ServerManager.h"
#include "NetworkManager.h"
#include "Packet.h"
#include <chrono>

Player::Player(char ip[], u_short port, SOCKET fd, std::string name) :
	m_ip(ip), m_port(port), m_fd(fd), m_name(name)
{
	m_roomPlayerIdx = 0;
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
	
	switch (packetID)
	{
	case Packet::PacketID::LOGINREQUEST:
	{
		ServerManager::getInstance().loginProcess(m_fd, packetChar);
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

		//std::cout << "player position : " << m_position[0] << " : " << m_position[1] << " : " << m_position[2] << '\n';
		//std::cout << "monster position : " << pmColliderRequestPacket.monsterPos[0] << " : " << pmColliderRequestPacket.monsterPos[1] << " : " << pmColliderRequestPacket.monsterPos[2] << '\n';

		float dirVec[3] = { 0.0f, 0.0f, 0.0f };
		bool IsCollided = checkCollide(pmColliderRequestPacket.monsterPos, Packet::PacketID::PMCOLLIDERESULT);

		if (IsCollided)
		{
			// donghyun : 힘 받을 direction vector 계산
			for (int i = 0; i < 2; ++i)
			{
				dirVec[i] = m_position[i] - pmColliderRequestPacket.monsterPos[i];
			}
		}

		Packet::PMCollideResultPacket pmcollideResultPacket(m_roomPlayerIdx, IsCollided, dirVec);
		//std::cout << "PMCollideResultPacket send!" << '\n';
		ServerManager::getInstance().broadCastPacketInRoom(m_roomNum, pmcollideResultPacket, Packet::PacketID::PMCOLLIDERESULT);

		// donghyun : 충돌 검출 시 hp 깎고 정보 브로드캐스팅
		if (IsCollided)
		{
			m_heartCnt--;
			Packet::HeartPacket heartPacket(m_roomPlayerIdx, m_heartCnt);
			ServerManager::getInstance().broadCastPacketInRoom(m_roomNum, heartPacket, Packet::PacketID::HEART);

			Room* room = ServerManager::getInstance().findRoomUsingRoomNum(m_roomNum);
			if (!room)
			{
				return;
			}

			// donghyun : HP가 0이 되었다면 해당 플레이어 생존 시간 표시
			if (0 == m_heartCnt)
			{
				// Get the end time
				auto endTime = std::chrono::high_resolution_clock::now();
				// Calculate the elapsed time in milliseconds
				auto elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - room->gameStartTime);
				m_surviveTime = elapsedTime.count();
				m_rank = room->lastRankNum;
				room->lastRankNum--;
			}

			// donghyun : 해당 방 안 플레이어들의 hp가 한 명을 제외하고 모두 0이 되어서 게임종료 조건을 충족하는지 검사
			unsigned short survivedPlayerCnt = 0;
			for (auto iter = room->roomPartInfo.begin(); iter != room->roomPartInfo.end(); ++iter)
			{
				Player* playerPtr = iter->second.first;
				if (playerPtr->m_heartCnt > 0)
				{
					survivedPlayerCnt++;
				}
			}

			// donghyun : 만약 게임 종료 성립이라면 gameend 패킷 브로드캐스팅
			if (1 == survivedPlayerCnt)
			{
				Player* firstRankPlayerPtr = nullptr;
				// donghyun : 1등 정보 입력
				for (auto iter = room->roomPartInfo.begin(); iter != room->roomPartInfo.end(); ++iter)
				{
					Player* playerPtr = iter->second.first;
					if (playerPtr->m_heartCnt > 0)
					{
						firstRankPlayerPtr = playerPtr;
						break;
					}
				}

				if (!firstRankPlayerPtr)
				{
					break;
				}

				// Get the end time
				auto endTime = std::chrono::high_resolution_clock::now();
				// Calculate the elapsed time in milliseconds
				auto elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - room->gameStartTime);
				firstRankPlayerPtr->m_surviveTime = elapsedTime.count();
				firstRankPlayerPtr->m_rank = room->lastRankNum;
				room->lastRankNum--;

				Packet::GameEndPacket gameEndPacket;
				// donghyun : 1등부터 채워 넣음
				unsigned short curRank = 1;
				for (int i = 0; i < ServerProtocol::ROOM_MAXPARTCNT; ++i)
				{
					for (auto iter = room->roomPartInfo.begin(); iter != room->roomPartInfo.end(); ++iter)
					{
						Player* playerPtr = iter->second.first;
						if (playerPtr->m_rank == curRank)
						{
							Packet::PlayerGameEndInfo playerGameEndInfo(playerPtr->m_roomPlayerIdx, playerPtr->m_rank, playerPtr->m_surviveTime);
							gameEndPacket.playerGameEndInfoArr[curRank - 1] = playerGameEndInfo;
							curRank++;
							break;
						}
					}
				}
				// donghyun : timer thread, spawn thread 중단
				ServerManager::getInstance().deleteRoomTimerList(m_roomNum);
				ServerManager::getInstance().stopSpawnThread(m_roomNum);

				ServerManager::getInstance().broadCastPacketInRoom(m_roomNum, gameEndPacket, Packet::PacketID::GAMEEND);
			}
		}

		break;
	}
	case Packet::PacketID::PPCOLLIDEREQUEST:
	{
		Packet::PPColliderRequestPacket ppColliderRequestPacket = *(Packet::PPColliderRequestPacket*)(packetChar);

		Player* oppoPlayer = ServerManager::getInstance().findPlayerUsingInfoMapIdx(ppColliderRequestPacket.oppoPlayerIdx);
		if (!oppoPlayer)
		{
			return;
		}

		//std::cout << "oppo pos : "<< oppoPlayer->m_position[0] << " : " << oppoPlayer->m_position[1] << oppoPlayer->m_position[2] << '\n';
		//std::cout << "my pos : " << oppoPlayer->m_position[0] << " : " << oppoPlayer->m_position[1] << oppoPlayer->m_position[2] << '\n';

		bool IsCollided = checkCollide(oppoPlayer->m_position, Packet::PacketID::PPCOLLIDERESULT);

		float dirVec[3] = { 0.0f, 0.0f, 0.0f };
		float oppoDirVec[3] = { 0.0f, 0.0f, 0.0f };

		if (IsCollided)
		{
			// donghyun : 힘 받을 direction vector 계산
			for (int i = 0; i < 2; ++i)
			{
				dirVec[i] = m_position[i] - oppoPlayer->m_position[i];
			}
		}

		for (int i = 0; i < 2; ++i)
		{
			oppoDirVec[i] = -dirVec[i];
		}

		Packet::PPCollideResultPacket pCollideResultPacket(IsCollided, dirVec);
		Packet::PPCollideResultPacket oCollideResultPacket(IsCollided, oppoDirVec);

		NetworkManager::getInstance().sendPacket(m_fd, pCollideResultPacket, pCollideResultPacket.packetSize);
		NetworkManager::getInstance().sendPacket(oppoPlayer->m_fd, oCollideResultPacket, oCollideResultPacket.packetSize);
		break;
	}
	default:
	{
		break;
	}
	}
}

// donghyun : true면 충돌
bool Player::checkCollide(const float* oppoPosVec, Packet::PacketID packetId)
{
	switch (packetId)
	{
	case Packet::PacketID::PMCOLLIDERESULT:
	{
		float closestX = m_position[0];
		float closestY = m_position[1];

		float rectLowerLeftX = oppoPosVec[0] - ServerProtocol::MONSTER_BOX_COLLIDER_WIDTH / 2.0f;
		float rectLowerLeftY = oppoPosVec[1] - ServerProtocol::MONSTER_BOX_COLLIDER_HEIGHT / 2.0f;

		if (m_position[0] < rectLowerLeftX) {
			closestX = rectLowerLeftX;
		}
		else if (m_position[0] > rectLowerLeftX + ServerProtocol::MONSTER_BOX_COLLIDER_WIDTH) {
			closestX = rectLowerLeftX + ServerProtocol::MONSTER_BOX_COLLIDER_WIDTH;
		}

		if (m_position[1] < rectLowerLeftY) {
			closestY = rectLowerLeftY;
		}
		else if (m_position[1] > rectLowerLeftY + ServerProtocol::MONSTER_BOX_COLLIDER_HEIGHT) {
			closestY = rectLowerLeftY + ServerProtocol::MONSTER_BOX_COLLIDER_HEIGHT;
		}
		return pow(m_position[0] - closestX, 2) + pow(m_position[1] - closestY, 2) < pow(ServerProtocol::PLAYER_COLLIDER_RADIUS, 2);
	}
	case Packet::PacketID::PPCOLLIDERESULT:
	{
		float sqaureDist = 0.0f;
		for (int i = 0; i < 3; ++i)
		{
			sqaureDist += pow(m_position[i] - oppoPosVec[i], 2);
		}
		float testValue = pow((ServerProtocol::PLAYER_COLLIDER_RADIUS * 2.0f), 2);
		return testValue >= sqaureDist;
	}
	default:
	{
		// donghyun : error -> return false
		return false;
	}
	}
}

