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

	PlayerInfo::PlayerInfo(unsigned short in_playerIdx, const char* in_nickName, float* in_posVec, float* in_rotvec)
	{
		nickName[0] = { 0, };
		playerIdx = in_playerIdx;
		memcpy_s(&playerIdx, PacketProtocol::NICKNAME_MAXSIZE, in_nickName, strlen(in_nickName));
		for (int i = 0; i < 3; ++i)
		{
			posVec[i] = in_posVec[i];
			rotVec[i] = in_rotvec[i];
		}
	}

	PlayerInfo::PlayerInfo(const PlayerInfo& in_playerInfo)
	{
		// 복사 생성자
		playerIdx = in_playerInfo.playerIdx;
		strcpy_s(nickName, PacketProtocol::NICKNAME_MAXSIZE, in_playerInfo.nickName);
		for (int i = 0; i < 3; ++i)
		{
			posVec[i] = in_playerInfo.posVec[i];
			rotVec[i] = in_playerInfo.rotVec[i];
		}
	}

	unsigned short PlayerInfo::getPlayerInfoByteSize()
	{
		return sizeof(unsigned short) + PacketProtocol::NICKNAME_MAXSIZE + sizeof(float) * 6;
	}

	GameStartPacket::GameStartPacket(const PlayerInfo& in_playerInfo)
	{
		packetSize = sizeof(unsigned short) + sizeof(PacketID) + PlayerInfo::getPlayerInfoByteSize();
		packetID = PacketID::GAMESTART;
		playerInfo = in_playerInfo;
	}

	// 클라

	//LoginRequestPacket::LoginRequestPacket(const FString& nickname)
	//{
	//	packetSize = sizeof(unsigned short) + sizeof(PacketID) + PacketProtocol::NICKNAME_MAXSIZE;
	//	packetID = PacketID::LOGINREQUEST;
	//	// donghyun : char[] �ʱ�ȭ
	//	LoginNickname[0] = { 0, };
	//	int32 CharArraySize = strlen(TCHAR_TO_UTF8(*nickname)) + 1;
	//	FCStringAnsi::Strncpy(LoginNickname, TCHAR_TO_UTF8(*nickname), CharArraySize);
	//}
}
