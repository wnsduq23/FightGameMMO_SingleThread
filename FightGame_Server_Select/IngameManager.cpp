#include "IngameManager.h"
#include "Sector.h"
#include "Profiler.h"
#include <Windows.h>
#include "SetSCPacket.h"
#include <algorithm>
#include "main.h"


IngameManager::IngameManager()
{
	_pPlayerPool = new ObjectPool<Player>(dfSESSION_MAX, true);
	for (int y = 0; y < dfSECTOR_CNT_Y; y++)
	{
		for (int x = 0; x < dfSECTOR_CNT_X; x++)
		{
			_Sectors[y][x].InitializeSector(x, y);
		}
	}
	SetSectorsAroundInfo();
}

IngameManager::~IngameManager()
{
	for (int i = 0; i < dfSESSION_MAX; i++)
	{
		if (_Players[i] != nullptr)
			_pPlayerPool->Free(_Players[i]);
	}

	delete _pPlayerPool;
}

// [1] 고정된 Update 간격(초 단위) 40ms == 0.04초 = 25fps
static const DWORD FIXED_STEP = 40; 

bool IngameManager::FixedUpdate()
{
    // static 변수를 사용하여 함수 호출 간 누적 정보 보관
    static DWORD oldTime = timeGetTime();    // ms 단위
    static DWORD accumulated = 0;         // 누적 시간(초 단위)

    DWORD curTime = timeGetTime();
    DWORD deltaTime = curTime - oldTime; // ms → 초
    oldTime = curTime;

    // 누적 시간에 이번 프레임 소요 시간 추가
    accumulated += deltaTime;

    bool didUpdate = false;

    // accumulated가 FIXED_STEP을 넘을 때마다 여러 번 Update 실행
    while (accumulated >= FIXED_STEP)
    {
		// 플레이어 이동 처리
        PlayerActionProc();
		// 더 확장성을 고려한다면 여기에 몬스터, 이벤트 등등이 들어올 듯

        accumulated -= FIXED_STEP;
        didUpdate = true;
    }

    return didUpdate;
}
void IngameManager::GameContentsModule()
{
	PRO_BEGIN(L"Content");
	// 프레임 설정 
	if (!FixedUpdate())
    {
        return;
    }
	PRO_END(L"Content");

	PRO_SAVE(L"output.txt");
}

/*===================================================================
*						About Sector
===================================================================*/

// 섹터 구역 나누기 : 플레이어 좌표 / 한 섹터 사이즈 = 플레이어 섹터 인덱스
void IngameManager::SetPlayerSector(Player* pPlayer)
{
	int x = (pPlayer->GetX() / dfSECTOR_SIZE_X) + 2;
	int y = (pPlayer->GetY() / dfSECTOR_SIZE_Y) + 2;
	_Sectors[y][x]._players.push_back(pPlayer);
	pPlayer->SetSector(&_Sectors[y][x]);
}

