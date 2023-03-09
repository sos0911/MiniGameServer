﻿#include "pch.h"

#include "ServerManager.h"
#include "NetworkManager.h"
#include <iostream>
#include <format>
#include <ctime>

ServerManager::ServerManager()
{
}

void ServerManager::login(SOCKET clntfd, std::vector<std::string>& splitStrList)
{
	// donghyun : 이름 재조립
	int vecSize = static_cast<int>(splitStrList.size());
	std::string playerName = "";
	for (int i = 1; i < vecSize - 1; i++)
	{
		playerName += splitStrList[i] + " ";
	}
	playerName += splitStrList[vecSize - 1];

	// donghyun : 이름 중복 검사
	if (nullptr == findPlayerUsingName(playerName))
	{
		Player* playerPtr = findPlayerUsingfd(clntfd);
		if (playerPtr)
		{
			playerPtr->m_name = playerName;
			//NetworkManager::getInstance().sendMsg(clntfd, std::format("** 로그인 하였습니다. {}\n\r", playerPtr->m_name));
			showHelp(clntfd);
		}
	}
	else
	{
		//NetworkManager::getInstance().sendMsg(clntfd, "해당 이름은 사용할 수 없습니다.\n\r");
	}
}

// donghyun : 도움말 호출
void ServerManager::showHelp(const SOCKET clntfd)
{
	std::string msg = "";
	msg.reserve(100);
	msg = "-------------------------------------------------------------- -\n\r\
H                         명령어 안내\n\r\
US                        이용자 목록 보기\n\r\
LT                        대화방 목록 보기\n\r\
ST [방번호]               대화방 정보 보기\n\r\
PF [상대방ID]             이용자 정보 보기\n\r\
TO [상대방ID] [메시지]    쪽지 보내기\n\r\
O  [최대인원] [방제목]    대화방 만들기\n\r\
J  [방번호]               대화방 참여하기\n\r\
X                         끝내기\n\r\
-------------------------------------------------------------- -\n\r";

	//NetworkManager::getInstance().sendMsg(clntfd, msg);
}

void ServerManager::showChatHelp(const SOCKET clntfd)
{
	std::string msg = "";
	msg.reserve(100);
	msg = "-------------------------------------------------------------- -\n\r\
/H                         명령어 안내\n\r\
/US                        이용자 목록 보기\n\r\
/LT                        대화방 목록 보기\n\r\
/ST [방번호]               대화방 정보 보기\n\r\
/PF [상대방ID]             이용자 정보 보기\n\r\
/TO [상대방ID] [메시지]    쪽지 보내기\n\r\
/Q                         대화방 나가기\n\r\
/X                         끝내기\n\r\
-------------------------------------------------------------- -\n\r";

	//NetworkManager::getInstance().sendMsg(clntfd, msg);
}

void ServerManager::createRoom(const SOCKET clntfd, const std::string& maxCntStr, const std::string& roomName)
{
	Player* playerPtr = findPlayerUsingfd(clntfd);
	if (playerPtr)
	{
		Room room(roomName, std::stoi(maxCntStr), *playerPtr);
		roomList[room.roomNum] = room;
		// donghyun : 방장에게도 속한 방이 있다고 표시해주기
		playerPtr->m_roomNum = room.roomNum;
		playerPtr->m_infoMapIdx = 1;

		//std::string msg = std::format("** 대화방이 개설되었습니다.\n\r** {}님이 들어오셨습니다. (현재인원 {} / {})\n\r", playerPtr->m_name, room.curPartCnt, room.maxPartCnt);
		//NetworkManager::getInstance().sendMsg(clntfd, msg);
	}
}

