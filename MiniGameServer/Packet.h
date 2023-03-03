#pragma once
#pragma pack(push,1)
enum class PacketID
{
	PLAY,
	SKILL,
	LOGIN,
	JOINROOM
};

enum class LoginPacketID
{
	SUCCESS,
	FAIL
};

enum class JoinRoom
{
	SUCCESS,
	FAIL
};

struct LoginPacket
{
	unsigned short packetSize;
	PacketID packetID;
	bool LoginSuccess;
	LoginPacket(bool In_LoginSuccess);
};

struct JoinRoomPacket
{
	int packetSize;
	PacketID packetID;
	bool JoinRoomSuccess;
	JoinRoomPacket(bool In_JoinRoomSuccess);
};
#pragma pack(pop)
