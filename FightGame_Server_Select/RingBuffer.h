#pragma once
#define DEFAULT_BUF_SIZE (32767 + 1)
#define MAX_BUF_SIZE (65535 + 1)

/*====================================================================

    <Ring Buffer>

    readPos�� ����ִ� ������,
    writePos�� ���������� ���� ������ ����Ų��
    ���� readPos == writePos�� ���۰� ��������� �ǹ��ϰ�
    ���۰� á�� ���� (readPos + 1)%_bufferSize == writePos �� �ȴ�.

======================================================================*/
class RingBuffer
{
public:
    RingBuffer(void);
    RingBuffer(int iBufferSize);
    ~RingBuffer(void);

    int GetBufferSize(void);
    int GetUseSize(void);
    int GetFreeSize(void);
    int DirectEnqueueSize(void);
    int DirectDequeueSize(void);

    int Enqueue(char* chpData, int iSize);
    int Dequeue(char* chpData, int iSize);
    int Peek(char* chpDest, int iSize);
    void ClearBuffer(void);
    bool Resize(int iSize);

    int MoveReadPos(int iSize);
    int MoveWritePos(int iSize);
    char* GetReadPtr(void) { return &_chpBuffer[(_iReadPos + 1) % _iBufferSize]; }
    char* GetWritePtr(void) { return &_chpBuffer[(_iWritePos + 1) % _iBufferSize]; }

    // For Debug
    void GetBufferDataForDebug();

private:
    char* _chpBuffer;
    int _iBufferSize;

    int _iUsedSize = 0;
    int _iFreeSize = 0;

    int _iReadPos = 0;
    int _iWritePos = 0;
};

