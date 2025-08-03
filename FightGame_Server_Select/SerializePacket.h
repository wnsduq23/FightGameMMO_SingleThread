#pragma once
#ifndef  __SERIALIZE_PACKET__
#define  __SERIALIZE_PACKET__

#ifndef _WINSOCKAPI_
#define _WINSOCKAPI_
#endif

#include <tchar.h>
#include <stdio.h>
#include <type_traits>
#include "SystemLog.h"

class SerializePacket
{
public:

	enum en_BUFFER
	{
		eBUFFER_DEFAULT = 2048,
		eBUFFER_MAX = 4096
	};

	SerializePacket()
		: _bufferSize(eBUFFER_DEFAULT), _dataSize(0), _readPos(0), _writePos(0)
	{
		_buffer = new char[_bufferSize];
	}

	SerializePacket(int bufferSize)
		: _bufferSize(bufferSize), _dataSize(0), _readPos(0), _writePos(0)
	{
		_buffer = new char[_bufferSize];
	}

	~SerializePacket()
	{
		delete[] _buffer;
	}

	inline void Clear(void)
	{
		_readPos = 0;
		_writePos = 0;
	}

	int Resize(int bufferSize)
	{
		if (bufferSize > eBUFFER_MAX)
		{
			LOG(L"FightGame", SystemLog::ERROR_LEVEL,
				L"%s[%d]: req %d max %d\n",
				_T(__FUNCTION__), __LINE__, bufferSize, eBUFFER_MAX);

			::wprintf(L"%s[%d]: buffer size %d, req size: %d\n",
				_T(__FUNCTION__), __LINE__, bufferSize, eBUFFER_MAX);

			return -1;
		}

		char* chpNewBuffer = new char[bufferSize];
		memcpy_s(chpNewBuffer, bufferSize, _buffer, _bufferSize);
		delete[] _buffer;

		_buffer = chpNewBuffer;
		_bufferSize = bufferSize;

		return _bufferSize;
	}

	inline int	MoveWritePos(int size)
	{
		if (size < 0) return -1;
		_writePos += size;
		return size;
	}

	inline int	MoveReadPos(int size)
	{
		if (size < 0) return -1;
		_readPos += size;
		return size;
	}

	inline char* GetWritePtr(void) { return &_buffer[_writePos]; }
	inline char* GetReadPtr(void) { return &_buffer[_readPos]; }

	inline int	GetBufferSize(void) { return _bufferSize; }
	inline int	GetDataSize(void) { return _writePos - _readPos; }
	inline bool IsEmpty(void) { return (_writePos == _readPos); }

	inline SerializePacket& operator = (SerializePacket& pSerializePacket)
	{
		*this = pSerializePacket;
		return *this;
	}

	// 템플릿을 사용한 operator << (쓰기)
	template<typename T>
	inline SerializePacket& operator << (T value)
	{
		
		if (_bufferSize - _writePos < sizeof(T))
			Resize((int)(_bufferSize * 1.5f));

		memcpy_s(&_buffer[_writePos],
			_bufferSize - _writePos,
			&value, sizeof(T));

		_writePos += sizeof(T);
		return *this;
	}

	// 템플릿을 사용한 operator >> (읽기)
	template<typename T>
	inline SerializePacket& operator >> (T& value)
	{
		
		if (_writePos - _readPos < sizeof(T))
		{
			LOG(L"FightGame", SystemLog::ERROR_LEVEL,
				L"%s[%d]: used size %d < req size: %llu\n",
				_T(__FUNCTION__), __LINE__, _writePos - _readPos, sizeof(T));

			::wprintf(L"%s[%d]: used size %d < req size: %llu\n",
				_T(__FUNCTION__), __LINE__, _writePos - _readPos, sizeof(T));

			return *this;
		}

		memcpy_s(&value, sizeof(T),
			&_buffer[_readPos], sizeof(T));

		_readPos += sizeof(T);
		return *this;
	}

	inline int	GetData(char* chpDest, int size)
	{
		if (_writePos - _readPos < size)
		{
			LOG(L"FightGame", SystemLog::ERROR_LEVEL,
				L"%s[%d]: used size %d < req size: %llu\n",
				_T(__FUNCTION__), __LINE__, _writePos - _readPos, size);

			::wprintf(L"%s[%d]: used size %d < req size: %llu\n",
				_T(__FUNCTION__), __LINE__, _writePos - _readPos, size);

			return -1;
		}

		memcpy_s(chpDest, size, &_buffer[_readPos], size);

		_readPos += size;
		return size;
	}

	inline int	PeekData(char* chpDest, int size)
	{
		if (_writePos - _readPos < size)
		{
			LOG(L"FightGame", SystemLog::ERROR_LEVEL,
				L"%s[%d]: used size %d < req size: %llu\n",
				_T(__FUNCTION__), __LINE__, _writePos - _readPos, size);

			::wprintf(L"%s[%d]: used size %d < req size: %llu\n",
				_T(__FUNCTION__), __LINE__, _writePos - _readPos, size);

			return -1;
		}

		memcpy_s(chpDest, size, &_buffer[_readPos], size);
		return size;
	}

	inline int CheckData(int size)
	{
		if (_writePos - _readPos < size)
		{
			LOG(L"FightGame", SystemLog::ERROR_LEVEL,
				L"%s[%d]: used size %d < req size: %llu\n",
				_T(__FUNCTION__), __LINE__, _writePos - _readPos, size);

			::wprintf(L"%s[%d]: used size %d < req size: %llu\n",
				_T(__FUNCTION__), __LINE__, _writePos - _readPos, size);

			return -1;
		}
	}

	inline int	PutData(char* src, int srcSize)
	{
		if (_bufferSize - _writePos < srcSize)
			Resize(_bufferSize + (int)(srcSize * 1.5f));

		memcpy_s(&_buffer[_writePos],
			_bufferSize - _writePos,
			src, srcSize);

		_writePos += srcSize;
		return srcSize;
	}

protected:
	int	_bufferSize;
	int	_dataSize;

private:
	char* _buffer;
	int _readPos;
	int _writePos;

};

#endif