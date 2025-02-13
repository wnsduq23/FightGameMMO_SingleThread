﻿#pragma once

#ifndef _WINSOCKAPI_
#define _WINSOCKAPI_
#endif

#include <windows.h>

class SerializePacket
{
public:

	enum en_BUFFER
	{
		eBUFFER_DEFAULT = 1024,
		eBUFFER_MAX = 4096
	};

	SerializePacket();
	SerializePacket(int iBufferSize);
	virtual	~SerializePacket();

	void Clear(void);
	int Resize(int iBufferSize);
	bool IsEmpty(void) { return (_iWritePos == _iReadPos); }
	int	GetBufferSize(void) { return _iBufferSize; }
	int	GetDataSize(void);

	char* GetWritePtr(void) { return &_chpBuffer[_iWritePos]; }
	char* GetReadPtr(void) { return &_chpBuffer[_iReadPos]; }
	int	MoveWritePos(int iSize);
	int	MoveReadPos(int iSize);
	int	PeekData(char* chpDest, int size);
	int CheckData(int size);

	SerializePacket& operator = (SerializePacket& clSrSerializePacket);

	SerializePacket& operator << (float fValue);
	SerializePacket& operator << (double dValue);

	SerializePacket& operator << (char chValue);
	SerializePacket& operator << (unsigned char byValue);

	SerializePacket& operator << (short shValue);
	SerializePacket& operator << (unsigned short wValue);

	SerializePacket& operator << (int iValue);
	SerializePacket& operator << (UINT32 uiValue);
	SerializePacket& operator << (long lValue);
	SerializePacket& operator << (__int64 iValue);

	//--------------------------------------

	SerializePacket& operator >> (float& fValue);
	SerializePacket& operator >> (double& dValue);

	SerializePacket& operator >> (char& chValue);
	SerializePacket& operator >> (BYTE& byValue);

	SerializePacket& operator >> (short& shValue);
	SerializePacket& operator >> (WORD& wValue);

	SerializePacket& operator >> (int& iValue);
	SerializePacket& operator >> (DWORD& dwValue);
	SerializePacket& operator >> (__int64& iValue);

	int	GetData(char* chpDest, int iSize);
	int	PutData(char* chpSrc, int iSrcSize);

protected:
	int	_iBufferSize;
	int	_iDataSize;

private:
	char* _chpBuffer;
	int _iReadPos;
	int _iWritePos;

};
