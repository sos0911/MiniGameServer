#pragma once
namespace PacketProtocol
{
	constexpr unsigned short PACKET_MAXSIZE = 1024;
	constexpr unsigned short NICKNAME_MAXSIZE = 32;
	constexpr unsigned short BUF_MAXSIZE = 1024;
	constexpr unsigned short IP_MAXSIZE = 20;
}
namespace ServerProtocol
{
	constexpr unsigned short ROOMNAME_MAXSIZE = 32;
	// donghyun : ms 단위
	constexpr unsigned int TIMER_UPDATE_PERIOD = 1000;
	constexpr unsigned short PLAYTIME_MAXSIZE = 9999;
	constexpr unsigned short SPAWN_PHASE_TIMES[] =	   {    0,    5,   10,   15,   20,   30,  40, PLAYTIME_MAXSIZE };
	constexpr unsigned short SPAWN_PHASE_INTERVALS[] = { 1100, 2100, 1100, 2100, 1100, 2100, 1100 };

	constexpr unsigned int TASK_UPDATE_PERIOD = 50;
	constexpr unsigned int RANDNUM_SEEDRANGE = 1000;
	constexpr unsigned int GAMEMAP_SIZE = 7;
	// donghyun : 0번째 position 정보는 사용X
	constexpr float PLAYER_INITPOS[5][3] = { {100.0f, -100.0f, 140.0f},
										   {100.0f, 100.0f, 140.0f},
										   {-100.0f, -100.0f, 140.0f},
										   {-100.0f, 100.0f, 140.0f},
										   {0.0f, 0.0f, 140.0f} };
	constexpr float PLAYER_COLLIDER_RADIUS = 110.0f;
	// donghyun : 언리얼 기준 width (x축?)
	constexpr float MONSTER_BOX_COLLIDER_WIDTH = 230.0f;
	// donghyun : 언리얼 기준 height (y축?)
	constexpr float MONSTER_BOX_COLLIDER_HEIGHT = 120.0f;

	constexpr unsigned short ROOM_MAXPARTCNT = 3;
	constexpr short ROOM_TIMER_STARTTIME = -4;
}
#pragma pack(push,1)
namespace Packet
{
	enum class PacketID : unsigned char
	{
		// server -> client
		PLAY,
		SPAWN,
		GAMEREADY,
		GAMESTART,
		LOGINRESULT,
		MAKEROOMRESULT,
		JOINROOMRESULT,
		TIMER,
		PMCOLLIDERESULT,
		PPCOLLIDERESULT,
		HEART,
		GAMEEND,
		ROOMINFO,
		// client -> server
		UPDATE,
		LOGINREQUEST,
		MAKEROOMREQUEST,
		JOINROOMREQUEST,
		PMCOLLIDEREQUEST,
		PPCOLLIDEREQUEST,
	};

	enum class LoginPacketID : unsigned char
	{
		SUCCESS,
		FAIL
	};

	enum class JoinRoom : unsigned char
	{
		SUCCESS,
		FAIL
	};
	
	// server -> client
	struct PlayPacket
	{
		unsigned short packetSize;
		PacketID packetID;
		unsigned short playerIdx;
		float posVec[3];
		float rotVec[3];
		PlayPacket(unsigned short InfoMapIdx);
	};

	struct SpawnPacket
	{
		unsigned short packetSize;
		PacketID packetID;
		// donghyun : row idx, 모두 1~7
		unsigned short rowIdx;
		// donghyun : col idx, 모두 1~7
		unsigned short colIdx;
		// donghyun : true면 왼쪽(수평) or 위쪽(수직), False면 오른쪽(수평) or 아래쪽(수직)
		unsigned short directionIdx;
		SpawnPacket(bool in_IsHorizontal, unsigned short in_lineIdx, bool in_directionFlag);
	};

	// donghyun : GameReadyPacket 내에 포함되는 개별 플레이어 정보 구조체
	struct PlayerInfo
	{
		unsigned short playerIdx;
		char nickName[PacketProtocol::NICKNAME_MAXSIZE];
		float posVec[3];
		float rotVec[3];
		PlayerInfo() = default;
		PlayerInfo(unsigned short in_playerIdx, const char* in_nickName, const float* in_posVec, const float* in_rotvec);
		PlayerInfo(const PlayerInfo& in_playerInfo);

		static unsigned short getPlayerInfoByteSize();
	};

	// donghyun : 모든 플레이어 참가 시 게임 시작 전 각 플레이어의 정보를 알려주는 패킷
	struct GameReadyPacket
	{
		unsigned short packetSize;
		PacketID packetID;
		PlayerInfo playerInfo;
		GameReadyPacket(const PlayerInfo& in_playerInfo);
	};

	// donghyun : 게임 시작을 알려주는 패킷
	struct GameStartPacket
	{
		unsigned short packetSize = sizeof(GameStartPacket);
		PacketID packetID = Packet::PacketID::GAMESTART;
	};

