#pragma once
#include <new.h>
#include <stdlib.h>
#include <tchar.h>

//#define __OBJECT_POOL_DEBUG__

/* ===============================================

<< __OBJECT_POOL_DEBUG__ ���� >>

1. Node

head�� tail�� ���� �� ������� ���� ���� ����.
head, tail�� �Ҵ�� ������ ������ ���� ���̹Ƿ�
Alloc���� ���� �������� Free���� Ȯ���Ѵ�.

�� ��尡 �ƴ� ���� tail�� ���� node�� �ּҰ��� ����.

1) head

32bit�� ���, ���� 1byte�� Object pool ID��
���� 3byte�� Data �ּ��� ���� 3byte�� ������.
64bit�� ���, ���� 4byte�� 0x0000��,
���� 4byte�� 32bit������ head ���� ������.

Object pool ID�� unsigned char(0~255)�̸�
�� �̻��� Object pool�� ��������� ���� ������� �ʾҴ�.

2) tail

tail�� �����Ǿ� pool �ȿ� ���� ���� ���� node�� �ּҸ�,
�Ҵ�Ǿ� pool �ۿ� ���� ���� head�� ������ ���� ������.

3) ���� ���

Free�� ȣ��Ǹ� �Ʒ� ���׵��� üũ�Ѵ�.
- head�� tail�� ���� ���� ���� ��������
- pool ID�� instance ID�� �������� ���� �� �ִ� ������

�������� ���� ���, Pool�� ������ �ʴ� �������� ���� ��û,
Overflow/Underflow �� �Ѱ��� ���� �Ǵ��Ͽ� ���ܸ� ������.

2. UseCount, Capacity

UseCount�� Capacity�� ����Ͽ� �ʿ��� �� ����� �� �ֵ��� �Ѵ�.
��, Pool �Ҹ� �� UseCount�� Ȯ���Ͽ� �̹�ȯ �����Ͱ� ������ �˸���.
����� �޽����� �ֿܼ� ����ϰ� ������ ���� ������ �����̴�.

==================================================*/

#ifdef __OBJECT_POOL_DEBUG__
#include <stdio.h>
extern unsigned char gObjectPoolID;
#endif


template <class DATA>
class ObjectPool
{
private:
#ifdef __OBJECT_POOL_DEBUG__

	struct stNODE
	{
		stNODE* head;
		DATA data;
		stNODE* tail;
	};

#else
	struct stNODE
	{
		DATA data;
		stNODE* tail = nullptr;
	};
#endif

public:
	template<typename... Types>
	ObjectPool(int iBlockNum, bool bPlacementNew, Types... args);
	virtual	~ObjectPool();

public:
	template<typename... Types>
	inline DATA* Alloc(Types... args);
	inline bool Free(DATA* pData);

private:
	bool _bPlacementNew;
	int _iBlockNum;
	stNODE* _pFreeNode = nullptr; // �Ҵ���� �ʰ� ��� ������ ���

#ifdef __OBJECT_POOL_DEBUG__
public:
	int		GetCapacityCount(void) { return _iCapacity; }
	int		GetUseCount(void) { return _iUseCount; }

private:
	int _iCapacity;
	int _iUseCount;
	unsigned char _iPoolID;
#endif

};

template<class DATA>
template<typename... Types>
ObjectPool<DATA>::ObjectPool(int iBlockNum, bool bPlacementNew, Types... args)
	:_bPlacementNew(bPlacementNew), _iBlockNum(iBlockNum), _pFreeNode(nullptr)
{
#ifdef __OBJECT_POOL_DEBUG__

	_iCapacity = _iBlockNum;
	_iUseCount = 0;
	_iPoolID = gObjectPoolID;
	gObjectPoolID++;
#endif

	if (_iBlockNum <= 0) return;

	if (_bPlacementNew)
	{
		// Alloc �� Data�� �����ڸ� ȣ���ϹǷ� �̶� ȣ���ϸ� �ȵȴ�

		_pFreeNode = (stNODE*)malloc(sizeof(stNODE));
		_pFreeNode->tail = (stNODE*)nullptr;
		for (int i = 1; i < _iBlockNum; i++)
		{
			stNODE* p = (stNODE*)malloc(sizeof(stNODE));
			p->tail = (stNODE*)_pFreeNode;
			_pFreeNode = p;
		}
	}
	else
	{
		// Alloc �� Data�� �����ڸ� ȣ������ �����Ƿ� �̶� ȣ���ؾ� �ȴ�

		_pFreeNode = (stNODE*)malloc(sizeof(stNODE));
		_pFreeNode->tail = (stNODE*)nullptr;
		for (int i = 1; i < _iBlockNum; i++)
		{
			new (&(_pFreeNode->data)) DATA(args...);	//
			stNODE* p = (stNODE*)malloc(sizeof(stNODE));
			p->tail = (stNODE*)_pFreeNode;
			_pFreeNode = p;
		}
		new (&(_pFreeNode->data)) DATA(args...);		//
	}
}

