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

struct JthreadLess {
	bool operator()(const std::jthread& lhs, const std::jthread& rhs) const {
		return lhs.get_id() < rhs.get_id();
	}
};

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
	// donghyun : 아직 게임을 시작하지 않은 애들 수
	int startWaitingPlayerNum = 0;
	// donghyun : 타이머가 업데이트가 되야 할 방 번호들 (이미 게임 시작한 방들)
	std::set<int> updateRoomTimerList;

	// donghyun : thread & lock mutex
	std::jthread m_timerThread;
	std::set< std::jthread, JthreadLess> m_spawnThreadSet;
	std::mutex m_mutex;

public:

	//// donghyun : 싱글톤 구현
	//static ServerManager& getInstance();

	//ServerManager();
	//virtual ~ServerManager() {}

	void login(SOCKET clntfd, std::vector<std::string>& splitStrList);
	void loginProcess(const SOCKET clntfd, const char* packetChar);
	void showHelp(const SOCKET clntfd);
	void showChatHelp(const SOCKET clntfd);
	void createRoom(const SOCKET clntfd, const unsigned short maxCnt);
	void sendWhisper(std::vector<std::string>& splitStrList, const SOCKET clntfd);
	void showRoomInfo(int roomNum, const SOCKET clntfd);
	void showRoomList(const SOCKET clntfd);
	void showPlayerInfo(std::string& playerName, const SOCKET clntfd);
	void showPlayerList(const SOCKET clntfd);

	bool joinRoom(const int roomNum, const SOCKET clntfd);

	// donghyun : get, set 관련 함수들
	int getLastRoomNum() { return lastRoomNum; }
	int getStartWaitingPlayerNum() { return startWaitingPlayerNum; }
	void increaseLastRoomNum() { ++lastRoomNum; }
	void increaseStartWaitingPlayerNum() { ++startWaitingPlayerNum; }

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
	int getLoginedPlayerNum();
	int getNoRoomPlayerNum();
	void addRoomTimerList(const int roomNum);

	// donghyun : threads methods
	void RunTimer();
	void UpdateRoomTimer();
	void RunSpawner(const int roomNum);

	//// packet generate method
	//Packet::GameStartPacket& makeGameStartPacket();

	Player* findPlayerUsingfd(const SOCKET clntfd);
	Player* findPlayerUsingName(const std::string& playerName);
	Player* findPlayerUsingInfoMapIdx(const unsigned short infoMapIdx);

	std::string getCurTime();
};