	struct LoginResultPacket
	{
		unsigned short packetSize;
		PacketID packetID;
		bool LoginSuccess;
		// donghyun : playerIdx는 로그인 실패 시 -1
		unsigned short playerIdx;
		LoginResultPacket(const bool in_LoginSuccess, const unsigned short in_playerIdx);
	};

	struct MakeRoomResultPacket
	{
		unsigned short packetSize;
		PacketID packetID;
		char roomName[ServerProtocol::ROOMNAME_MAXSIZE];
		// TODO : 생성자 구현
		//MakeRoomResultPacket();
	};

	struct JoinRoomResultPacket
	{
		unsigned short packetSize;
		PacketID packetID;
		bool JoinRoomSuccess;
		JoinRoomResultPacket(bool In_JoinRoomSuccess);
	};
	
	struct TimerPacket
	{
		unsigned short packetSize = sizeof(TimerPacket);
		PacketID packetID = Packet::PacketID::TIMER;
		short timeSecond;
		TimerPacket(short in_timeSecond);
	};

	// donghyun : gameendPacket에 들어가는 playerGameEndInfo
	struct PlayerGameEndInfo
	{
		unsigned short playerIdx;
		unsigned short rank;
		long long survivedTime;
		PlayerGameEndInfo() = default;
		PlayerGameEndInfo(const unsigned short in_playerIdx, const unsigned short in_rank, const long long in_survivedTime);
	};

	struct GameEndPacket
	{
		unsigned short packetSize = sizeof(GameEndPacket);
		PacketID packetID = Packet::PacketID::GAMEEND;
		PlayerGameEndInfo playerGameEndInfoArr[ServerProtocol::ROOM_MAXPARTCNT];
	};

	struct RoomInfoPacket
	{
		unsigned short packetSize = sizeof(RoomInfoPacket);
		PacketID packetID = Packet::PacketID::ROOMINFO;
		unsigned short roomPartCnt;
		RoomInfoPacket(const unsigned short in_roomPartCnt);
	};

	// donghyun : 플-투 충돌 결과 패킷
	struct PMCollideResultPacket
	{
		unsigned short packetSize;
		PacketID packetID;
		unsigned short playerIdx;
		bool IsCollided;
		float forceDir[3];
		PMCollideResultPacket() = default;
		PMCollideResultPacket(const unsigned short in_playerIdx, const bool in_IsCollided, const float* in_forceDir);
	};

	//// donghyun : 플-플 충돌 결과 패킷
	//struct PlayerCollideInfo
	//{
	//	unsigned short playerIdx;
	//	float forceDir[3];
	//	PlayerCollideInfo() = default;
	//	PlayerCollideInfo(const unsigned short in_playerIdx, const float* in_forceDir);
	//	PlayerCollideInfo(const PlayerCollideInfo& in_playerInfo);
	//};

	struct PPCollideResultPacket
	{
		unsigned short packetSize;
		PacketID packetID;
		bool IsCollided;
		//PlayerCollideInfo playerCollideInfoArr[2];
		float forceDir[3];
		PPCollideResultPacket() = default;
		//PPCollideResultPacket(const bool in_IsCollided, const PlayerCollideInfo* in_playerCollideInfoArr);
		PPCollideResultPacket(const bool in_IsCollided, const float* in_forceDir);
	};

	struct HeartPacket
	{
		unsigned short packetSize = sizeof(HeartPacket);
		PacketID packetID = Packet::PacketID::HEART;
		unsigned short playerIdx;
		unsigned short heartCnt;
		HeartPacket(const unsigned short in_playerIdx, const unsigned short in_heartCnt);
	};
	
	/////////////////////
	// client -> server//
	/////////////////////
	struct UpdatePacket
	{
		unsigned short packetSize;
		PacketID packetID;
		float posVec[3];
		float rotVec[3];
	};

	struct LoginRequestPacket
	{
		unsigned short packetSize;
		PacketID packetID;
		char LoginNickname[PacketProtocol::NICKNAME_MAXSIZE];
		// 클라
		//LoginRequestPacket(const FString& nickname);
	};

	struct MakeRoomRequestPacket
	{
		unsigned short packetSize;
		PacketID packetID;
		char LoginNickname[PacketProtocol::NICKNAME_MAXSIZE];
	};

	struct JoinRoomRequestPacket
	{
		unsigned short packetSize;
		PacketID packetID;
		unsigned short RoomNum;
	};

	struct PMColliderRequestPacket
	{
		const unsigned short packetSize    = sizeof( PMColliderRequestPacket );
		      const PacketID packetID      = PacketID::PMCOLLIDEREQUEST;
	                   float monsterPos[3];
	};

	struct PPColliderRequestPacket
	{
		unsigned short packetSize;
		PacketID packetID;
		unsigned short oppoPlayerIdx;
	};
}
#pragma pack(pop)