template<class DATA>
ObjectPool<DATA>::~ObjectPool()
{
#ifdef __OBJECT_POOL_DEBUG__

	if (_iUseCount != 0)
	{
		printf("There is Unfree Data!!\n");
		LOG(L"ERROR", SystemLog::ERROR_LEVEL, L"%s[%d]: There is Unfree Data\n", _T(__FUNCTION__), __LINE__);
	}

#endif

	if (_pFreeNode == nullptr)
		return;

	if (_bPlacementNew)
	{
		// Free �� Data�� �Ҹ��ڸ� ȣ���ϹǷ� �̶��� ȣ���ϸ� �ȵȴ�

		while (_pFreeNode->tail != (stNODE*)nullptr)
		{
			stNODE* next = _pFreeNode->tail;
			free(_pFreeNode);
			_pFreeNode = (stNODE*)next;
		}
		free(_pFreeNode);
	}
	else
	{
		// Free �� Data�� �Ҹ��ڸ� ȣ������ �����Ƿ� �̶� ȣ���ؾ� �ȴ�

		while (_pFreeNode->tail != (stNODE*)nullptr)
		{
			stNODE* next = _pFreeNode->tail;
			(_pFreeNode->data).~DATA(); //
			free(_pFreeNode);
			_pFreeNode = (stNODE*)next;
		}
		(_pFreeNode->data).~DATA();		//
		free(_pFreeNode);
	}
}

template<class DATA>
template<typename... Types>
DATA* ObjectPool<DATA>::Alloc(Types... args)
{
	if (_pFreeNode == nullptr)
	{
		// ���� ������ ��� Data�� �����ڸ� ȣ���Ѵ�

		stNODE* pNew = (stNODE*)malloc(sizeof(stNODE));
		new (&(pNew->data)) DATA(args...);

#ifdef __OBJECT_POOL_DEBUG__

		_iCapacity++;
		_iUseCount++;

		stNODE* code = 0;
		code |= (stNODE*)_iPoolID << (3 * 3);
		code |= 0777 & (stNODE*)(&(pNew->data));

		pNew->head = code;
		pNew->tail = code;

#endif

		return &(pNew->data);
	}

	if (_bPlacementNew)
	{
		// _pFreeNode���� ������ �� Data�� �����ڸ� ȣ���Ѵ�

		stNODE* p = _pFreeNode;
		_pFreeNode = (stNODE*)_pFreeNode->tail;
		new (&(p->data)) DATA(args...);				//

#ifdef __OBJECT_POOL_DEBUG__

		_iUseCount++;

		stNODE* code = 0;
		code |= (stNODE*)_iPoolID << (3 * 3);
		code |= 0777 & (stNODE*)(&(p->data));

		p->head = code;
		p->tail = code;
#endif

		return &(p->data);
	}
	else
	{
		// _pFreeNode���� ������ �� Data�� �����ڸ� ȣ������ �ʴ´�

		stNODE* p = _pFreeNode;
		_pFreeNode = (stNODE*)_pFreeNode->tail;

#ifdef __OBJECT_POOL_DEBUG__

		_iUseCount++;

		stNODE* code = 0;
		code |= (stNODE*)_iPoolID << (3 * 3);
		code |= 0777 & (stNODE*)(&(p->data));

		p->head = code;
		p->tail = code;

#endif

		return &(p->data);
	}

	return nullptr;
}

template<class DATA>
bool ObjectPool<DATA>::Free(DATA* pData)
{
	if (_bPlacementNew)
	{
		// Data�� �Ҹ��ڸ� ȣ���� �� _pFreeNode�� push�Ѵ�

#ifdef __OBJECT_POOL_DEBUG__

		_iUseCount--;

		stNODE* code = 0;
		code |= (stNODE*)_iPoolID << (3 * 3);
		code |= 0777 & (stNODE*)pData;


		stNODE* offset = (stNODE*)(&(((stNODE*)nullptr)->data));
		stNODE* pNode = (stNODE*)((stNODE*)pData - offset);

		if (pNode->head != code || pNode->tail != code)
		{
			printf("Error! code %o, head %o, tail %o\n",
				code, pNode->head, pNode->tail);
			LOG(L"ERROR", SystemLog::ERROR_LEVEL,
				L"%s[%d]: code %o, head %o, tail %o\n",
				_T(__FUNCTION__), __LINE__, code, pNode->head, pNode->tail);
		}

		pData->~DATA();
		pNode->tail = (stNODE*)_pFreeNode;
		_pFreeNode = pNode;

#else

		pData->~DATA();								//
		((stNODE*)pData)->tail = (stNODE*)_pFreeNode;
		_pFreeNode = (stNODE*)pData;

#endif
		return true;
	}
	else
	{
		// Data�� �Ҹ��ڸ� ȣ������ �ʰ� _pFreeNode�� push�Ѵ�

#ifdef __OBJECT_POOL_DEBUG__

		_iUseCount--;

		stNODE* code = 0;
		code |= (stNODE*)_iPoolID << (3 * 3);
		code |= 0777 & (stNODE*)pData;

		stNODE* offset = (stNODE*)(&(((stNODE*)nullptr)->data));
		stNODE* pNode = (stNODE*)((stNODE*)pData - offset);

		if (pNode->head != code || pNode->tail != code)
		{
			printf("Error! code %o, head %o, tail %o\n",
				code, pNode->head, pNode->tail);
			LOG(L"ERROR", SystemLog::ERROR_LEVEL,
				L"%s[%d]: code %o, head %o, tail %o\n",
				_T(__FUNCTION__), __LINE__, code, pNode->head, pNode->tail);
			return false;
		}

		pNode->tail = (stNODE*)_pFreeNode;
		_pFreeNode = pNode;

#else

		((stNODE*)pData)->tail = (stNODE*)_pFreeNode;
		_pFreeNode = (stNODE*)pData;

#endif
		return true;
	}
	return false;
}
