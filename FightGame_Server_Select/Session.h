#pragma once

#include "RingBuffer.h"
#include "SerializePacket.h"
#include <WinSock2.h>
#include <WS2tcpip.h>
#include "Protocol.h"
#include "NetworkManager.h"
#pragma comment(lib, "ws2_32")


class Session
{
public:
	bool GetSessionAlive() { return _bAlive; }
	void SetSessionAlive() { _bAlive = true; }
	void SetSessionDead();
	inline void Initialize(DWORD ID)
	{
		_ID = ID;
		_bAlive = true;
		_recvRingBuf.ClearBuffer();
		_sendRingBuf.ClearBuffer();
	}

	// Getter methods
	DWORD GetID() const { return _ID; }
	SOCKET GetSocket() const { return _socket; }
	SOCKADDR_IN GetAddr() const { return _addr; }
	RingBuffer& GetSendRingBuf() { return _sendRingBuf; }
	RingBuffer& GetRecvRingBuf() { return _recvRingBuf; }
	SerializePacket& GetRecvSerialPacket() { return _recvSerialPacket; }
	SerializePacket& GetSendSerialPacket() { return _sendSerialPacket; }
	DWORD GetLastRecvTime() const { return _lastRecvTime; }

	// Setter methods
	void SetSocket(SOCKET socket) { _socket = socket; }
	void SetAddr(const SOCKADDR_IN& addr) { _addr = addr; }
	void SetLastRecvTime(DWORD time) { _lastRecvTime = time; }

private:
	DWORD		_ID;
	bool		_bAlive;
	SOCKET		_socket;
	SOCKADDR_IN	_addr;
	RingBuffer	_sendRingBuf;
	RingBuffer	_recvRingBuf;
	SerializePacket _recvSerialPacket;
	SerializePacket _sendSerialPacket;
	DWORD _lastRecvTime;
};