void ServerManager::sendWhisper(std::vector<std::string>& splitStrList, const SOCKET clntfd)
{
	std::string msg = "";
	msg.reserve(100);

	Player* sendPlayerPtr = findPlayerUsingName(splitStrList[1]);
	if (sendPlayerPtr)
	{
		if (clntfd == sendPlayerPtr->m_fd)
		{
			//NetworkManager::getInstance().sendMsg(clntfd, "** 자기 자신에게는 보낼 수 없습니다.\n\r");
			return;
		}

		Player& player = playerList[clntfd];
		msg = std::format("# {}님의 쪽지 ==> ", player.m_name);
		size_t splitStrSize = splitStrList.size();
		for (size_t i = 2; i < splitStrSize; ++i)
		{
			msg += splitStrList[i] + " ";
		}
		msg += "\n\r";

		//NetworkManager::getInstance().sendMsg(sendPlayerPtr->m_fd, msg);
		//NetworkManager::getInstance().sendMsg(clntfd, "** 쪽지를 보냈습니다.\n\r");
	}
	else
	{
		msg = std::format("** {}님을 찾을 수 없습니다.\n\r", splitStrList[1]);
		//NetworkManager::getInstance().sendMsg(clntfd, msg);
	}
}

void ServerManager::showRoomInfo(int roomNum, const SOCKET clntfd)
{
	std::string msg = "";
	msg.reserve(100);

	if (roomList.find(roomNum) == roomList.end())
	{
		msg = "** 존재하지 않는 대화방입니다.\n\r";
	}
	else
	{
		Room& room = roomList[roomNum];
		msg = std::format("------------------------- 대화방 정보 -------------------------\n\r\
[  {}] ( {}/ {}) {}\n\r\
   개설시간:  {}\n\r", room.roomNum, room.curPartCnt, room.maxPartCnt, room.roomName, room.openTime);

		for (auto iter = room.roomPartInfo.begin(); iter != room.roomPartInfo.end(); ++iter)
		{
			// donghyun : 자기 자신도 브로드캐스팅함
			auto& playerInfo = iter->second;
			msg += std::format("   참여자: {}              참여시간:  {}\n\r",
				playerInfo.first->m_name, playerInfo.second);
		}

		msg += "---------------------------------------------------------------\n\r";
	}

	//NetworkManager::getInstance().sendMsg(clntfd, msg);
}

void ServerManager::showRoomList(const SOCKET clntfd)
{
	std::string msg = "";
	msg.reserve(100);

	msg += "------------------------- 대화방 목록 -------------------------\n\r";
	for (auto iter = roomList.begin(); iter != roomList.end(); ++iter)
	{
		Room& room = iter->second;
		std::string roomInfo = std::format(
			"[  {}] ( {}/ {}) {}\n\r", room.roomNum, room.curPartCnt, room.maxPartCnt, room.roomName);
		msg += roomInfo;
	}
	msg += "---------------------------------------------------------------\n\r";

	//NetworkManager::getInstance().sendMsg(clntfd, msg);
}

void ServerManager::showPlayerInfo(std::string& playerName, const SOCKET clntfd)
{

	std::string msg = "";
	msg.reserve(100);

	Player* playerPtr = findPlayerUsingName(playerName);
	if (nullptr == playerPtr)
	{
		msg = std::format("** {}님을 찾을 수 없습니다.\n\r", playerName);
	}
	else
	{
		Player& player = *playerPtr;
		if (player.m_roomNum > 0)
		{
			msg = std::format("** {}님은 현재 {}번 채팅방에 있습니다.\n\r", player.m_name, player.m_roomNum);
		}
		else
		{
			msg = std::format("** {}님은 현재 대기실에 있습니다.\n\r", player.m_name);
		}
		msg += std::format("** 접속지: {} : {}\n\r", player.m_ip, player.m_port);
	}

	//NetworkManager::getInstance().sendMsg(clntfd, msg);
}

void ServerManager::showPlayerList(const SOCKET clntfd)
{
	std::string msg = "";
	msg.reserve(100);

	msg += "------------------------- 이용자 목록 -------------------------\n\r";
	for (auto iter = playerList.begin(); iter != playerList.end(); ++iter)
	{
		auto player = iter->second;
		/*std::string playerInfo = std::format("이용자: {}              접속지 : {} : {}\n\r", player.m_name, player.m_ip, player.m_port);
		msg += playerInfo;*/
		msg += player.getInfoStr() + "\n\r";
	}
	msg += "---------------------------------------------------------------\n\r";

	//NetworkManager::getInstance().sendMsg(clntfd, msg);
}