void IngameManager::SetSectorsAroundInfo()
{
	for (int y = 2; y < dfSECTOR_CNT_Y - 2; y++)
	{
		for (int x = 2; x < dfSECTOR_CNT_X - 2; x++)
		{
			Sector* pSector = &_Sectors[y][x];

			pSector->_around[dfMOVE_DIR_LL] = &_Sectors[y][x - 1];
			pSector->_around[dfMOVE_DIR_LU] = &_Sectors[y - 1][x - 1];
			pSector->_around[dfMOVE_DIR_UU] = &_Sectors[y - 1][x];
			pSector->_around[dfMOVE_DIR_RU] = &_Sectors[y - 1][x + 1];
			pSector->_around[dfMOVE_DIR_RR] = &_Sectors[y][x + 1];
			pSector->_around[dfMOVE_DIR_RD] = &_Sectors[y + 1][x + 1];
			pSector->_around[dfMOVE_DIR_DD] = &_Sectors[y + 1][x];
			pSector->_around[dfMOVE_DIR_LD] = &_Sectors[y + 1][x - 1];
			pSector->_around[dfMOVE_DIR_INPLACE] = &_Sectors[y][x];
			
			// dfMOVE_DIR_LL

			pSector->_llNew[0] = &_Sectors[y - 1][x - 2];
			pSector->_llNew[1] = &_Sectors[y][x - 2];
			pSector->_llNew[2] = &_Sectors[y + 1][x - 2];

			pSector->_llOld[0] = &_Sectors[y - 1][x + 1];
			pSector->_llOld[1] = &_Sectors[y][x + 1];
			pSector->_llOld[2] = &_Sectors[y + 1][x + 1];

			// dfMOVE_DIR_LU

			pSector->_luNew[0] = &_Sectors[y - 2][x];
			pSector->_luNew[1] = &_Sectors[y - 2][x - 1];
			pSector->_luNew[2] = &_Sectors[y - 2][x - 2];
			pSector->_luNew[3] = &_Sectors[y - 1][x - 2];
			pSector->_luNew[4] = &_Sectors[y][x - 2];

			pSector->_luOld[0] = &_Sectors[y + 1][x];
			pSector->_luOld[1] = &_Sectors[y + 1][x - 1];
			pSector->_luOld[2] = &_Sectors[y + 1][x + 1];
			pSector->_luOld[3] = &_Sectors[y - 1][x + 1];
			pSector->_luOld[4] = &_Sectors[y][x + 1];

			// dfMOVE_DIR_UU

			pSector->_uuNew[0] = &_Sectors[y - 2][x - 1];
			pSector->_uuNew[1] = &_Sectors[y - 2][x];
			pSector->_uuNew[2] = &_Sectors[y - 2][x + 1];

			pSector->_uuOld[0] = &_Sectors[y + 1][x - 1];
			pSector->_uuOld[1] = &_Sectors[y + 1][x];
			pSector->_uuOld[2] = &_Sectors[y + 1][x + 1];

			// dfMOVE_DIR_RU

			pSector->_ruNew[0] = &_Sectors[y - 2][x];
			pSector->_ruNew[1] = &_Sectors[y - 2][x + 1];
			pSector->_ruNew[2] = &_Sectors[y - 2][x + 2];
			pSector->_ruNew[3] = &_Sectors[y - 1][x + 2];
			pSector->_ruNew[4] = &_Sectors[y][x + 2];

			pSector->_ruOld[0] = &_Sectors[y][x - 1];
			pSector->_ruOld[1] = &_Sectors[y - 1][x - 1];
			pSector->_ruOld[2] = &_Sectors[y + 1][x - 1];
			pSector->_ruOld[3] = &_Sectors[y + 1][x + 1];
			pSector->_ruOld[4] = &_Sectors[y + 1][x];

			// dfMOVE_DIR_RR

			pSector->_rrNew[0] = &_Sectors[y - 1][x + 2];
			pSector->_rrNew[1] = &_Sectors[y][x + 2];
			pSector->_rrNew[2] = &_Sectors[y + 1][x + 2];

			pSector->_rrOld[0] = &_Sectors[y - 1][x - 1];
			pSector->_rrOld[1] = &_Sectors[y][x - 1];
			pSector->_rrOld[2] = &_Sectors[y + 1][x - 1];

			// dfMOVE_DIR_RD

			pSector->_rdNew[0] = &_Sectors[y + 2][x];
			pSector->_rdNew[1] = &_Sectors[y + 2][x + 1];
			pSector->_rdNew[2] = &_Sectors[y + 2][x + 2];
			pSector->_rdNew[3] = &_Sectors[y + 1][x + 2];
			pSector->_rdNew[4] = &_Sectors[y][x + 2];

			pSector->_rdOld[0] = &_Sectors[y - 1][x];
			pSector->_rdOld[1] = &_Sectors[y - 1][x + 1];
			pSector->_rdOld[2] = &_Sectors[y - 1][x - 1];
			pSector->_rdOld[3] = &_Sectors[y + 1][x - 1];
			pSector->_rdOld[4] = &_Sectors[y][x - 1];

			// dfMOVE_DIR_DD

			pSector->_ddNew[0] = &_Sectors[y + 2][x - 1];
			pSector->_ddNew[1] = &_Sectors[y + 2][x];
			pSector->_ddNew[2] = &_Sectors[y + 2][x + 1];

			pSector->_ddOld[0] = &_Sectors[y - 1][x - 1];
			pSector->_ddOld[1] = &_Sectors[y - 1][x];
			pSector->_ddOld[2] = &_Sectors[y - 1][x + 1];

			// dfMOVE_DIR_LD

			pSector->_ldNew[0] = &_Sectors[y + 2][x];
			pSector->_ldNew[1] = &_Sectors[y + 2][x - 1];
			pSector->_ldNew[2] = &_Sectors[y + 2][x - 2];
			pSector->_ldNew[3] = &_Sectors[y + 1][x - 2];
			pSector->_ldNew[4] = &_Sectors[y][x - 2];

			pSector->_ldOld[0] = &_Sectors[y][x + 1];
			pSector->_ldOld[1] = &_Sectors[y + 1][x + 1];
			pSector->_ldOld[2] = &_Sectors[y - 1][x + 1];
			pSector->_ldOld[3] = &_Sectors[y - 1][x - 1];
			pSector->_ldOld[4] = &_Sectors[y - 1][x];
		}
	}
}

