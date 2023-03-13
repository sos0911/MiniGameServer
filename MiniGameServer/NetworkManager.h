#pragma once
#include <WinSock2.h>
#include <string>
#include <map>
#include "Singleton.h"

//#define _WINSOCK_DEPRECATED_NO_WARNINGS

class NetworkManager : public Singleton<NetworkManager>
{
private:
	friend class Singleton;

	SOCKET hServsock, hClntSock;
	SOCKADDR_IN servAdr, clntAdr;
	timeval timeout;
	fd_set reads;

	NetworkManager(const NetworkManager& ref) = delete;
	NetworkManager& operator=(const NetworkManager& ref) = delete;
	NetworkManager();
	virtual ~NetworkManager() {}

	int adrSize = 0;
	int strLen = 0, fdNum = 0;

public:
	void execute();
	void ErrorHandling(const char* message);
	void sendMsg(const std::string playerName, const std::string& msg);
	void sendMsg(const SOCKET clntFd, const std::string& msg);
	void closeClient(const SOCKET clntfd);

	template< class PacketType >
	void sendPacket(const SOCKET clntFd, const PacketType& packet, const unsigned short packetSize)
	{
		const char* packetStr = reinterpret_cast<const char*>(&packet);
		send(clntFd, packetStr, packetSize, 0);
	}
};
