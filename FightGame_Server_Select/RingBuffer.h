#pragma once
#include <iostream>
#include <WinSock2.h>
#include "SystemLog.h"
#include <tchar.h>

#define DEFAULT_BUF_SIZE 2048 + 1
#define MAX_BUF_SIZE 4096 + 1

/*====================================================================

    <Ring Buffer>

    readPos는 읽고있는 위치이고,
    writePos는 쓸수있는 위치를 나타낸다.
    즉 readPos == writePos는 빈것과 가득찬것을 의미하고
    빈것이 가득찬것 (readPos + 1)%_bufferSize == writePos 이다.

======================================================================*/
class RingBuffer
{
public:
    inline RingBuffer(void)
        : _initBufferSize(DEFAULT_BUF_SIZE), _bufferSize(DEFAULT_BUF_SIZE), _freeSize(DEFAULT_BUF_SIZE - 1)
    {
        _buffer = new char[_bufferSize];
    }

    inline RingBuffer(int bufferSize)
        : _initBufferSize(bufferSize), _bufferSize(bufferSize), _freeSize(bufferSize - 1)
    {
        _buffer = new char[_bufferSize];
    }

    inline ~RingBuffer(void)
    {
        delete[] _buffer;
    }


    inline void ClearBuffer(void)
    {
        _readPos = 0;
        _writePos = 0;
        _useSize = 0;
        _bufferSize = _initBufferSize;
        _freeSize = _bufferSize - 1;
    }

    inline int Peek(char* chpDest, int size)
    {
        if (size > _useSize)
        {
            LOG(L"FightGame", SystemLog::ERROR_LEVEL,
                L"%s[%d]: used size %d < req size %d\n",
                _T(__FUNCTION__), __LINE__, _useSize, size);

            ::wprintf(L"%s[%d]: used size %d < req size %d\n",
                _T(__FUNCTION__), __LINE__, _useSize, size);

            return -1;
        }

        int directDequeueSize = DirectDequeueSize();
        if (size <= directDequeueSize)
        {
            memcpy_s(chpDest, size, &_buffer[(_readPos + 1) % _bufferSize], size);
        }
        else
        {
            int size1 = directDequeueSize;
            int size2 = size - size1;
            memcpy_s(chpDest, size1, &_buffer[(_readPos + 1) % _bufferSize], size1);
            memcpy_s(&chpDest[size1], size2, _buffer, size2);
        }

        return size;
    }

    inline int Enqueue(char* chpData, int size)
    {
        if (size > MAX_BUF_SIZE)
        {
            LOG(L"FightGame", SystemLog::ERROR_LEVEL,
                L"%s[%d] req %d, max %d",
                _T(__FUNCTION__), __LINE__, size, MAX_BUF_SIZE);

            ::wprintf(L"%s[%d] req %d, max %d",
                _T(__FUNCTION__), __LINE__, size, MAX_BUF_SIZE);

            return -1;
        }
        else if (size > _freeSize)
        {
            if (!Resize(_bufferSize + (int)(size * 1.5f)))
            {
                LOG(L"FightGame", SystemLog::ERROR_LEVEL,
                    L"%s[%d] Fail to Resize",
                    _T(__FUNCTION__), __LINE__);

                ::wprintf(L"%s[%d] Fail to Resize",
                    _T(__FUNCTION__), __LINE__);

                return -1;
            }
        }

        int directEnqueueSize = DirectEnqueueSize();
        if (size <= directEnqueueSize)
        {
            memcpy_s(&_buffer[(_writePos + 1) % _bufferSize], size, chpData, size);
        }
        else
        {
            int size1 = directEnqueueSize;
            int size2 = size - size1;
            memcpy_s(&_buffer[(_writePos + 1) % _bufferSize], size1, chpData, size1);
            memcpy_s(_buffer, size2, &chpData[size1], size2);
        }

        _useSize += size;
        _freeSize -= size;
        _writePos = (_writePos + size) % _bufferSize;

        return size;
    }

    inline int Dequeue(char* chpData, int size)
    {
        if (size > _useSize)
        {
            LOG(L"FightGame", SystemLog::ERROR_LEVEL,
                L"%s[%d]: used size %d, req size %d\n",
                _T(__FUNCTION__), __LINE__, _useSize, size);

            ::wprintf(L"%s[%d]: used size %d, req size %d\n",
                _T(__FUNCTION__), __LINE__, _useSize, size);

            return -1;
        }

        int directDequeueSize = DirectDequeueSize();
        if (size <= directDequeueSize)
        {
            memcpy_s(chpData, size, &_buffer[(_readPos + 1) % _bufferSize], size);
        }
        else
        {
            int size1 = directDequeueSize;
            int size2 = size - size1;
            memcpy_s(chpData, size1, &_buffer[(_readPos + 1) % _bufferSize], size1);
            memcpy_s(&chpData[size1], size2, _buffer, size2);
        }

        _useSize -= size;
        _freeSize += size;
        _readPos = (_readPos + size) % _bufferSize;

        return size;
    }