void IngameManager::UpdateSector(Player* pPlayer)
{
	PRO_BEGIN(L"Content: Update Sector");
	short direction;
	if (pPlayer->GetX() < pPlayer->GetSector()->_xPosMin &&
		pPlayer->GetY() > pPlayer->GetSector()->_yPosMax)
		direction = dfMOVE_DIR_LD;
	else if (pPlayer->GetX() < pPlayer->GetSector()->_xPosMin)
		direction = dfMOVE_DIR_LL;
	else if (pPlayer->GetY() > pPlayer->GetSector()->_yPosMax)
		direction = dfMOVE_DIR_DD;
	else if (pPlayer->GetX() > pPlayer->GetSector()->_xPosMax &&
		pPlayer->GetY() > pPlayer->GetSector()->_yPosMax)
		direction = dfMOVE_DIR_RD;
	else if (pPlayer->GetX() > pPlayer->GetSector()->_xPosMax)
		direction = dfMOVE_DIR_RR;
	else if (pPlayer->GetX() > pPlayer->GetSector()->_xPosMax &&
		pPlayer->GetY() < pPlayer->GetSector()->_yPosMin)
		direction = dfMOVE_DIR_RU;
	else if (pPlayer->GetY() < pPlayer->GetSector()->_yPosMin)
		direction = dfMOVE_DIR_UU;
	else if (pPlayer->GetX() < pPlayer->GetSector()->_xPosMin &&
		pPlayer->GetY() < pPlayer->GetSector()->_yPosMin)
		direction = dfMOVE_DIR_LU;
	else
		return;

	vector<Player*>::iterator iter = pPlayer->GetSector()->_players.begin();
	for (; iter < pPlayer->GetSector()->_players.end(); iter++)
	{
		if (pPlayer == (*iter))
		{
			pPlayer->GetSector()->_players.erase(iter); // check
			break;
		}
	}

	// Get Around Sector Data =======================================

	int updated_sector_cnt = _sectorCnt[direction];
	Sector** new_updated_sector = pPlayer->GetSector()->_new[direction];
	Sector** old_updated_sector = pPlayer->GetSector()->_old[direction];
	Sector* new_around_sector = pPlayer->GetSector()->_around[direction];

	// Send Data About My Player to Other Player ==============================================

	pPlayer->GetSession()->_sendSerialPacket.Clear();
	int createMeToOtherRet = SetSCPacket_CREATE_OTHER_CHARACTER(&pPlayer->GetSession()->_sendSerialPacket,
		pPlayer->GetID(), pPlayer->GetHeadDirection(), pPlayer->GetX(), pPlayer->GetY(), pPlayer->GetHp());
	//새로 변경된 섹터 안에 있는 플레이어들에게 내 정보 send
	for (int i = 0; i < updated_sector_cnt; i++)
		SendPacketOneSector(pPlayer->GetSession()->_sendSerialPacket.GetReadPtr(), createMeToOtherRet, new_updated_sector[i]);

	// 섹터 안으로 들어오는 동작 send
	pPlayer->GetSession()->_sendSerialPacket.Clear();
	int moveMeToOtherRet = SetSCPacket_MOVE_START(&pPlayer->GetSession()->_sendSerialPacket,
		pPlayer->GetID(), pPlayer->GetMoveDirection(), pPlayer->GetX(), pPlayer->GetY());
	for (int i = 0; i < updated_sector_cnt; i++)
		SendPacketOneSector(pPlayer->GetSession()->_sendSerialPacket.GetReadPtr(), moveMeToOtherRet, new_updated_sector[i]);

	// 이동 후 old sector에 있는 플레이어들에게 내 정보 삭제 send
	pPlayer->GetSession()->_sendSerialPacket.Clear();
	int deleteMeToOtherRet = SetSCPacket_DELETE_CHARACTER(&pPlayer->GetSession()->_sendSerialPacket, pPlayer->GetID());
	for (int i = 0; i < updated_sector_cnt; i++)
		SendPacketOneSector(pPlayer->GetSession()->_sendSerialPacket.GetReadPtr(), deleteMeToOtherRet, old_updated_sector[i]);

	// Send Data About Other Player to My Player ==============================================

	for (int i = 0; i < updated_sector_cnt; i++)
	{
		vector<Player*>::iterator iter = new_updated_sector[i]->_players.begin();
		for (; iter < new_updated_sector[i]->_players.end(); iter++)
		{
			pPlayer->GetSession()->_sendSerialPacket.Clear();
			int createOtherRet = SetSCPacket_CREATE_OTHER_CHARACTER(&pPlayer->GetSession()->_sendSerialPacket,
				(*iter)->GetID(), (*iter)->GetHeadDirection(), (*iter)->GetX(), (*iter)->GetY(), (*iter)->GetHp());
			NetworkManager::GetInstance().SendPacketUnicast(pPlayer->GetSession()->_sendSerialPacket.GetReadPtr(), createOtherRet, pPlayer->GetSession());

			if ((*iter)->GetStateMoving()) // check
			{
				pPlayer->GetSession()->_sendSerialPacket.Clear();
				int MoveOtherRet = SetSCPacket_MOVE_START(&pPlayer->GetSession()->_sendSerialPacket,
					(*iter)->GetID(), (*iter)->GetMoveDirection(), (*iter)->GetX(), (*iter)->GetY());
				NetworkManager::GetInstance().SendPacketUnicast(pPlayer->GetSession()->_sendSerialPacket.GetReadPtr(), MoveOtherRet, pPlayer->GetSession());
			}
		}
	}

	for (int i = 0; i < updated_sector_cnt; i++)
	{
		vector<Player*>::iterator iter = old_updated_sector[i]->_players.begin();
		for (; iter < old_updated_sector[i]->_players.end(); iter++)
		{
			pPlayer->GetSession()->_sendSerialPacket.Clear();
			int deleteOtherRet = SetSCPacket_DELETE_CHARACTER(&pPlayer->GetSession()->_sendSerialPacket, (*iter)->GetID());
			NetworkManager::GetInstance().SendPacketUnicast(pPlayer->GetSession()->_sendSerialPacket.GetReadPtr(), deleteOtherRet, pPlayer->GetSession());
		}
	}

	pPlayer->SetSector(new_around_sector);
	new_around_sector->_players.push_back(pPlayer); // check 

	PRO_END(L"Content: Update Sector");
}