void ServerManager::joinRoom(const int roomNum, const SOCKET clntfd)
{
	std::string msg = "";
	msg.reserve(100);

	if (roomList.find(roomNum) == roomList.end())
	{
		msg = std::format("** {}번 방은 존재하지 않는 대화방입니다.\n\r", roomNum);
		//NetworkManager::getInstance().sendMsg(clntfd, msg);
	}
	else
	{
		Player* playerPtr = findPlayerUsingfd(clntfd);
		if (playerPtr)
		{
			Room& room = roomList[roomNum];
			// donghyun : 만약 최대 인원보다 많아진다면 인원 초과로 입장 불가
			if (room.curPartCnt >= room.maxPartCnt)
			{
				msg = std::format("** 방 인원 초과로 입장이 불가능합니다. (현재인원 {} / {})\n\r",
					room.curPartCnt, room.maxPartCnt);
				//NetworkManager::getInstance().sendMsg(clntfd, msg);
				return;
			}
			// donghyun : room에 자기 자신 추가 (현실 시간도)
			room.roomPartInfo[playerPtr->m_name] = { playerPtr, ServerManager::getInstance().getCurTime() };
			++room.curPartCnt;

			playerPtr->m_roomNum = room.roomNum;

			msg = std::format("**{}님이 들어오셨습니다. (현재인원 {} / {})\n\r",
				playerPtr->m_name, room.curPartCnt, room.maxPartCnt);

			playerPtr->m_infoMapIdx = room.curPartCnt;

			broadCastInRoom(room.roomNum, msg);
		}
	}
}

int ServerManager::getChatRoomNum(SOCKET clntfd)
{
	Player* playerPtr = findPlayerUsingfd(clntfd);
	if (playerPtr)
	{
		return playerPtr->m_roomNum;
	}
	// donghyun : 못찾았을때는 문제가 있음.
	return -2;
}

void ServerManager::broadCastChatInRoom(SOCKET clntfd, int roomNum, std::string& msg)
{
	Room& room = roomList[roomNum];

	std::string broadMsg = "";
	Player* playerPtr = findPlayerUsingfd(clntfd);
	if (playerPtr)
	{
		broadMsg = std::format("{} > {}\n\r", playerPtr->m_name, msg);

		// donghyun : 만약 개인이 채팅치던 게 있으면, 그대로 클라단에 나오게 한다.
		for (auto iter = room.roomPartInfo.begin(); iter != room.roomPartInfo.end(); ++iter)
		{
			Player* iterPtr = iter->second.first;
			if (iterPtr)
			{
				std::string subMsg = iterPtr->m_buf;
				if (iterPtr->m_fd != clntfd)
				{
					if (iterPtr->m_bufStartIdx != 0)
					{
						//NetworkManager::getInstance().sendMsg(iterPtr->m_fd, "\n\r");
						//NetworkManager::getInstance().sendMsg(iterPtr->m_fd, broadMsg);
						//NetworkManager::getInstance().sendMsg(iterPtr->m_fd, subMsg.substr(0, playerPtr->m_bufStartIdx));
					}
					else
					{
						//NetworkManager::getInstance().sendMsg(iterPtr->m_fd, broadMsg);
					}
				}
				else
				{
					//NetworkManager::getInstance().sendMsg(iterPtr->m_fd, broadMsg);
				}
			}
		}
	}
}

void ServerManager::broadCastInRoom(int roomNum, std::string& msg)
{
	Room& room = roomList[roomNum];

	for (auto iter = room.roomPartInfo.begin(); iter != room.roomPartInfo.end(); ++iter)
	{
		// donghyun : 자기 자신도 브로드캐스팅함
		auto& playerInfo = iter->second;
		//NetworkManager::getInstance().sendMsg(playerInfo.first->m_fd, msg);
	}
}

