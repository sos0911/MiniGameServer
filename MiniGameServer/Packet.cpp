#include "pch.h"

#include "Packet.h"

LoginPacket::LoginPacket(bool In_LoginSuccess)
{
	packetSize = sizeof(int) + sizeof(PacketID) + sizeof(bool);
	packetID = PacketID::LOGIN;
	LoginSuccess = In_LoginSuccess;
}

JoinRoomPacket::JoinRoomPacket(bool In_JoinRoomSuccess)
{
	packetSize = sizeof(int) + sizeof(PacketID) + sizeof(bool);
	packetID = PacketID::JOINROOM;
	JoinRoomSuccess = In_JoinRoomSuccess;
}