// 한 섹터 안에 있는 모든 플레이어들에게 메시지 링버퍼에 넣음
void IngameManager::SendPacketOneSector(char* msg, int size, Sector* sector, Session* pExpSession)
{
	if (pExpSession == nullptr)
	{
		vector<Player*>::iterator playerIter = sector->_players.begin();
		for (; playerIter < sector->_players.end(); playerIter++)
		{
			if ((*playerIter) == nullptr)
			{
				LOG(L"FightGame", SystemLog::ERROR_LEVEL,
					L"%s[%d] Player in sector[%d][%d] is nullptr\n", 
					_T(__FUNCTION__), __LINE__, sector->_xIndex, sector->_yIndex);
					
				::wprintf(L"%s[%d] Player in sector[%d][%d] is nullptr\n",
					_T(__FUNCTION__), __LINE__, sector->_xIndex, sector->_yIndex);
				g_dump.Crash();
				return;
			}

			int enqueueRet = (*playerIter)->GetSession()->_sendRingBuf.Enqueue(msg, size);
			if (enqueueRet != size)
			{
				 LOG(L"FightGame", SystemLog::ERROR_LEVEL,
					L"%s[%d] Session %d - sendRBuf Enqueue Error (req - %d, ret - %d)\n",
					_T(__FUNCTION__), __LINE__, (*playerIter)->GetSession()->_ID, size, enqueueRet);

				::wprintf(L"%s[%d] Session %d - sendRBuf Enqueue Error (req - %d, ret - %d)\n",
					_T(__FUNCTION__), __LINE__, (*playerIter)->GetSession()->_ID, size, enqueueRet);

				g_dump.Crash();
				return;
			}
		}
	}
	else
	{
		vector<Player*>::iterator playerIter = sector->_players.begin();
		for (; playerIter < sector->_players.end(); playerIter++)
		{
			if ((*playerIter)->GetSession() != pExpSession)
			{
				if ((*playerIter) == nullptr)
				{
					 LOG(L"FightGame", SystemLog::ERROR_LEVEL,
						L"%s[%d] Player in sector[%d][%d] is nullptr\n",
						_T(__FUNCTION__), __LINE__, sector->_xIndex, sector->_yIndex);

					::wprintf(L"%s[%d] Player in sector[%d][%d] is nullptr\n",
						_T(__FUNCTION__), __LINE__, sector->_xIndex, sector->_yIndex);

					g_dump.Crash();
					return;
				}

				int enqueueRet = (*playerIter)->GetSession()->_sendRingBuf.Enqueue(msg, size);
				if (enqueueRet != size)
				{
					 LOG(L"FightGame", SystemLog::ERROR_LEVEL,
						L"%s[%d] Session %d - sendRBuf Enqueue Error (req - %d, ret - %d)\n",
						_T(__FUNCTION__), __LINE__, (*playerIter)->GetSession()->_ID, size, enqueueRet);

					::wprintf(L"%s[%d] Session %d - sendRBuf Enqueue Error (req - %d, ret - %d)\n",
						_T(__FUNCTION__), __LINE__, (*playerIter)->GetSession()->_ID, size, enqueueRet);

					g_dump.Crash();
					return;
				}
			}
		}
	}

}

