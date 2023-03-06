#include "pch.h"

#include "Packet.h"

namespace Packet
{
	LoginResultPacket::LoginResultPacket(bool In_LoginSuccess)
	{
		packetSize = sizeof(unsigned short) + sizeof(PacketID) + sizeof(bool);
		packetID = PacketID::LOGINRESULT;
		LoginSuccess = In_LoginSuccess;
	}

	JoinRoomResultPacket::JoinRoomResultPacket(bool In_JoinRoomSuccess)
	{
		packetSize = sizeof(unsigned short) + sizeof(PacketID) + sizeof(bool);
		packetID = PacketID::JOINROOMRESULT;
		JoinRoomSuccess = In_JoinRoomSuccess;
	}

	PlayPacket::PlayPacket(int InfoMapIdx)
	{
		packetSize = sizeof(unsigned short) + sizeof(PacketID) + sizeof(unsigned short) + (sizeof(float) * 6);
		packetID = PacketID::PLAY;
		playerIdx = InfoMapIdx;
		posVec[0] = { 0.0f,};
		rotVec[0] = { 0.0f,};
	}
}
