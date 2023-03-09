#pragma once
#include <map>
#include <vector>
#include <WinSock2.h>
#include "Singleton.h"
#include "Room.h"
#include "Player.h"
#include "Packet.h"
#include "BaseTaskManager.h"
#include "NetworkManager.h"

class ServerManager : public Singleton<ServerManager>, public BaseTaskManager
{
private:
	friend class Singleton;
	ServerManager();
	ServerManager(const ServerManager& ref) = delete;
	ServerManager& operator=(const ServerManager& ref) = delete;
	virtual ~ServerManager() {}

	// donghyun : key는 name이다.
	std::map<SOCKET, Player> playerList;
	// donghyun : key는 방 번호이다.
	std::map<int, Room> roomList;
	int lastRoomNum = 1;
	// donghyun : 타이머가 업데이트가 되야 할 방 번호들 (이미 게임 시작한 방들)
	std::set<int> updateRoomTimerList;

	std::jthread m_timerThread;

public:

	//// donghyun : 싱글톤 구현
	//static ServerManager& getInstance();

	//ServerManager();
	//virtual ~ServerManager() {}

	void login(SOCKET clntfd, std::vector<std::string>& splitStrList);
	void showHelp(const SOCKET clntfd);
	void showChatHelp(const SOCKET clntfd);
	void createRoom(const SOCKET clntfd, const std::string& maxCntStr, const std::string& roomName);
	void sendWhisper(std::vector<std::string>& splitStrList, const SOCKET clntfd);
	void showRoomInfo(int roomNum, const SOCKET clntfd);
	void showRoomList(const SOCKET clntfd);
	void showPlayerInfo(std::string& playerName, const SOCKET clntfd);
	void showPlayerList(const SOCKET clntfd);

	void joinRoom(const int roomNum, const SOCKET clntfd);

	int getLastRoomNum() { return ++lastRoomNum; }

	int getChatRoomNum(SOCKET clntfd);
	void broadCastChatInRoom(SOCKET clntfd, int roomNum, std::string& msg);
	void broadCastInRoom(int roomNum, std::string& msg);
	void broadCastPacketInRoom(const SOCKET clntfd, int roomNum, Packet::PacketID packetID);

	template< class PacketType >
	void broadCastPacketInRoom(int roomNum, PacketType& packet, Packet::PacketID packetID)
	{
		if (roomList.find(roomNum) == roomList.end())
		{
			return;
		}
		
		Room& room = roomList[roomNum];
		
		switch (packetID)
		{
		case Packet::PacketID::PPCOLLIDERESULT:
		{
			Packet::PPCollideResultPacket ppcollideResultPacket = *(Packet::PPCollideResultPacket*)( &packet );
			for (auto iter = room.roomPartInfo.begin(); iter != room.roomPartInfo.end(); ++iter)
			{
				auto playerInfo = iter->second.first;
				NetworkManager::getInstance().sendPacket(playerInfo->m_fd, ppcollideResultPacket, ppcollideResultPacket.packetSize);
			}
			break;
		}
		case Packet::PacketID::PMCOLLIDERESULT:
		{
			Packet::PMCollideResultPacket pmcollideResultPacket = *(Packet::PMCollideResultPacket*)(&packet);
			for (auto iter = room.roomPartInfo.begin(); iter != room.roomPartInfo.end(); ++iter)
			{
				auto playerInfo = iter->second.first;
				NetworkManager::getInstance().sendPacket(playerInfo->m_fd, pmcollideResultPacket, pmcollideResultPacket.packetSize);
			}
			break;
		}
		case Packet::PacketID::HEART:
		{
			Packet::HeartPacket heartPacket = *(Packet::HeartPacket*)(&packet);
			for (auto iter = room.roomPartInfo.begin(); iter != room.roomPartInfo.end(); ++iter)
			{
				auto playerInfo = iter->second.first;
				NetworkManager::getInstance().sendPacket(playerInfo->m_fd, heartPacket, heartPacket.packetSize);
			}
			break;
		}
		default:
		{
			break;
		}
		}
	}

	void quitPlayer(const SOCKET clntfd);
	void quitRoom(const int roomNum, Player* playerPtr);
	bool addPlayer(Player& player);
	int getPlayerNum();
	void addRoomTimerList(const int roomNum);

	// donghyun : threads methods
	void RunTimer();
	void UpdateRoomTimer();

	//// packet generate method
	//Packet::GameStartPacket& makeGameStartPacket();

	Player* findPlayerUsingfd(const SOCKET clntfd);
	Player* findPlayerUsingName(const std::string& playerName);
	Player* findPlayerUsingInfoMapIdx(const unsigned short infoMapIdx);

	std::string getCurTime();
};