void IngameManager::SendPacketAroundSector(char* msg, int size, Sector* centerSector, Session* pExpSession)
{
	if (pExpSession == nullptr)
	{
		for(int i = 0; i < dfAROUND_SECTOR_NUM; i++)
		{
			vector<Player*>::iterator playerIter 
				= centerSector->_around[i]->_players.begin();

			for (; playerIter < centerSector->_around[i]->_players.end(); playerIter++)
			{
				if ((*playerIter) == nullptr)
				{
					 LOG(L"FightGame", SystemLog::ERROR_LEVEL,
						L"%s[%d] Player in sector[%d][%d] is nullptr\n",
						_T(__FUNCTION__), __LINE__, 
						centerSector->_around[i]->_xIndex, 
						centerSector->_around[i]->_yIndex);

					::wprintf(L"%s[%d] Player in sector[%d][%d] is nullptr\n",
						_T(__FUNCTION__), __LINE__, 
						centerSector->_around[i]->_xIndex, 
						centerSector->_around[i]->_yIndex);

					g_dump.Crash();
					return;
				}

				int enqueueRet = (*playerIter)->GetSession()->_sendRingBuf.Enqueue(msg, size);
				if (enqueueRet != size)
				{
					 LOG(L"FightGame", SystemLog::ERROR_LEVEL,
						L"%s[%d] Session %d - sendRBuf Enqueue Error (req - %d, ret - %d)\n",
						_T(__FUNCTION__), __LINE__, (*playerIter)->GetSession()->_ID, size, enqueueRet);

					::wprintf(L"%s[%d] Session %d - sendRBuf Enqueue Error (req - %d, ret - %d)\n",
						_T(__FUNCTION__), __LINE__, (*playerIter)->GetSession()->_ID, size, enqueueRet);

					g_dump.Crash();
					return;
				}
			}
		}
	}
	else
	{		
		for (int i = 0; i < dfMOVE_DIR_MAX; i++)
		{
			vector<Player*>::iterator playerIter 
				= centerSector->_around[i]->_players.begin();

			for (; playerIter < centerSector->_around[i]->_players.end(); playerIter++)
			{
				if ((*playerIter) == nullptr)
				{
					 LOG(L"FightGame", SystemLog::ERROR_LEVEL,
						L"%s[%d] Player in sector[%d][%d] is nullptr\n",
						_T(__FUNCTION__), __LINE__,
						centerSector->_around[i]->_xIndex,
						centerSector->_around[i]->_yIndex);

					::wprintf(L"%s[%d] Player in sector[%d][%d] is nullptr\n",
						_T(__FUNCTION__), __LINE__,
						centerSector->_around[i]->_xIndex,
						centerSector->_around[i]->_yIndex);

					g_dump.Crash();
					return;
				}

				int enqueueRet = (*playerIter)->GetSession()->_sendRingBuf.Enqueue(msg, size);
				if (enqueueRet != size)
				{
					 LOG(L"FightGame", SystemLog::ERROR_LEVEL,
						L"%s[%d] Session %d - sendRBuf Enqueue Error (req - %d, ret - %d)\n",
						_T(__FUNCTION__), __LINE__, (*playerIter)->GetSession()->_ID, size, enqueueRet);

					::wprintf(L"%s[%d] Session %d - sendRBuf Enqueue Error (req - %d, ret - %d)\n",
						_T(__FUNCTION__), __LINE__, (*playerIter)->GetSession()->_ID, size, enqueueRet);

					g_dump.Crash();
					return;
				}
			}
		}

		vector<Player*>::iterator playerIter 
			= centerSector->_around[dfMOVE_DIR_INPLACE]->_players.begin();
		for (; playerIter < centerSector->_around[dfMOVE_DIR_INPLACE]->_players.end(); playerIter++)
		{
			if ((*playerIter)->GetSession() != pExpSession)
			{
				if ((*playerIter) == nullptr)
				{
					 LOG(L"FightGame", SystemLog::ERROR_LEVEL,
						L"%s[%d] Player in sector[%d][%d] is nullptr\n",
						_T(__FUNCTION__), __LINE__,
						centerSector->_around[dfMOVE_DIR_INPLACE]->_xIndex,
						centerSector->_around[dfMOVE_DIR_INPLACE]->_yIndex);

					::wprintf(L"%s[%d] Player in sector[%d][%d] is nullptr\n",
						_T(__FUNCTION__), __LINE__,
						centerSector->_around[dfMOVE_DIR_INPLACE]->_xIndex,
						centerSector->_around[dfMOVE_DIR_INPLACE]->_yIndex);

					g_dump.Crash();
					return;
				}

				int enqueueRet = (*playerIter)->GetSession()->_sendRingBuf.Enqueue(msg, size);
				if (enqueueRet != size)
				{
					 LOG(L"FightGame", SystemLog::ERROR_LEVEL,
						L"%s[%d] Session %d - sendRBuf Enqueue Error (req - %d, ret - %d)\n",
						_T(__FUNCTION__), __LINE__, (*playerIter)->GetSession()->_ID, size, enqueueRet);

					::wprintf(L"%s[%d] Session %d - sendRBuf Enqueue Error (req - %d, ret - %d)\n",
						_T(__FUNCTION__), __LINE__, (*playerIter)->GetSession()->_ID, size, enqueueRet);

					g_dump.Crash();
					return;
				}
			}
		}
	}
}