    inline int MoveReadPos(int size)
    {
        if (size > _useSize)
        {
            LOG(L"FightGame", SystemLog::ERROR_LEVEL,
                L"%s[%d]: used size %d, req size %d\n",
                _T(__FUNCTION__), __LINE__, _useSize, size);

            ::wprintf(L"%s[%d]: used size %d, req size %d\n",
                _T(__FUNCTION__), __LINE__, _useSize, size);

            return -1;
        }

        _readPos = (_readPos + size) % _bufferSize;
        _useSize -= size;
        _freeSize += size;

        return size;
    }

    inline int MoveWritePos(int size)
    {
        if (size > MAX_BUF_SIZE)
        {
            LOG(L"FightGame", SystemLog::ERROR_LEVEL,
                L"%s[%d] req %d, max %d",
                _T(__FUNCTION__), __LINE__, size, MAX_BUF_SIZE);
            ::wprintf(L"%s[%d] req %d, max %d",
                _T(__FUNCTION__), __LINE__, size, MAX_BUF_SIZE);

            return -1;
        }
        else if (size > _freeSize)
        {
            if (!Resize(_bufferSize + (int)(size * 1.5f)))
            {
                LOG(L"FightGame", SystemLog::ERROR_LEVEL,
                    L"%s[%d] Fail to Resize",
                    _T(__FUNCTION__), __LINE__);
                ::wprintf(L"%s[%d] Fail to Resize",
                    _T(__FUNCTION__), __LINE__);

                return -1;
            }
        }

        _writePos = (_writePos + size) % _bufferSize;
        _useSize += size;
        _freeSize -= size;

        return size;
    }

    inline int GetBufferSize(void) { return _bufferSize; }
    inline int GetUseSize(void) { return _useSize; }
    inline int GetFreeSize(void) { return _freeSize; }
    inline char* GetReadPtr(void) { return &_buffer[(_readPos + 1) % _bufferSize]; }
    inline char* GetWritePtr(void) { return &_buffer[(_writePos + 1) % _bufferSize]; }

    inline int DirectEnqueueSize(void)
    {
        if (_writePos >= _readPos)
            return _bufferSize - _writePos - 1;
        else
            return _readPos - _writePos - 1;
    }

    inline int DirectDequeueSize(void)
	{
        if (_writePos >= _readPos)
            return _writePos - _readPos;
        else
            return _bufferSize - _readPos - 1;
    }

    // For Debug
    inline void GetBufferDataForDebug()
    {
        ::wprintf(L"\n");
        ::wprintf(L"Buffer Size: %d\n", _bufferSize);
        ::wprintf(L"Read: %d\n", _readPos);
        ::wprintf(L"Write: %d\n", _writePos);
        ::wprintf(L"Real Use Size: %d\n", _useSize);
        ::wprintf(L"Real Free Size: %d\n", _freeSize);
        ::wprintf(L"Direct Dequeue Size: %d\n", DirectDequeueSize());
        ::wprintf(L"Direct Enqueue Size: %d\n", DirectEnqueueSize());
        ::wprintf(L"\n");
    }

    bool Resize(int size)
    {
        LOG(L"FightGame", SystemLog::DEBUG_LEVEL,
            L"%s[%d] Resize\n", _T(__FUNCTION__), __LINE__);

        ::wprintf(L"%s[%d] Resize\n", _T(__FUNCTION__), __LINE__);

        if (size > MAX_BUF_SIZE)
        {
            LOG(L"FightGame", SystemLog::ERROR_LEVEL,
                L"%s[%d]: req %d, max %d\n",
                _T(__FUNCTION__), __LINE__, size, MAX_BUF_SIZE);

            ::wprintf(L"%s[%d]: req %d, max %d\n",
                _T(__FUNCTION__), __LINE__, size, MAX_BUF_SIZE);

            return false;
        }

        if (size < _useSize)
        {
            LOG(L"FightGame", SystemLog::ERROR_LEVEL,
                L"%s[%d]: used size %d, req size %d\n",
                _T(__FUNCTION__), __LINE__, _useSize, size);

            ::wprintf(L"%s[%d]: used size %d, req size %d\n",
                _T(__FUNCTION__), __LINE__, _useSize, size);

            return false;
        }

        char* newBuffer = new char[size];
        memset(newBuffer, '\0', size);

        if (_writePos > _readPos)
        {
            memcpy_s(&newBuffer[1], size - 1, &_buffer[(_readPos + 1) % _bufferSize], _useSize);
        }
        else if (_writePos <= _readPos)
        {
            int size1 = _bufferSize - _readPos - 1;
            int size2 = _writePos + 1;
            memcpy_s(&newBuffer[1], size - 1, &_buffer[(_readPos + 1) % _bufferSize], size1);
            memcpy_s(&newBuffer[size1 + 1], size - (size1 + 1), &_buffer[0], size2);
        }

        delete[] _buffer;
        _buffer = newBuffer;
        _readPos = 0;
        _writePos = _useSize;
        _bufferSize = size;
        _freeSize = _bufferSize - _useSize - 1;

        return true;
    }

private:
    char* _buffer;
    int _readPos = 0;
    int _writePos = 0;
    int _bufferSize;
    int _useSize = 0;
    int _freeSize = 0;

    int _initBufferSize;

};