void ServerManager::broadCastPacketInRoom(const SOCKET clntfd, int roomNum, Packet::PacketID packetID)
{
	if (roomList.find(roomNum) == roomList.end())
	{
		return;
	}

	Room& room = roomList[roomNum];

	switch (packetID)
	{
	case Packet::PacketID::PLAY:
	{
		Player* playerPtr = ServerManager::getInstance().findPlayerUsingfd(clntfd);
		if (!playerPtr)
		{
			return;
		}

		// donghyun : playpacket 제작
		int playerIdx = playerPtr->m_infoMapIdx;
		Packet::PlayPacket playPacket(playerPtr->m_infoMapIdx);
		for (int i = 0; i < 3; i++)
		{
			playPacket.posVec[i] = playerPtr->m_position[i];
			playPacket.rotVec[i] = playerPtr->m_rotation[i];
		}

		for (auto iter = room.roomPartInfo.begin(); iter != room.roomPartInfo.end(); ++iter)
		{
			auto playerInfo = iter->second.first;
			// donghyun : 자기 자신에겐 브로드캐스팅 X
			if (playerInfo->m_fd == clntfd)
			{
				continue;
			}
			NetworkManager::getInstance().sendPacket(playerInfo->m_fd, playPacket, playPacket.packetSize);
		}
		break;
	}
	case Packet::PacketID::GAMESTART:
	{
		// donghyun : GameStartPacket 패킷 제작
		int playerPositionIdx = 0;
		for (auto iter = room.roomPartInfo.begin(); iter != room.roomPartInfo.end(); ++iter)
		{
			// donghyun : 자기 자신도 브로드캐스팅함
			auto playerPtr = iter->second.first;
			//Packet::PlayerInfo playerInfo(playerPtr->m_infoMapIdx, playerPtr->m_name.c_str(), playerPtr->m_position, playerPtr->m_rotation);
			Packet::PlayerInfo playerInfo(playerPtr->m_infoMapIdx, playerPtr->m_name.c_str(), ServerProtocol::PLAYER_INITPOS[playerPositionIdx], playerPtr->m_rotation);
			for (int i = 0; i < 3; ++i)
			{
				playerPtr->m_position[i] = ServerProtocol::PLAYER_INITPOS[playerPositionIdx][i];
			}
			playerPositionIdx++;

			Packet::GameStartPacket gameStartPacket(playerInfo);
			// donghyun : 모든 클라에게 브로드캐스팅
			for (auto iter = room.roomPartInfo.begin(); iter != room.roomPartInfo.end(); ++iter)
			{
				// donghyun : 자기 자신도 브로드캐스팅함
				auto& playerInfo = iter->second;
				NetworkManager::getInstance().sendPacket(playerInfo.first->m_fd, gameStartPacket, gameStartPacket.packetSize);
			}
		}
		break;
	}
	case Packet::PacketID::SPAWN:
	{
		break;
	}
	default:
	{
		break;
	}
	}
}

void ServerManager::quitPlayer(const SOCKET clntfd)
{
	Player* playerPtr = findPlayerUsingfd(clntfd);
	if (playerPtr)
	{
		if (playerPtr->m_roomNum > 0)
		{
			quitRoom(playerPtr->m_roomNum, playerPtr);
		}
		// donghyun : 플레이어 set에서도 빼기
		playerList.erase(playerPtr->m_fd);
	}
	NetworkManager::getInstance().closeClient(clntfd);
}

void ServerManager::quitRoom(const int roomNum, Player* playerPtr)
{
	Room& room = roomList[roomNum];
	room.roomPartInfo.erase(playerPtr->m_name);
	room.curPartCnt--;

	playerPtr->m_roomNum = -1;

	std::string msg = "";
	msg.reserve(100);
	msg = std::format("**{}님이 나가셨습니다. (현재인원 {} / {})\n\r",
		playerPtr->m_name, room.curPartCnt, room.maxPartCnt);
	broadCastInRoom(room.roomNum, msg);
	//NetworkManager::getInstance().sendMsg(playerPtr->m_fd, "**채팅방에서 나왔습니다.\n\r");

	// donghyun : 현재 인원이 0명 이하면 방 폭파
	if (room.curPartCnt <= 0)
	{
		roomList.erase(room.roomNum);
	}
}

// donghyun : 첫 클라 소켓 연결 요청 시에 사용됨
bool ServerManager::addPlayer(Player& player)
{
	return playerList.insert({ player.m_fd, player }).second;
}

int ServerManager::getPlayerNum()
{
	return playerList.size();
}

