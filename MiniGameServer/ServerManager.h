#pragma once
#include <map>
#include <vector>
#include <WinSock2.h>
#include "Singleton.h"
#include "Room.h"
#include "Player.h"
#include "Packet.h"
#include "BaseTaskManager.h"

class ServerManager : public Singleton<ServerManager>, public BaseTaskManager
{
private:
	friend class Singleton;
	ServerManager();
	ServerManager(const ServerManager& ref) = delete;
	ServerManager& operator=(const ServerManager& ref) = delete;
	virtual ~ServerManager() {}

	// donghyun : key�� name�̴�.
	std::map<SOCKET, Player> playerList;
	// donghyun : key�� �� ��ȣ�̴�.
	std::map<int, Room> roomList;
	int lastRoomNum = 1;
	// donghyun : Ÿ�̸Ӱ� ������Ʈ�� �Ǿ� �� �� ��ȣ�� (�̹� ���� ������ ���)
	std::set<int> updateRoomTimerList;

	std::jthread m_timerThread;

public:

	//// donghyun : �̱��� ����
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