/*===================================================================
*						About Player
===================================================================*/

void IngameManager::PlayerActionProc()
{
	for (int i = 0; i < dfSESSION_MAX; i++)
	{
		if (_Players[i] == nullptr) continue;

		if (timeGetTime() -_Players[i]->GetSession()->_lastRecvTime
				> dfNETWORK_PACKET_RECV_TIMEOUT)
		{ 
			_Players[i]->GetSession()->SetSessionDead();
			continue;
		}


		if (_Players[i]->GetStateMoving())
			_Players[i]->MoveUpdate();
	}
}

void IngameManager::CreatePlayer(Session* pSession)
{
	PRO_BEGIN(L"ObjectPool Alloc");
	Player* pPlayer = _pPlayerPool->Alloc(pSession, _playerID++);
	PRO_END(L"ObjectPool Alloc");
	_Players[pSession->_ID] = pPlayer;
	SetPlayerSector(pPlayer);
	
	// Send <Allocate ID Message> to New Player
	pPlayer->GetSession()->_sendSerialPacket.Clear();
	int createMeRet = SetSCPacket_CREATE_MY_CHARACTER(&pPlayer->GetSession()->_sendSerialPacket,
		pPlayer->GetID(), pPlayer->GetHeadDirection(), pPlayer->GetX(), pPlayer->GetY(), pPlayer->GetHp());
	NetworkManager::GetInstance().SendPacketUnicast(pPlayer->GetSession()->_sendSerialPacket.GetReadPtr(), createMeRet, pPlayer->GetSession());

	// Send <Create New Player Message> to All Player
	pPlayer->GetSession()->_sendSerialPacket.Clear();
	int createMeToOtherRet = SetSCPacket_CREATE_OTHER_CHARACTER(&pPlayer->GetSession()->_sendSerialPacket,
		pPlayer->GetID(), pPlayer->GetHeadDirection(), pPlayer->GetX(), pPlayer->GetY(), pPlayer->GetHp());
	SendPacketAroundSector(pPlayer->GetSession()->_sendSerialPacket.GetReadPtr(),
		createMeToOtherRet, pPlayer->GetSector(), pPlayer->GetSession());

	// Send <Create All Players Message> to New Player
	// 자기가 속한 섹터를 제외한 8방향 
	for (int i = 0; i < dfMOVE_DIR_MAX; i++)
	{
		vector<Player*>::iterator iter 
			= pPlayer->GetSector()->_around[i]->_players.begin();
		for (; iter < pPlayer->GetSector()->_around[i]->_players.end(); iter++)
		{
			pPlayer->GetSession()->_sendSerialPacket.Clear();
			int createOtherRet = SetSCPacket_CREATE_OTHER_CHARACTER(&pPlayer->GetSession()->_sendSerialPacket,
				(*iter)->GetID(), (*iter)->GetHeadDirection(), (*iter)->GetX(), (*iter)->GetY(), (*iter)->GetHp());
			NetworkManager::GetInstance().SendPacketUnicast(pPlayer->GetSession()->_sendSerialPacket.GetReadPtr(), createOtherRet, pPlayer->GetSession());

			if((*iter)->GetStateMoving())
			{
				pPlayer->GetSession()->_sendSerialPacket.Clear();
				int moveRet = SetSCPacket_MOVE_START(&pPlayer->GetSession()->_sendSerialPacket,
					(*iter)->GetID(), (*iter)->GetMoveDirection(), (*iter)->GetX(), (*iter)->GetY());
				NetworkManager::GetInstance().SendPacketUnicast(pPlayer->GetSession()->_sendSerialPacket.GetReadPtr(), moveRet, pPlayer->GetSession());
			}
		}
	}

	// 자기 자신이 위치한 섹터
	vector<Player*>::iterator iter
		= pPlayer->GetSector()->_around[dfMOVE_DIR_INPLACE]->_players.begin();
	for (; iter < pPlayer->GetSector()->_around[dfMOVE_DIR_INPLACE]->_players.end(); iter++)
	{
		if ((*iter) != pPlayer)
		{
			pPlayer->GetSession()->_sendSerialPacket.Clear();
			int createOtherRet = SetSCPacket_CREATE_OTHER_CHARACTER(&pPlayer->GetSession()->_sendSerialPacket,
				(*iter)->GetID(), (*iter)->GetHeadDirection(), (*iter)->GetX(), (*iter)->GetY(), (*iter)->GetHp());
			NetworkManager::GetInstance().SendPacketUnicast(pPlayer->GetSession()->_sendSerialPacket.GetReadPtr(), createOtherRet, pPlayer->GetSession());
			
			if ((*iter)->GetStateMoving())
			{
				pPlayer->GetSession()->_sendSerialPacket.Clear();
				int moveRet = SetSCPacket_MOVE_START(&pPlayer->GetSession()->_sendSerialPacket,
					(*iter)->GetID(), (*iter)->GetMoveDirection(), (*iter)->GetX(), (*iter)->GetY());
				NetworkManager::GetInstance().SendPacketUnicast(pPlayer->GetSession()->_sendSerialPacket.GetReadPtr(), moveRet, pPlayer->GetSession());
			}
		
		}
	}
}
