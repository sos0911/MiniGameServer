#include "pch.h"

#include "NetworkManager.h"
#include "ServerManager.h"
#include <sstream>
#include <iostream>
#include <algorithm>
#include <format>
#include <WS2tcpip.h>
#include "Packet.h"
#include "Player.h"

#include <string>
#include <vector>

NetworkManager::NetworkManager()
{
	hServsock = 0;
	hClntSock = 0;
	servAdr = { 0 };
	clntAdr = { 0 };
	timeout = { 0 };
	reads = { 0 };
}

void NetworkManager::execute()
{
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		ErrorHandling("WSAstartup() error");
	}

	hServsock = socket(PF_INET, SOCK_STREAM, 0);
	if (hServsock == INVALID_SOCKET)
	{
		ErrorHandling("socket() error");
	}

	// donghyun : nagle off
	int socketOption = 1;
	int returnValue = setsockopt(hServsock, SOL_SOCKET, TCP_NODELAY, reinterpret_cast<const char*>(&socketOption), sizeof(socketOption));
	if (returnValue != 0)
	{
		return;
	}

	memset(&servAdr, 0, sizeof(servAdr));
	servAdr.sin_family = AF_INET;
	servAdr.sin_addr.s_addr = htonl(INADDR_ANY);
	servAdr.sin_port = htons(8080);
	//servAdr.sin_port = htons(static_cast<short>(atoi(argv[1])));

	if (bind(hServsock, (sockaddr*)&servAdr, sizeof(servAdr)) == SOCKET_ERROR)
	{
		ErrorHandling("bind() error");
	}
	if (listen(hServsock, 5) == SOCKET_ERROR)
	{
		ErrorHandling("listen() error");
	}

	std::cout << "서버 준비 완료!" << '\n';

	FD_ZERO(&reads);
	FD_SET(hServsock, &reads);

	fd_set cpyReads;

	// donghyun : servermanager 타이머 쓰레드 실행
	ServerManager::getInstance().RunTimer();

	while (1)
	{
		cpyReads = reads;
		timeout.tv_sec = 5;
		timeout.tv_usec = 5000;

		if ((fdNum = select(0, &cpyReads, 0, 0, &timeout)) == SOCKET_ERROR)
		{
			break;
		}
		if (fdNum == 0)
		{
			continue;
		}

		for (u_int i = 0; i < reads.fd_count; ++i)
		{
			if (!FD_ISSET(reads.fd_array[i], &cpyReads))
			{
				continue;
			}
			// donghyun : 새로 연결 요청하는 클라 소켓의 경우
			if (reads.fd_array[i] == hServsock)
			{
				adrSize = sizeof(clntAdr);
				hClntSock = accept(hServsock, (SOCKADDR*)&clntAdr, &adrSize);

				// donghyun : nagle algorithm off
				int socketOption = 1;
				int returnValue = setsockopt(hClntSock, SOL_SOCKET, TCP_NODELAY, reinterpret_cast<const char*>(&socketOption), sizeof(socketOption));
				if (returnValue != 0)
				{
					return;
				}

				FD_SET(hClntSock, &reads);

				char clntIP[20] = { 0 };
				inet_ntop(AF_INET, &clntAdr.sin_addr,
					clntIP, sizeof(clntIP)), ntohs(clntAdr.sin_port);
				Player player(clntIP, ntohs(clntAdr.sin_port), hClntSock, "");

				if (!ServerManager::getInstance().addPlayer(player))
				{
					//sendMsg(reads.fd_array[i], "초기 접속 실패! 플레이어 fd 겹침.\n\r");
					continue;
				}

				printf("connected client : %lu\n", static_cast<unsigned long>(hClntSock));
				printf("client ip : %s, client port : %d\n", inet_ntop(AF_INET, &clntAdr.sin_addr,
					clntIP, sizeof(clntIP)), ntohs(clntAdr.sin_port));

				//sendMsg(hClntSock, "접속되었습니다. login [닉네임] 형태로 로그인 바랍니다.\n\r");
			}
			// donghyun : 이미 연결된 클라 소켓에게서 데이터를 받는 경우
			else
			{
				Player* playerPtr = ServerManager::getInstance().findPlayerUsingfd(reads.fd_array[i]);
				if (nullptr == playerPtr)
				{
					continue;
				}

				if ( playerPtr->m_bufStartIdx >= PacketProtocol::BUF_MAXSIZE )
				{
					volatile int hello = 0;
				}

				const int strLen = recv(playerPtr->m_fd, playerPtr->m_buf + playerPtr->m_bufStartIdx, PacketProtocol::BUF_MAXSIZE - playerPtr->m_bufStartIdx, 0);

				if (strLen == 0)
				{
					ServerManager::getInstance().quitPlayer(playerPtr->m_fd);
					closesocket(playerPtr->m_fd);
				}
				// donghyun : 받은 게 뭐라도 있는 경우
				else
				{
					playerPtr->m_bufStartIdx += strLen;

					while(1)
					{
						if (playerPtr->m_bufStartIdx < sizeof(unsigned short))
						{
							break;
						}

						const unsigned short packetSize = *(unsigned short*)(playerPtr->m_buf);

						if (playerPtr->m_bufStartIdx >= packetSize)
						{
							playerPtr->m_buf[playerPtr->m_bufStartIdx] = '\0';
							// donghyun : 패킷 내용을 char[]에 복사, 처리 넘김
							char packetChar[PacketProtocol::PACKET_MAXSIZE];
							memcpy_s(&packetChar, PacketProtocol::PACKET_MAXSIZE, playerPtr->m_buf, packetSize);
							memcpy_s(&playerPtr->m_buf, PacketProtocol::PACKET_MAXSIZE, playerPtr->m_buf + packetSize, playerPtr->m_bufStartIdx - packetSize);
							playerPtr->decomposePacket(packetChar);
							playerPtr->m_bufStartIdx -= packetSize;

							/*static std::vector< std::pair< Packet::PacketID, PacketSizeType > > packetSizeTypeDebugCont;
							Packet::PacketID packetID = *(Packet::PacketID*)(packetChar + sizeof(unsigned short));
							packetSizeTypeDebugCont.push_back({ packetID, packetSize });*/
						}
						else
						{
							break;
						}
					}

				}
			}
		}
	}
	closesocket(hServsock);
	WSACleanup();
}

void NetworkManager::ErrorHandling(const char* message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}

void NetworkManager::sendMsg(const std::string playerName, const std::string& msg)
{
	Player* playerPtr = ServerManager::getInstance().findPlayerUsingName(playerName);
	if (playerPtr)
	{
		send(playerPtr->m_fd, msg.c_str(), static_cast<int>(msg.size()), 0);
	}
}

// donghyun : 파일 디스크럽터로 send
void NetworkManager::sendMsg(const SOCKET clntFd, const std::string& msg)
{
	send(clntFd, msg.c_str(), static_cast<int>(msg.size()), 0);
}

void NetworkManager::closeClient(const SOCKET clntfd)
{
	FD_CLR(clntfd, &reads);
	printf("closed client fd: %lu\n", static_cast<unsigned long>(clntfd));
	closesocket(clntfd);
}
