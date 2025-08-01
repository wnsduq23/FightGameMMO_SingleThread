#pragma once
#include <new.h>
#include <stdlib.h>
#include <tchar.h>
#include "SystemLog.h"
#include "main.h"

//#define __OBJECT_POOL_DEBUG__

/* ===============================================

<< __OBJECT_POOL_DEBUG__ 설명 >>

1. Node

head와 tail은 각각 노드의 앞뒤를 나타내는 포인터이다.
head, tail은 할당된 노드의 앞뒤를 나타내는 것이므로
Alloc에서 할당된 노드의 앞뒤를 Free에서 확인한다.

이 노드가 아닌 경우 tail은 다음 node의 포인터를 나타낸다.

1) head

32bit의 경우, 앞 1byte는 Object pool ID이고
뒤 3byte는 Data 주소의 뒤 3byte를 나타낸다.
64bit의 경우, 앞 4byte는 0x0000이고,
뒤 4byte는 32bit와 같은 head 구조를 나타낸다.

Object pool ID는 unsigned char(0~255)이므로
이 이상의 Object pool은 생성할 수 없다.

2) tail

tail은 할당되지 않은 pool 안의 다음 노드의 포인터이고,
할당된 pool 밖의 노드의 head와 같은 구조를 나타낸다.

3) 할당 검증

Free가 호출되면 아래 사항들을 체크한다.
- head와 tail이 같은 구조를 나타내는지
- pool ID와 instance ID가 일치하는지 등

일치하지 않는 경우, Pool에서 할당되지 않은 노드에 대한 해제 요청,
Overflow/Underflow 등이 발생했음을 의미하고 크래시를 발생시킨다.

2. UseCount, Capacity

UseCount와 Capacity를 사용해서 필요한 노드의 개수를 파악한다.
즉, Pool 해제 시 UseCount를 확인해서 이미 해제된 노드인지 알 수 있다.
이런 메시지를 간단하고 명확하게 처리해서 디버깅을 용이하게 한다.

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
		size_t head;
		DATA data;
		size_t tail;
	};

#else
	struct stNODE
	{
		DATA data;
		size_t tail = nullptr;
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
	stNODE* _pFreeNode = nullptr; // 할당되지 않은 노드의 리스트

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
		// Alloc 시 Data의 생성자를 호출하므로 여기서 호출하지 않는다

		_pFreeNode = (stNODE*)malloc(sizeof(stNODE));
		_pFreeNode->tail = (size_t)nullptr;
		for (int i = 1; i < _iBlockNum; i++)
		{
			stNODE* p = (stNODE*)malloc(sizeof(stNODE));
			p->tail = (size_t)_pFreeNode;
			_pFreeNode = p;
		}
	}
	else
	{
		// Alloc 시 Data의 생성자를 호출하지 않으므로 여기서 호출해야 한다

		_pFreeNode = (stNODE*)malloc(sizeof(stNODE));
		_pFreeNode->tail = (size_t)nullptr;
		for (int i = 1; i < _iBlockNum; i++)
		{
			new (&(_pFreeNode->data)) DATA(args...);	//
			stNODE* p = (stNODE*)malloc(sizeof(stNODE));
			p->tail = (size_t)_pFreeNode;
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
		LOG(L"FightGame", SystemLog::ERROR_LEVEL,
			L"%s[%d]: There is Unfree Data\n",
			_T(__FUNCTION__), __LINE__);

		::wprintf(L"ERROR", SystemLog::ERROR_LEVEL,
			L"%s[%d]: There is Unfree Data\n",
			_T(__FUNCTION__), __LINE__);

		g_dump.Crash();
	}

#endif

	if (_pFreeNode == nullptr)
		return;

	if (_bPlacementNew)
	{
		// Free 시 Data의 소멸자를 호출하므로 여기서 호출하지 않는다

		while (_pFreeNode->tail != (size_t)nullptr)
		{
			size_t next = _pFreeNode->tail;
			free(_pFreeNode);
			_pFreeNode = (stNODE*)next;
		}
		free(_pFreeNode);
	}
	else
	{
		// Free 시 Data의 소멸자를 호출하지 않으므로 여기서 호출해야 한다

		while (_pFreeNode->tail != (size_t)nullptr)
		{
			size_t next = _pFreeNode->tail;
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
inline DATA* ObjectPool<DATA>::Alloc(Types... args)
{
	if (_pFreeNode == nullptr)
	{
		// 여유 노드가 없으면 Data의 생성자를 호출한다

		stNODE* pNew = (stNODE*)malloc(sizeof(stNODE));
		new (&(pNew->data)) DATA(args...);

#ifdef __OBJECT_POOL_DEBUG__

		_iCapacity++;
		_iUseCount++;

		size_t code = 0;
		code |= (size_t)_iPoolID << (3 * 3);
		code |= 0777 & (size_t)(&(pNew->data));

		pNew->head = code;
		pNew->tail = code;

#endif

		return &(pNew->data);
	}

	if (_bPlacementNew)
	{
		// _pFreeNode에서 노드를 가져와서 Data의 생성자를 호출한다

		stNODE* p = _pFreeNode;
		_pFreeNode = (stNODE*)_pFreeNode->tail;
		new (&(p->data)) DATA(args...);				//

#ifdef __OBJECT_POOL_DEBUG__

		_iUseCount++;

		size_t code = 0;
		code |= (size_t)_iPoolID << (3 * 3);
		code |= 0777 & (size_t)(&(p->data));

		p->head = code;
		p->tail = code;
#endif

		return &(p->data);
	}
	else
	{
		// _pFreeNode에서 노드를 가져와서 Data의 생성자를 호출하지 않는다

		stNODE* p = _pFreeNode;
		_pFreeNode = (stNODE*)_pFreeNode->tail;

#ifdef __OBJECT_POOL_DEBUG__

		_iUseCount++;

		size_t code = 0;
		code |= (size_t)_iPoolID << (3 * 3);
		code |= 0777 & (size_t)(&(p->data));

		p->head = code;
		p->tail = code;

#endif

		return &(p->data);
	}

	return nullptr;
}

template<class DATA>
inline bool ObjectPool<DATA>::Free(DATA* pData)
{
	if (_bPlacementNew)
	{
		// Data의 소멸자를 호출하고 _pFreeNode에 push한다

#ifdef __OBJECT_POOL_DEBUG__

		_iUseCount--;

		size_t code = 0;
		code |= (size_t)_iPoolID << (3 * 3);
		code |= 0777 & (size_t)pData;


		size_t offset = (size_t)(&(((stNODE*)nullptr)->data));
		stNODE* pNode = (stNODE*)((size_t)pData - offset);

		if (pNode->head != code || pNode->tail != code)
		{
			LOG(L"FightGame", SystemLog::ERROR_LEVEL,
				L"%s[%d]: Code is Diffrent. code %o, head %o, tail %o\n",
				_T(__FUNCTION__), __LINE__, code, pNode->head, pNode->tail);

			::wprintf(L"%s[%d]: Code is Diffrent. code %o, head %o, tail %o\n",
				_T(__FUNCTION__), __LINE__, code, pNode->head, pNode->tail);

			g_dump.Crash();
		}

		pData->~DATA();
		pNode->tail = (size_t)_pFreeNode;
		_pFreeNode = pNode;

#else

		pData->~DATA();								//
		((stNODE*)pData)->tail = (size_t)_pFreeNode;
		_pFreeNode = (stNODE*)pData;

#endif
		return true;
	}
	else
	{
		// Data의 소멸자를 호출하지 않고 _pFreeNode에 push한다

#ifdef __OBJECT_POOL_DEBUG__

		_iUseCount--;

		size_t code = 0;
		code |= (size_t)_iPoolID << (3 * 3);
		code |= 0777 & (size_t)pData;

		size_t offset = (size_t)(&(((stNODE*)nullptr)->data));
		stNODE* pNode = (stNODE*)((size_t)pData - offset);

		if (pNode->head != code || pNode->tail != code)
		{
			printf("Error! code %o, head %o, tail %o\n",
				code, pNode->head, pNode->tail);
			LOG(L"ERROR", SystemLog::ERROR_LEVEL,
				L"%s[%d]: code %o, head %o, tail %o\n",
				_T(__FUNCTION__), __LINE__, code, pNode->head, pNode->tail);
			return false;
		}

		pNode->tail = (size_t)_pFreeNode;
		_pFreeNode = pNode;

#else

		((stNODE*)pData)->tail = (size_t)_pFreeNode;
		_pFreeNode = (stNODE*)pData;

#endif
		return true;
	}
	return false;
}
