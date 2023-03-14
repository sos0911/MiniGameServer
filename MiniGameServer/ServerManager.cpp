#include "pch.h"

#include "ServerManager.h"
#include "NetworkManager.h"
#include <iostream>
#include <format>
#include <ctime>
#include <chrono>

ServerManager::ServerManager()
{
}

void ServerManager::loginProcess(const SOCKET clntfd, const char* packetChar)
{
	Packet::LoginRequestPacket loginRequestPacket = *(Packet::LoginRequestPacket*)(packetChar);

	Player* playerPtr = findPlayerUsingfd(clntfd);
	if (!playerPtr)
	{
		Packet::LoginResultPacket loginPacket(false, -1);
		NetworkManager::getInstance().sendPacket(clntfd, loginPacket, loginPacket.packetSize);
		return;
	}
	if (findPlayerUsingName(loginRequestPacket.LoginNickname))
	{
		Packet::LoginResultPacket loginPacket(false, -1);
		NetworkManager::getInstance().sendPacket(clntfd, loginPacket, loginPacket.packetSize);
		return;
	}
	playerPtr->m_name = loginRequestPacket.LoginNickname;

	Packet::LoginResultPacket loginPacket(true, playerPtr->m_roomPlayerIdx);
	NetworkManager::getInstance().sendPacket(clntfd, loginPacket, loginPacket.packetSize);

	// donghyun : 방에 없는 3명이 모일 때마다 묶어서 게임 시작하게끔 함
	if (playerPtr->m_roomNum == -1)
	{
		bool joinRoomResult = joinRoom(lastRoomNum, clntfd);

		if (joinRoomResult)
		{
			// donghyun : 현재 방 인원 패킷 전송
			ServerManager::getInstance().broadCastPacketInRoom(clntfd, playerPtr->m_roomNum, Packet::PacketID::ROOMINFO);

			// donghyun : 아직 마지막 방에 들어갈 수 있었음
			// donghyun : 3명 다 들어왔는지 체크하고 맞으면 게임 시작 처리
			if (roomList[lastRoomNum].curPartCnt == ServerProtocol::ROOM_MAXPARTCNT)
			{
				// donghyun : 해당 방을 일정 주기마다 타이머 증가시키는 방 리스트에 추가
				addRoomTimerList(lastRoomNum);
				std::cout << " gameReady logic!" << '\n';
				gameReadyProcess(clntfd, lastRoomNum);
			}
		}
		else
		{
			// donghyun : 마지막 방이 풀방이라 하나 만들어야 함
			// donghyun : 물론 nullptr일 경우 있지만 고려하지 않음
			ServerManager::getInstance().createRoom(clntfd, ServerProtocol::ROOM_MAXPARTCNT);

			// donghyun : 현재 방 인원 패킷 전송
			ServerManager::getInstance().broadCastPacketInRoom(clntfd, playerPtr->m_roomNum, Packet::PacketID::ROOMINFO);
		}
	}
}

void ServerManager::gameReadyProcess(const SOCKET clntfd, const int roomNum)
{
	if (roomList.find(roomNum) == roomList.end())
	{
		return;
	}
	Room& room = roomList[roomNum];

	// donghyun : 2명이 찼을 때 게임 시작 패킷 브로드캐스팅
	broadCastPacketInRoom(clntfd, lastRoomNum, Packet::PacketID::GAMEREADY);
}

void ServerManager::gameStartProcess(const SOCKET clntfd, const int roomNum)
{
	if (roomList.find(roomNum) == roomList.end())
	{
		return;
	}
	Room& room = roomList[roomNum];
	// donghyun : 게임 시작 시각 저장
	room.gameStartTime = std::chrono::high_resolution_clock::now();

	// donghyun : 2명이 찼을 때 게임 시작 패킷 브로드캐스팅
	broadCastPacketInRoom(clntfd, lastRoomNum, Packet::PacketID::GAMESTART);
	RunSpawner(lastRoomNum);
}