void ServerManager::addRoomTimerList(const int roomNum)
{
	if (updateRoomTimerList.find(roomNum) != updateRoomTimerList.end())
	{
		return;
	}
	updateRoomTimerList.insert(roomNum);
}

void ServerManager::RunTimer()
{
	//타이머 만들기
	m_timerThread = static_cast<std::jthread>
		([this](std::stop_token stoken)
	{
		while (!stoken.stop_requested())
		{
			// 특정 구간마다 깨어나서 해당 메소드 실행
			UpdateRoomTimer();
			std::this_thread::sleep_for(static_cast<std::chrono::milliseconds>(ServerProtocol::TIMER_UPDATE_PERIOD));
		}
	});
}

void ServerManager::UpdateRoomTimer()
{
	// 각 플레이어에 시간 send
	for (auto& roomNum : updateRoomTimerList)
	{
		if (roomList.find(roomNum) == roomList.end())
		{
			continue;
		}

		Room& room = roomList[roomNum];
		room.curPlayTime++;

		// 각 방의 플레이어들에게 타이머 전송
		for (auto iter = room.roomPartInfo.begin(); iter != room.roomPartInfo.end(); ++iter)
		{
			auto playerPtr = iter->second.first;
			// donghyun : 타이머 패킷 만들어서 전송
			Packet::TimerPacket timerPacket(room.curPlayTime);
			NetworkManager::getInstance().sendPacket(playerPtr->m_fd, timerPacket, timerPacket.packetSize);

			// donghyun : 스폰 패킷 만들어서 전송 (방향, idx 등은 모두 랜덤)
			srand(time(NULL)); // 시드(seed) 값을 현재 시간으로 설정
			int rand_num = -1;

			rand_num = rand() % ServerProtocol::RANDNUM_SEEDRANGE + 1; // 1~1000 사이의 난수 생성
			bool IsHorizontal = rand_num % 2 == 0 ? true : false;
			rand_num = rand() % ServerProtocol::RANDNUM_SEEDRANGE + 1; // 1~1000 사이의 난수 생성
			unsigned short lineIdx = rand_num % ServerProtocol::GAMEMAP_SIZE + 1;
			rand_num = rand() % ServerProtocol::RANDNUM_SEEDRANGE + 1; // 1~1000 사이의 난수 생성
			bool directionFlag = rand_num % 2 == 0 ? true : false;

			Packet::SpawnPacket spawnPacket(IsHorizontal, lineIdx, directionFlag);
			NetworkManager::getInstance().sendPacket(playerPtr->m_fd, spawnPacket, spawnPacket.packetSize);
		}
	}
}

// donghyun : 만약 못찾았을 때는 nullptr 반환
Player* ServerManager::findPlayerUsingfd(const SOCKET clntfd)
{
	return (playerList.find(clntfd) == playerList.end() ? nullptr : &(playerList[clntfd]));
}

// donghyun : 만약 못찾았을 때는 nullptr 반환
Player* ServerManager::findPlayerUsingName(const std::string& playerName)
{
	Player* playerPtr = nullptr;
	for (auto iter = playerList.begin(); iter != playerList.end(); ++iter)
	{
		if (iter->second.m_name == playerName)
		{
			playerPtr = &(iter->second);
			break;
		}
	}
	return playerPtr;
}

Player* ServerManager::findPlayerUsingInfoMapIdx(const unsigned short infoMapIdx)
{
	Player* playerPtr = nullptr;
	for (auto iter = playerList.begin(); iter != playerList.end(); ++iter)
	{
		if (iter->second.m_infoMapIdx == infoMapIdx)
		{
			playerPtr = &(iter->second);
			break;
		}
	}
	return playerPtr;
}

std::string ServerManager::getCurTime()
{
	// donghyun : HH:MM:SS 형식으로 구하는 함수
	auto now = std::chrono::system_clock::now();
	std::time_t current_time = std::chrono::system_clock::to_time_t(now);
	std::tm local_time{};
	localtime_s(&local_time, &current_time);
	char time_str[9];
	std::strftime(time_str, sizeof(time_str), "%T", &local_time);

	return time_str;
}


