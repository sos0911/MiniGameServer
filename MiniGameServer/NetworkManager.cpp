#include "pch.h"

#include "NetworkManager.h"
#include <sstream>
#include <vector>
#include <iostream>
#include <algorithm>
#include <format>
#include <WS2tcpip.h>
#include "Packet.h"

NetworkManager::NetworkManager()
{
	hServsock = 0;
	hClntSock = 0;
	servAdr = { 0 };
	clntAdr = { 0 };
	timeout = { 0 };
	reads = { 0 };
}

//void NetworkManager::init(int in_argc, char* in_argv[])
//{
//	argc = in_argc;
//	argv = in_argv;
//}

void NetworkManager::execute()
{
	/*if (argc != 2)
	{
		printf("usage : %s <port>\n", argv[0]);
		exit(1);
	}*/
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

	std::cout << "���� �غ� �Ϸ�!" << '\n';

	FD_ZERO(&reads);
	FD_SET(hServsock, &reads);

	fd_set cpyReads;

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
			// donghyun : ���� ���� ��û�ϴ� Ŭ�� ������ ���
			if (reads.fd_array[i] == hServsock)
			{
				adrSize = sizeof(clntAdr);
				hClntSock = accept(hServsock, (SOCKADDR*)&clntAdr, &adrSize);

				FD_SET(hClntSock, &reads);

				char clntIP[20] = { 0 };
				inet_ntop(AF_INET, &clntAdr.sin_addr,
					clntIP, sizeof(clntIP)), ntohs(clntAdr.sin_port);
				Player player(clntIP, ntohs(clntAdr.sin_port), hClntSock, "");

				if (!ServerManager::getInstance().addPlayer(player))
				{
					//sendMsg(reads.fd_array[i], "�ʱ� ���� ����! �÷��̾� fd ��ħ.\n\r");
					continue;
				}

				printf("connected client : %lu\n", static_cast<unsigned long>(hClntSock));
				printf("client ip : %s, client port : %d\n", inet_ntop(AF_INET, &clntAdr.sin_addr,
					clntIP, sizeof(clntIP)), ntohs(clntAdr.sin_port));

				//sendMsg(hClntSock, "���ӵǾ����ϴ�. login [�г���] ���·� �α��� �ٶ��ϴ�.\n\r");
			}
			// donghyun : �̹� ����� Ŭ�� ���Ͽ��Լ� �����͸� �޴� ���
			else
			{
				Player* playerPtr = ServerManager::getInstance().findPlayerUsingfd(reads.fd_array[i]);
				if (nullptr == playerPtr)
				{
					continue;
				}

				int strLen = recv(playerPtr->m_fd, playerPtr->m_buf + playerPtr->m_bufStartIdx, BUF_SIZE - 1, 0);

				if (strLen == 0)
				{
					ServerManager::getInstance().quitPlayer(playerPtr->m_fd);
					closesocket(playerPtr->m_fd);
				}
				// donghyun : ���� �� ���� �ִ� ���
				else
				{
					LoginPacket loginPacket(true);
					NetworkManager::getInstance().sendPacket(playerPtr->m_fd, loginPacket, loginPacket.packetSize);

					playerPtr->m_bufStartIdx += strLen;

					// donghyun : ���� �տ� �ִ� ��Ŷ������ ������
					unsigned short packetSize = *(unsigned short*)playerPtr->m_buf;

					if (playerPtr->m_bufStartIdx >= packetSize)
					{
						playerPtr->m_buf[playerPtr->m_bufStartIdx] = '\0';
						// donghyun : ��Ŷ ������ char[]�� ����, ó�� �ѱ�
						char packetChar[PacketProtocol::MAXPACKETSIZE];
						memcpy_s(&packetChar, PacketProtocol::MAXPACKETSIZE, playerPtr->m_buf, packetSize);
						memcpy_s(&playerPtr->m_buf, PacketProtocol::MAXPACKETSIZE, playerPtr->m_buf + packetSize, playerPtr->m_bufStartIdx - packetSize);
						playerPtr->decomposePacket(packetChar);
						playerPtr->m_bufStartIdx -= packetSize;
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

// donghyun : ���� ��ũ���ͷ� send
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