void ServerManager::createRoom(const SOCKET clntfd, const unsigned short maxCnt)
{
	Player* playerPtr = findPlayerUsingfd(clntfd);
	if (playerPtr)
	{
		Room room(maxCnt, *playerPtr);
		roomList[room.roomNum] = room;
		// donghyun : 방장에게도 속한 방이 있다고 표시해주기
		playerPtr->m_roomNum = room.roomNum;
		playerPtr->m_roomPlayerIdx = 1;

		//std::string msg = std::format("** 대화방이 개설되었습니다.\n\r** {}님이 들어오셨습니다. (현재인원 {} / {})\n\r", playerPtr->m_name, room.curPartCnt, room.maxPartCnt);
		//NetworkManager::getInstance().sendMsg(clntfd, msg);
	}
}

bool ServerManager::joinRoom(const int roomNum, const SOCKET clntfd)
{
	if (roomList.find(roomNum) == roomList.end())
	{
		return false;
	}

	Player* playerPtr = findPlayerUsingfd(clntfd);
	if (!playerPtr)
	{
		return false;
	}

	Room& room = roomList[roomNum];
	// donghyun : 만약 최대 인원보다 많아진다면 인원 초과로 입장 불가
	if (room.curPartCnt >= room.maxPartCnt)
	{
		return false;
	}
	// donghyun : room에 자기 자신 추가 (현실 시간도)
	room.roomPartInfo[playerPtr->m_name] = { playerPtr, ServerManager::getInstance().getCurTime() };
	++room.curPartCnt;

	playerPtr->m_roomNum = room.roomNum;
	playerPtr->m_roomPlayerIdx = room.curPartCnt;
	return true;
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
		int playerIdx = playerPtr->m_roomPlayerIdx;
		Packet::PlayPacket playPacket(playerPtr->m_roomPlayerIdx);
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
	case Packet::PacketID::GAMEREADY:
	{
		// donghyun : GameReadyPacket 패킷 제작
		int playerPositionIdx = 0;
		for (auto iter = room.roomPartInfo.begin(); iter != room.roomPartInfo.end(); ++iter)
		{
			// donghyun : 자기 자신도 브로드캐스팅함
			auto playerPtr = iter->second.first;
			//Packet::PlayerInfo playerInfo(playerPtr->m_roomPlayerIdx, playerPtr->m_name.c_str(), playerPtr->m_position, playerPtr->m_rotation);
			Packet::PlayerInfo playerInfo(playerPtr->m_roomPlayerIdx, playerPtr->m_name.c_str(), ServerProtocol::PLAYER_INITPOS[playerPositionIdx], playerPtr->m_rotation);
			for (int i = 0; i < 3; ++i)
			{
				playerPtr->m_position[i] = ServerProtocol::PLAYER_INITPOS[playerPositionIdx][i];
			}
			playerPositionIdx++;

			Packet::GameReadyPacket GameReadyPacket(playerInfo);
			// donghyun : 모든 클라에게 브로드캐스팅
			for (auto iter = room.roomPartInfo.begin(); iter != room.roomPartInfo.end(); ++iter)
			{
				// donghyun : 자기 자신도 브로드캐스팅함
				auto& playerInfo = iter->second;
				NetworkManager::getInstance().sendPacket(playerInfo.first->m_fd, GameReadyPacket, GameReadyPacket.packetSize);
			}
		}
		break;
	}
	case Packet::PacketID::SPAWN:
	{
		break;
	}
	case Packet::PacketID::GAMESTART:
	{
		// donghyun : GameStartPacket 패킷 제작
		Packet::GameStartPacket gameStartPacket;
		for (auto iter = room.roomPartInfo.begin(); iter != room.roomPartInfo.end(); ++iter)
		{
			// donghyun : 자기 자신도 브로드캐스팅함
			auto playerPtr = iter->second.first;
			NetworkManager::getInstance().sendPacket(playerPtr->m_fd, gameStartPacket, gameStartPacket.packetSize);
		}
		break;
	}
	case Packet::PacketID::ROOMINFO:
	{
		Player* playerPtr = ServerManager::getInstance().findPlayerUsingfd(clntfd);
		if (!playerPtr)
		{
			return;
		}
		if (roomList.find(playerPtr->m_roomNum) == roomList.end())
		{
			return;
		}
		Room& room = roomList[playerPtr->m_roomNum];

		Packet::RoomInfoPacket roomInfoPacket(room.curPartCnt);
		// donghyun : 모든 클라에게 브로드캐스팅
		for (auto iter = room.roomPartInfo.begin(); iter != room.roomPartInfo.end(); ++iter)
		{
			// donghyun : 자기 자신도 브로드캐스팅함
			auto& playerInfo = iter->second;
			NetworkManager::getInstance().sendPacket(playerInfo.first->m_fd, roomInfoPacket, roomInfoPacket.packetSize);
		}

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
	if (!playerPtr)
	{
		return;
	}

	if (playerPtr->m_roomNum > 0)
	{
		quitRoom(playerPtr->m_roomNum, playerPtr);
	}
	// donghyun : 플레이어 set에서도 빼기
	playerList.erase(playerPtr->m_fd);
	NetworkManager::getInstance().closeClient(clntfd);
}

void ServerManager::quitRoom(const int roomNum, Player* playerPtr)
{
	Room& room = roomList[roomNum];
	room.roomPartInfo.erase(playerPtr->m_name);
	room.curPartCnt--;

	playerPtr->m_roomNum = -1;

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

int ServerManager::getLoginedPlayerNum()
{
	int playerCnt = 0;
	for (auto iter = playerList.begin(); iter != playerList.end(); ++iter)
	{
		auto& player = iter->second;
		if (player.m_name != "")
		{
			playerCnt++;
		}
	}
	return playerCnt;
}

void ServerManager::addRoomTimerList(const int roomNum)
{
	if (updateRoomTimerList.find(roomNum) != updateRoomTimerList.end())
	{
		return;
	}
	updateRoomTimerList.insert(roomNum);
}

void ServerManager::deleteRoomTimerList(const int roomNum)
{
	if (updateRoomTimerList.find(roomNum) == updateRoomTimerList.end())
	{
		return;
	}
	updateRoomTimerList.erase(roomNum);
}

void ServerManager::stopSpawnThread(const int roomNum)
{
	if (roomList.find(roomNum) == roomList.end())
	{
		return;
	}
	if (m_spawnThreadSet.find(roomNum) == m_spawnThreadSet.end())
	{
		return;
	}
	m_spawnThreadSet[roomNum].request_stop();
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

		// donghyun : mutex 처리
		std::lock_guard<std::mutex> lockGuard(m_mutex);
		{
			room.curPlayTime++;
		}

		// donghyun : 타이머 패킷 만들어서 전송
		Packet::TimerPacket timerPacket(room.curPlayTime);
		std::cout << "current time : " << room.curPlayTime << '\n';

		if (0 == room.curPlayTime)
		{
			std::cout << " gamestart logic!" << '\n';
			// donghyun : 그냥 방 안에 있는 아무나 인자로 넘김 (땜빵용)
			for (auto iter = room.roomPartInfo.begin(); iter != room.roomPartInfo.end(); ++iter)
			{
				Player* playerPtr = iter->second.first;
				SOCKET playerSoc = playerPtr->m_fd;
				gameStartProcess(playerSoc, roomNum);
				break;
			}
		}

		// 각 방의 플레이어들에게 타이머 전송
		for (auto iter = room.roomPartInfo.begin(); iter != room.roomPartInfo.end(); ++iter)
		{
			auto playerPtr = iter->second.first;

			NetworkManager::getInstance().sendPacket(playerPtr->m_fd, timerPacket, timerPacket.packetSize);
			//NetworkManager::getInstance().sendPacket(playerPtr->m_fd, spawnPacket, spawnPacket.packetSize);
		}
	}
}

void ServerManager::RunSpawner(const int roomNum)
{
	std::cout << "spawner thread incoming!" << '\n';
	//스폰 타이머 만들기
	std::jthread spawnThread = static_cast<std::jthread>
		([this, roomNum](std::stop_token stoken)
			{
				int spawnPhaseIdx = 0;
				while (!stoken.stop_requested())
				{
					// 특정 구간마다 깨어나서 해당 메소드 실행
					auto roomItr = roomList.find(roomNum);
					if (roomItr == roomList.end())
					{
						continue;
					}
					Room& room = roomItr->second;

					// donghyun : 스폰 패킷 만들어서 전송 (방향, idx 등은 모두 랜덤)
					//srand(time(NULL)); // 시드(seed) 값을 현재 시간으로 설정

					// std::chrono::system_clock의 현재 시간을 가져온다.
					auto now = std::chrono::system_clock::now();

					// now의 시간을 epoch 이후의 밀리초 단위로 계산한다.
					auto now_ms = std::chrono::time_point_cast<std::chrono::milliseconds>(now);

					// now_ms의 시간을 시드로 사용하여 난수 생성기를 초기화한다.
					auto seed = now_ms.time_since_epoch().count();
					std::srand(static_cast<unsigned int>(seed));

					int rand_num = -1;

					rand_num = rand() % ServerProtocol::RANDNUM_SEEDRANGE + 1; // 1~1000 사이의 난수 생성
					bool IsHorizontal = rand_num % 2 == 0 ? true : false;
					rand_num = rand() % ServerProtocol::RANDNUM_SEEDRANGE + 1; // 1~1000 사이의 난수 생성
					unsigned short lineIdx = rand_num % ServerProtocol::GAMEMAP_SIZE + 1;
					rand_num = rand() % ServerProtocol::RANDNUM_SEEDRANGE + 1; // 1~1000 사이의 난수 생성
					bool directionFlag = rand_num % 2 == 0 ? true : false;

					Packet::SpawnPacket spawnPacket(IsHorizontal, lineIdx, directionFlag);
					std::cout << "spawn packet send :: " << IsHorizontal << " :: " << lineIdx << " :: " << directionFlag << '\n';

					// 각 방의 플레이어들에게 스폰 패킷 전송
					for (auto iter = room.roomPartInfo.begin(); iter != room.roomPartInfo.end(); ++iter)
					{
						auto playerPtr = iter->second.first;
						NetworkManager::getInstance().sendPacket(playerPtr->m_fd, spawnPacket, spawnPacket.packetSize);
					}

					// donghyun : mutex 적용
					unsigned short curPlayTime = 0;
					{
						std::lock_guard<std::mutex> lockGuard(m_mutex);
						curPlayTime = room.curPlayTime;
					}

					if (curPlayTime >= ServerProtocol::SPAWN_PHASE_TIMES[spawnPhaseIdx])
					{
						spawnPhaseIdx++;
					}

					std::cout << "spawnthread :: sleep for : " << ServerProtocol::SPAWN_PHASE_INTERVALS[spawnPhaseIdx - 1] << '\n';

					std::this_thread::sleep_for(static_cast<std::chrono::milliseconds>(ServerProtocol::SPAWN_PHASE_INTERVALS[spawnPhaseIdx - 1]));
				}
			});
	
	m_spawnThreadSet.insert({ roomNum, std::move(spawnThread) });
}

Room* ServerManager::findRoomUsingRoomNum(const int roomNum)
{
	return (roomList.find(roomNum) == roomList.end() ? nullptr : &(roomList[roomNum]));
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
		if (iter->second.m_roomPlayerIdx == infoMapIdx)
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


