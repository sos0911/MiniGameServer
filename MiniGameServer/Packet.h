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
	constexpr unsigned int TASK_UPDATE_PERIOD = 50;
	constexpr unsigned int RANDNUM_SEEDRANGE = 1000;
	constexpr unsigned int GAMEMAP_SIZE = 7;
}
#pragma pack(push,1)
namespace Packet
{
	enum class PacketID : unsigned char
	{
		// server -> client
		PLAY,
		SPAWN,
		GAMESTART,
		LOGINRESULT,
		MAKEROOMRESULT,
		JOINROOMRESULT,
		TIMER,
		// client -> server
		UPDATE,
		LOGINREQUEST,
		MAKEROOMREQUEST,
		JOINROOMREQUEST,
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
		PlayPacket(int InfoMapIdx);
	};

	struct SpawnPacket
	{
		unsigned short packetSize;
		PacketID packetID;
		// donghyun : true면 수평 방향, 아니면 수직 방향
		bool IsHorizontal;
		// donghyun : row, col 둘 중 하나, 모두 1~7
		unsigned short lineIdx;
		// donghyun : true면 왼쪽(수평) or 위쪽(수직), False면 오른쪽(수평) or 아래쪽(수직)
		bool directionFlag;
		SpawnPacket(bool in_IsHorizontal, unsigned short in_lineIdx, bool in_directionFlag);
	};

	// donghyun : GameStartPacket 내에 포함되는 개별 플레이어 정보 구조체
	struct PlayerInfo
	{
		unsigned short playerIdx;
		char nickName[PacketProtocol::NICKNAME_MAXSIZE];
		float posVec[3];
		float rotVec[3];
		PlayerInfo() {}
		PlayerInfo(unsigned short in_playerIdx, const char* in_nickName, float* in_posVec, float* in_rotvec);
		PlayerInfo(const PlayerInfo& in_playerInfo);

		static unsigned short getPlayerInfoByteSize();
	};

	// donghyun : 게임 시작 전 각 플레이어의 정보를 알려주는 패킷
	struct GameStartPacket
	{
		unsigned short packetSize;
		PacketID packetID;
		PlayerInfo playerInfo;
		GameStartPacket(const PlayerInfo& in_playerInfo);
	};

	struct LoginResultPacket
	{
		unsigned short packetSize;
		PacketID packetID;
		bool LoginSuccess;
		LoginResultPacket(bool In_LoginSuccess);
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
		int packetSize;
		PacketID packetID;
		bool JoinRoomSuccess;
		JoinRoomResultPacket(bool In_JoinRoomSuccess);
	};
	
	struct TimerPacket
	{
		int packetSize;
		PacketID packetID;
		unsigned short timeSecond;
		TimerPacket(unsigned short in_timeSecond);
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
		int packetSize;
		PacketID packetID;
		unsigned short RoomNum;
	};
}
#pragma pack(pop)
