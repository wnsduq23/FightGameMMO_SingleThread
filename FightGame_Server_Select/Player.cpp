#include "Player.h"
#include "SetSCPacket.h"
#include "Protocol.h"
#include "IngameManager.h"

 Player::Player(Session* pSession, DWORD ID)
	: _pSession(pSession), _pSector(nullptr), _ID(ID),
	_headDirection(dfMOVE_DIR_RR),
	_moveDirection(dfMOVE_DIR_LL),
	_hp(dfMAX_HP), _bMove(false)
{
	_x = rand() % dfRANGE_MOVE_RIGHT;
	_y = rand() % dfRANGE_MOVE_BOTTOM;
}
Player::~Player()
{
}

bool Player::CheckMovable(short x, short y)
{
	if (x < dfRANGE_MOVE_LEFT || x > dfRANGE_MOVE_RIGHT ||
		y < dfRANGE_MOVE_TOP || y > dfRANGE_MOVE_BOTTOM)
		return false;

	return true;
}

void Player::MoveUpdate()
{
	switch (_moveDirection)
	{
	case dfMOVE_DIR_LL:
		if (CheckMovable(_x - dfSPEED_PLAYER_X, _y))
			_x -= dfSPEED_PLAYER_X;
		break;

	case dfMOVE_DIR_LU:
		if (CheckMovable(_x - dfSPEED_PLAYER_X, _y - dfSPEED_PLAYER_Y))
		{
			_x -= dfSPEED_PLAYER_X;
			_y -= dfSPEED_PLAYER_Y;
		}
		break;

	case dfMOVE_DIR_UU:
		if (CheckMovable(_x, _y - dfSPEED_PLAYER_Y))
			_y -= dfSPEED_PLAYER_Y;
		break;

	case dfMOVE_DIR_RU:
		if (CheckMovable(_x + dfSPEED_PLAYER_X, _y - dfSPEED_PLAYER_Y))
		{
			_x += dfSPEED_PLAYER_X;
			_y -= dfSPEED_PLAYER_Y;
		}
		break;

	case dfMOVE_DIR_RR:
		if (CheckMovable(_x + dfSPEED_PLAYER_X, _y))
			_x += dfSPEED_PLAYER_X;
		break;

	case dfMOVE_DIR_RD:
		if (CheckMovable(_x + dfSPEED_PLAYER_X, _y + dfSPEED_PLAYER_Y))
		{
			_x += dfSPEED_PLAYER_X;
			_y += dfSPEED_PLAYER_Y;
		}
		break;

	case dfMOVE_DIR_DD:
		if (CheckMovable(_x, _y + dfSPEED_PLAYER_Y))
			_y += dfSPEED_PLAYER_Y;
		break;

	case dfMOVE_DIR_LD:
		if (CheckMovable(_x - dfSPEED_PLAYER_X, _y + dfSPEED_PLAYER_Y))
		{
			_x -= dfSPEED_PLAYER_X;
			_y += dfSPEED_PLAYER_Y;
		}
		break;
	}
	IngameManager::GetInstance().UpdateSector(this);
}

void Player::SetPlayerMoveStart(BYTE& moveDirection, short& x, short& y)
{
	_x = x;
	_y = y;
	_bMove = true;
	_moveDirection = moveDirection;

	switch (moveDirection)
	{
	case  dfMOVE_DIR_LL:
	case  dfMOVE_DIR_LU:
	case  dfMOVE_DIR_LD:
		_headDirection = dfMOVE_DIR_LL;
		break;

	case  dfMOVE_DIR_RU:
	case  dfMOVE_DIR_RR:
	case  dfMOVE_DIR_RD:
		_headDirection = dfMOVE_DIR_RR;
		break;
	}	
}

void Player::SetPlayerMoveStop(BYTE& direction, short& x, short& y)
{
	_x = x;
	_y = y;
	_bMove = false;
	_headDirection = direction;	
}

void Player::SetPlayerAttack1(Player*& pDamagedPlayer, BYTE& direction, short& x, short& y)
{
	_x = x;
	_y = y;
	_headDirection = direction;

	if (direction == dfMOVE_DIR_LL)
	{
		Sector* pSector = _pSector;

		for (auto& pair : pSector->_around[dfMOVE_DIR_INPLACE]->_players)
		{
			Player* otherPlayer = pair.second;
			if (otherPlayer != this)
			{
				int dist = _x - otherPlayer->_x;
				if (dist >= 0 && dist <= dfATTACK1_RANGE_X &&
					abs(otherPlayer->_y - _y) <= dfATTACK1_RANGE_Y)
				{
					pDamagedPlayer = otherPlayer;
					pDamagedPlayer->_hp -= dfATTACK1_DAMAGE;

					if (pDamagedPlayer->_hp <= 0 || pDamagedPlayer->_hp > 100)
					{
						//_deadCnt++;
						pDamagedPlayer->_hp = 0;
						pDamagedPlayer->GetSession()->SetSessionDead();
					}
					return;
				}
			}
		}

		if (_x <= pSector->_xPosMin + dfATTACK1_RANGE_X)
		{
			for (auto& pair : pSector->_around[dfMOVE_DIR_LL]->_players)
			{
				Player* otherPlayer = pair.second;
				int dist = _x - otherPlayer->_x;
				if (dist >= 0 && dist <= dfATTACK1_RANGE_X &&
					abs(otherPlayer->_y - _y) <= dfATTACK1_RANGE_Y)
				{
					pDamagedPlayer = otherPlayer;
					pDamagedPlayer->_hp -= dfATTACK1_DAMAGE;

					if (pDamagedPlayer->_hp <= 0 || pDamagedPlayer->_hp > 100)
					{
						//_deadCnt++;
						pDamagedPlayer->_hp = 0;
						pDamagedPlayer->GetSession()->SetSessionDead();
					}
					return;
				}
			}

			if (_y <= pSector->_yPosMin + dfATTACK1_RANGE_Y)
			{
				for (auto& pair : pSector->_around[dfMOVE_DIR_LD]->_players)
				{
					Player* otherPlayer = pair.second;
					int dist = _x - otherPlayer->_x;
					if (dist >= 0 && dist <= dfATTACK1_RANGE_X &&
						abs(otherPlayer->_y - _y) <= dfATTACK1_RANGE_Y)
					{
						pDamagedPlayer = otherPlayer;
						pDamagedPlayer->_hp -= dfATTACK1_DAMAGE;

						if (pDamagedPlayer->_hp <= 0 || pDamagedPlayer->_hp > 100)
						{
							//_deadCnt++;
							pDamagedPlayer->_hp = 0;
							pDamagedPlayer->GetSession()->SetSessionDead();
						}

						return;
					}
				}
			}
			else if (_y >= pSector->_yPosMax - dfATTACK1_RANGE_Y)
			{
				for (auto& pair : pSector->_around[dfMOVE_DIR_LU]->_players)
				{
					Player* otherPlayer = pair.second;
					int dist = _x - otherPlayer->_x;
					if (dist >= 0 && dist <= dfATTACK1_RANGE_X &&
						abs(otherPlayer->_y - _y) <= dfATTACK1_RANGE_Y)
					{
						pDamagedPlayer = otherPlayer;
						pDamagedPlayer->_hp -= dfATTACK1_DAMAGE;

						if (pDamagedPlayer->_hp <= 0 || pDamagedPlayer->_hp > 100)
						{
							//_deadCnt++;
							pDamagedPlayer->_hp = 0;
							pDamagedPlayer->GetSession()->SetSessionDead();
						}

						return;
					}
				}
			}
		}
		else
		{
			if (_y <= pSector->_yPosMin + dfATTACK1_RANGE_Y)
			{
				for (auto& pair : pSector->_around[dfMOVE_DIR_DD]->_players)
				{
					Player* otherPlayer = pair.second;
					int dist = _x - otherPlayer->_x;
					if (dist >= 0 && dist <= dfATTACK1_RANGE_X &&
						abs(otherPlayer->_y - _y) <= dfATTACK1_RANGE_Y)
					{
						pDamagedPlayer = otherPlayer;
						pDamagedPlayer->_hp -= dfATTACK1_DAMAGE;

						if (pDamagedPlayer->_hp <= 0 || pDamagedPlayer->_hp > 100)
						{
							//_deadCnt++;
							pDamagedPlayer->_hp = 0;
							pDamagedPlayer->GetSession()->SetSessionDead();
						}
						return;
					}
				}
			}
			else if (_y >= pSector->_yPosMax - dfATTACK1_RANGE_Y)
			{
				for (auto& pair : pSector->_around[dfMOVE_DIR_UU]->_players)
				{
					Player* otherPlayer = pair.second;
					int dist = _x - otherPlayer->_x;
					if (dist >= 0 && dist <= dfATTACK1_RANGE_X &&
						abs(otherPlayer->_y - _y) <= dfATTACK1_RANGE_Y)
					{
						pDamagedPlayer = otherPlayer;
						pDamagedPlayer->_hp -= dfATTACK1_DAMAGE;

						if (pDamagedPlayer->_hp <= 0 || pDamagedPlayer->_hp > 100)
						{
							//_deadCnt++;
							pDamagedPlayer->_hp = 0;
							pDamagedPlayer->GetSession()->SetSessionDead();
						}

						return;
					}
				}
			}
		}
	}
	else if (direction == dfMOVE_DIR_RR)
	{
		Sector* pSector = _pSector;
		
		for (auto& pair : pSector->_around[dfMOVE_DIR_INPLACE]->_players)
		{
			Player* otherPlayer = pair.second;
			if (otherPlayer != this)
			{
				int dist = otherPlayer->_x - _x;
				if (dist >= 0 && dist <= dfATTACK1_RANGE_X &&
					abs(otherPlayer->_y - _y) <= dfATTACK1_RANGE_Y)
				{
					pDamagedPlayer = otherPlayer;
					pDamagedPlayer->_hp -= dfATTACK1_DAMAGE;

					if (pDamagedPlayer->_hp <= 0 || pDamagedPlayer->_hp > 100)
					{
						//_deadCnt++;
						pDamagedPlayer->_hp = 0;
						pDamagedPlayer->GetSession()->SetSessionDead();
					}

					return;
				}
			}
		}

		if (_x >= pSector->_xPosMax - dfATTACK1_RANGE_X)
		{
			for (auto& pair : pSector->_around[dfMOVE_DIR_RR]->_players)
			{
				Player* otherPlayer = pair.second;
				int dist = otherPlayer->_x - _x;
				if (dist >= 0 && dist <= dfATTACK1_RANGE_X &&
					abs(otherPlayer->_y - _y) <= dfATTACK1_RANGE_Y)
				{
					pDamagedPlayer = otherPlayer;
					pDamagedPlayer->_hp -= dfATTACK1_DAMAGE;

					if (pDamagedPlayer->_hp <= 0 || pDamagedPlayer->_hp > 100)
					{
						//_deadCnt++;
						pDamagedPlayer->_hp = 0;
						pDamagedPlayer->GetSession()->SetSessionDead();
					}

					return;
				}
			}

			if (_y >= pSector->_yPosMax - dfATTACK1_RANGE_Y)
			{
				for (auto& pair : pSector->_around[dfMOVE_DIR_RU]->_players)
				{
					Player* otherPlayer = pair.second;
					int dist = otherPlayer->_x - _x;
					if (dist >= 0 && dist <= dfATTACK1_RANGE_X &&
						abs(otherPlayer->_y - _y) <= dfATTACK1_RANGE_Y)
					{
						pDamagedPlayer = otherPlayer;
						pDamagedPlayer->_hp -= dfATTACK1_DAMAGE;

						if (pDamagedPlayer->_hp <= 0 || pDamagedPlayer->_hp > 100)
						{
							//_deadCnt++;
							pDamagedPlayer->_hp = 0;
							pDamagedPlayer->GetSession()->SetSessionDead();
						}
						return;
					}
				}
			}
			else if(_y <= pSector->_yPosMin + dfATTACK1_RANGE_Y)
			{
				for (auto& pair : pSector->_around[dfMOVE_DIR_RD]->_players)
				{
					Player* otherPlayer = pair.second;
					int dist = otherPlayer->_x - _x;
					if (dist >= 0 && dist <= dfATTACK1_RANGE_X &&
						abs(otherPlayer->_y - _y) <= dfATTACK1_RANGE_Y)
					{
						pDamagedPlayer = otherPlayer;
						pDamagedPlayer->_hp -= dfATTACK1_DAMAGE;

						if (pDamagedPlayer->_hp <= 0 || pDamagedPlayer->_hp > 100)
						{
							//_deadCnt++;
							pDamagedPlayer->_hp = 0;
							pDamagedPlayer->GetSession()->SetSessionDead();
						}

						return;
					}
				}
			}
		}
		else
		{
			if (_y >= pSector->_yPosMax - dfATTACK1_RANGE_Y)
			{
				for (auto& pair : pSector->_around[dfMOVE_DIR_UU]->_players)
				{
					Player* otherPlayer = pair.second;
					int dist = otherPlayer->_x - _x;
					if (dist >= 0 && dist <= dfATTACK1_RANGE_X &&
						abs(otherPlayer->_y - _y) <= dfATTACK1_RANGE_Y)
					{
						pDamagedPlayer = otherPlayer;
						pDamagedPlayer->_hp -= dfATTACK1_DAMAGE;

						if (pDamagedPlayer->_hp <= 0 || pDamagedPlayer->_hp > 100)
						{
							//_deadCnt++;
							pDamagedPlayer->_hp = 0;
							pDamagedPlayer->GetSession()->SetSessionDead();
						}
						return;
					}
				}
			}
			else if (_y <= pSector->_yPosMin + dfATTACK1_RANGE_Y)
			{
				for (auto& pair : pSector->_around[dfMOVE_DIR_DD]->_players)
				{
					Player* otherPlayer = pair.second;
					int dist = otherPlayer->_x - _x;
					if (dist >= 0 && dist <= dfATTACK1_RANGE_X &&
						abs(otherPlayer->_y - _y) <= dfATTACK1_RANGE_Y)
					{
						pDamagedPlayer = otherPlayer;
						pDamagedPlayer->_hp -= dfATTACK1_DAMAGE;

						if (pDamagedPlayer->_hp <= 0 || pDamagedPlayer->_hp > 100)
						{
							//_deadCnt++;
							pDamagedPlayer->_hp = 0;
							pDamagedPlayer->GetSession()->SetSessionDead();
						}

						return;
					}
				}
			}
		}
	}
}

void Player::SetPlayerAttack2(Player*& pDamagedPlayer, BYTE& direction, short& x, short& y)
{
	_x = x;
	_y = y;
	_headDirection = direction;

	if (direction == dfMOVE_DIR_LL)
	{
		Sector* pSector = _pSector;

		for (auto& pair : pSector->_around[dfMOVE_DIR_INPLACE]->_players)
		{
			Player* otherPlayer = pair.second;
			if (otherPlayer != this)
			{
				int dist = _x - otherPlayer->_x;
				if (dist >= 0 && dist <= dfATTACK2_RANGE_X &&
					abs(otherPlayer->_y - _y) <= dfATTACK2_RANGE_Y)
				{
					pDamagedPlayer = otherPlayer;
					pDamagedPlayer->_hp -= dfATTACK2_DAMAGE;

					if (pDamagedPlayer->_hp <= 0 || pDamagedPlayer->_hp > 100)
					{
						//_deadCnt++;
						pDamagedPlayer->_hp = 0;
						pDamagedPlayer->GetSession()->SetSessionDead();
					}

					return;
				}
			}
		}

		if (_x <= pSector->_xPosMin + dfATTACK2_RANGE_X)
		{
			for (auto& pair : pSector->_around[dfMOVE_DIR_LL]->_players)
			{
				Player* otherPlayer = pair.second;
				int dist = _x - otherPlayer->_x;
				if (dist >= 0 && dist <= dfATTACK2_RANGE_X &&
					abs(otherPlayer->_y - _y) <= dfATTACK2_RANGE_Y)
				{
					pDamagedPlayer = otherPlayer;
					pDamagedPlayer->_hp -= dfATTACK2_DAMAGE;

					if (pDamagedPlayer->_hp <= 0 || pDamagedPlayer->_hp > 100)
					{
						//_deadCnt++;
						pDamagedPlayer->_hp = 0;
						pDamagedPlayer->GetSession()->SetSessionDead();
					}

					return;
				}
			}

			if (_y <= pSector->_yPosMin + dfATTACK2_RANGE_Y)
			{
				for (auto& pair : pSector->_around[dfMOVE_DIR_LD]->_players)
				{
					Player* otherPlayer = pair.second;
					int dist = _x - otherPlayer->_x;
					if (dist >= 0 && dist <= dfATTACK2_RANGE_X &&
						abs(otherPlayer->_y - _y) <= dfATTACK2_RANGE_Y)
					{
						pDamagedPlayer = otherPlayer;
						pDamagedPlayer->_hp -= dfATTACK2_DAMAGE;

						if (pDamagedPlayer->_hp <= 0 || pDamagedPlayer->_hp > 100)
						{
							//_deadCnt++;
							pDamagedPlayer->_hp = 0;
							pDamagedPlayer->GetSession()->SetSessionDead();
						}
						return;
					}
				}
			}
			else if (_y >= pSector->_yPosMax - dfATTACK2_RANGE_Y)
			{
				for (auto& pair : pSector->_around[dfMOVE_DIR_LU]->_players)
				{
					Player* otherPlayer = pair.second;
					int dist = _x - otherPlayer->_x;
					if (dist >= 0 && dist <= dfATTACK2_RANGE_X &&
						abs(otherPlayer->_y - _y) <= dfATTACK2_RANGE_Y)
					{
						pDamagedPlayer = otherPlayer;
						pDamagedPlayer->_hp -= dfATTACK2_DAMAGE;

						if (pDamagedPlayer->_hp <= 0 || pDamagedPlayer->_hp > 100)
						{
							//_deadCnt++;
							pDamagedPlayer->_hp = 0;
							pDamagedPlayer->GetSession()->SetSessionDead();
						}

						return;
					}
				}
			}
		}
		else
		{
			if (_y <= pSector->_yPosMin + dfATTACK2_RANGE_Y)
			{
				for (auto& pair : pSector->_around[dfMOVE_DIR_DD]->_players)
				{
					Player* otherPlayer = pair.second;
					int dist = _x - otherPlayer->_x;
					if (dist >= 0 && dist <= dfATTACK2_RANGE_X &&
						abs(otherPlayer->_y - _y) <= dfATTACK2_RANGE_Y)
					{
						pDamagedPlayer = otherPlayer;
						pDamagedPlayer->_hp -= dfATTACK2_DAMAGE;

						if (pDamagedPlayer->_hp <= 0 || pDamagedPlayer->_hp > 100)
						{
							//_deadCnt++;
							pDamagedPlayer->_hp = 0;
							pDamagedPlayer->GetSession()->SetSessionDead();
						}

						return;
					}
				}
			}
			else if (_y >= pSector->_yPosMax - dfATTACK2_RANGE_Y)
			{
				for (auto& pair : pSector->_around[dfMOVE_DIR_UU]->_players)
				{
					Player* otherPlayer = pair.second;
					int dist = _x - otherPlayer->_x;
					if (dist >= 0 && dist <= dfATTACK2_RANGE_X &&
						abs(otherPlayer->_y - _y) <= dfATTACK2_RANGE_Y)
					{
						pDamagedPlayer = otherPlayer;
						pDamagedPlayer->_hp -= dfATTACK2_DAMAGE;

						if (pDamagedPlayer->_hp <= 0 || pDamagedPlayer->_hp > 100)
						{
							//_deadCnt++;
							pDamagedPlayer->_hp = 0;
							pDamagedPlayer->GetSession()->SetSessionDead();
						}

						return;
					}
				}
			}
		}
	}
	else if (direction == dfMOVE_DIR_RR)
	{
		Sector* pSector = _pSector;

		for (auto& pair : pSector->_around[dfMOVE_DIR_INPLACE]->_players)
		{
			Player* otherPlayer = pair.second;
			if (otherPlayer != this)
			{
				int dist = otherPlayer->_x - _x;
				if (dist >= 0 && dist <= dfATTACK2_RANGE_X &&
					abs(otherPlayer->_y - _y) <= dfATTACK2_RANGE_Y)
				{
					pDamagedPlayer = otherPlayer;
					pDamagedPlayer->_hp -= dfATTACK2_DAMAGE;

					if (pDamagedPlayer->_hp <= 0 || pDamagedPlayer->_hp > 100)
					{
						//_deadCnt++;
						pDamagedPlayer->_hp = 0;
						pDamagedPlayer->GetSession()->SetSessionDead();
					}

					return;
				}
			}
		}

		if (_x >= pSector->_xPosMax - dfATTACK2_RANGE_X)
		{
			for (auto& pair : pSector->_around[dfMOVE_DIR_RR]->_players)
			{
				Player* otherPlayer = pair.second;
				int dist = otherPlayer->_x - _x;
				if (dist >= 0 && dist <= dfATTACK2_RANGE_X &&
					abs(otherPlayer->_y - _y) <= dfATTACK2_RANGE_Y)
				{
					pDamagedPlayer = otherPlayer;
					pDamagedPlayer->_hp -= dfATTACK2_DAMAGE;

					if (pDamagedPlayer->_hp <= 0 || pDamagedPlayer->_hp > 100)
					{
						//_deadCnt++;
						pDamagedPlayer->_hp = 0;
						pDamagedPlayer->GetSession()->SetSessionDead();
					}

					return;
				}
			}

			if (_y >= pSector->_yPosMax - dfATTACK2_RANGE_Y)
			{
				for (auto& pair : pSector->_around[dfMOVE_DIR_RU]->_players)
				{
					Player* otherPlayer = pair.second;
					int dist = otherPlayer->_x - _x;
					if (dist >= 0 && dist <= dfATTACK2_RANGE_X &&
						abs(otherPlayer->_y - _y) <= dfATTACK2_RANGE_Y)
					{
						pDamagedPlayer = otherPlayer;
						pDamagedPlayer->_hp -= dfATTACK2_DAMAGE;

						if (pDamagedPlayer->_hp <= 0 || pDamagedPlayer->_hp > 100)
						{
							//_deadCnt++;
							pDamagedPlayer->_hp = 0;
							pDamagedPlayer->GetSession()->SetSessionDead();
						}

						return;
					}
				}
			}
			else if (_y <= pSector->_yPosMin + dfATTACK2_RANGE_Y)
			{
				for (auto& pair : pSector->_around[dfMOVE_DIR_RD]->_players)
				{
					Player* otherPlayer = pair.second;
					int dist = otherPlayer->_x - _x;
					if (dist >= 0 && dist <= dfATTACK2_RANGE_X &&
						abs(otherPlayer->_y - _y) <= dfATTACK2_RANGE_Y)
					{
						pDamagedPlayer = otherPlayer;
						pDamagedPlayer->_hp -= dfATTACK2_DAMAGE;

						if (pDamagedPlayer->_hp <= 0 || pDamagedPlayer->_hp > 100)
						{
							//_deadCnt++;
							pDamagedPlayer->_hp = 0;
							pDamagedPlayer->GetSession()->SetSessionDead();
						}

						return;
					}
				}
			}
		}
		else
		{
			if (_y >= pSector->_yPosMax - dfATTACK2_RANGE_Y)
			{
				for (auto& pair : pSector->_around[dfMOVE_DIR_UU]->_players)
				{
					Player* otherPlayer = pair.second;
					int dist = otherPlayer->_x - _x;
					if (dist >= 0 && dist <= dfATTACK2_RANGE_X &&
						abs(otherPlayer->_y - _y) <= dfATTACK2_RANGE_Y)
					{
						pDamagedPlayer = otherPlayer;
						pDamagedPlayer->_hp -= dfATTACK2_DAMAGE;

						if (pDamagedPlayer->_hp <= 0 || pDamagedPlayer->_hp > 100)
						{
							//_deadCnt++;
							pDamagedPlayer->_hp = 0;
							pDamagedPlayer->GetSession()->SetSessionDead();
						}

						return;
					}
				}
			}
			else if (_y <= pSector->_yPosMin + dfATTACK2_RANGE_Y)
			{
				for (auto& pair : pSector->_around[dfMOVE_DIR_DD]->_players)
				{
					Player* otherPlayer = pair.second;
					int dist = otherPlayer->_x - _x;
					if (dist >= 0 && dist <= dfATTACK2_RANGE_X &&
						abs(otherPlayer->_y - _y) <= dfATTACK2_RANGE_Y)
					{
						pDamagedPlayer = otherPlayer;
						pDamagedPlayer->_hp -= dfATTACK2_DAMAGE;

						if (pDamagedPlayer->_hp <= 0 || pDamagedPlayer->_hp > 100)
						{
							//_deadCnt++;
							pDamagedPlayer->_hp = 0;
							pDamagedPlayer->GetSession()->SetSessionDead();
						}

						return;
					}
				}
			}
		}
	}
}
void Player::SetPlayerAttack3(Player*& pDamagedPlayer, BYTE& direction, short& x, short& y)
{
	_x = x;
	_y = y;
	_headDirection = direction;

	if (direction == dfMOVE_DIR_LL)
	{
		Sector* pSector = _pSector;

		for (auto& pair : pSector->_around[dfMOVE_DIR_INPLACE]->_players)
		{
			Player* otherPlayer = pair.second;
			if (otherPlayer != this)
			{
				int dist = _x - otherPlayer->_x;
				if (dist >= 0 && dist <= dfATTACK3_RANGE_X &&
					abs(otherPlayer->_y - _y) <= dfATTACK3_RANGE_Y)
				{
					pDamagedPlayer = otherPlayer;
					pDamagedPlayer->_hp -= dfATTACK3_DAMAGE;

					if (pDamagedPlayer->_hp <= 0 || pDamagedPlayer->_hp > 100)
					{
						//_deadCnt++;
						pDamagedPlayer->_hp = 0;
						pDamagedPlayer->GetSession()->SetSessionDead();
					}

					return;
				}
			}
		}

		if (_x <= pSector->_xPosMin + dfATTACK3_RANGE_X)
		{
			for (auto& pair : pSector->_around[dfMOVE_DIR_LL]->_players)
			{
				Player* otherPlayer = pair.second;
				int dist = _x - otherPlayer->_x;
				if (dist >= 0 && dist <= dfATTACK3_RANGE_X &&
					abs(otherPlayer->_y - _y) <= dfATTACK3_RANGE_Y)
				{
					pDamagedPlayer = otherPlayer;
					pDamagedPlayer->_hp -= dfATTACK3_DAMAGE;

					if (pDamagedPlayer->_hp <= 0 || pDamagedPlayer->_hp > 100)
					{
						//_deadCnt++;
						pDamagedPlayer->_hp = 0;
						pDamagedPlayer->GetSession()->SetSessionDead();
					}

					return;
				}
			}

			if (_y <= pSector->_yPosMin + dfATTACK3_RANGE_Y)
			{
				for (auto& pair : pSector->_around[dfMOVE_DIR_LD]->_players)
				{
					Player* otherPlayer = pair.second;
					int dist = _x - otherPlayer->_x;
					if (dist >= 0 && dist <= dfATTACK3_RANGE_X &&
						abs(otherPlayer->_y - _y) <= dfATTACK3_RANGE_Y)
					{
						pDamagedPlayer = otherPlayer;
						pDamagedPlayer->_hp -= dfATTACK3_DAMAGE;

						if (pDamagedPlayer->_hp <= 0 || pDamagedPlayer->_hp > 100)
						{
							//_deadCnt++;
							pDamagedPlayer->_hp = 0;
							pDamagedPlayer->GetSession()->SetSessionDead();
						}

						return;
					}
				}
			}
			else if (_y >= pSector->_yPosMax - dfATTACK3_RANGE_Y)
			{
				for (auto& pair : pSector->_around[dfMOVE_DIR_LU]->_players)
				{
					Player* otherPlayer = pair.second;
					int dist = _x - otherPlayer->_x;
					if (dist >= 0 && dist <= dfATTACK3_RANGE_X &&
						abs(otherPlayer->_y - _y) <= dfATTACK3_RANGE_Y)
					{
						pDamagedPlayer = otherPlayer;
						pDamagedPlayer->_hp -= dfATTACK3_DAMAGE;

						if (pDamagedPlayer->_hp <= 0 || pDamagedPlayer->_hp > 100)
						{
							//_deadCnt++;
							pDamagedPlayer->_hp = 0;
							pDamagedPlayer->GetSession()->SetSessionDead();
						}

						return;
					}
				}
			}
		}
		else
		{
			if (_y <= pSector->_yPosMin + dfATTACK3_RANGE_Y)
			{
				for (auto& pair : pSector->_around[dfMOVE_DIR_DD]->_players)
				{
					Player* otherPlayer = pair.second;
					int dist = _x - otherPlayer->_x;
					if (dist >= 0 && dist <= dfATTACK3_RANGE_X &&
						abs(otherPlayer->_y - _y) <= dfATTACK3_RANGE_Y)
					{
						pDamagedPlayer = otherPlayer;
						pDamagedPlayer->_hp -= dfATTACK3_DAMAGE;

						if (pDamagedPlayer->_hp <= 0 || pDamagedPlayer->_hp > 100)
						{
							//_deadCnt++;
							pDamagedPlayer->_hp = 0;
							pDamagedPlayer->GetSession()->SetSessionDead();
						}

						return;
					}
				}
			}
			else if (_y >= pSector->_yPosMax - dfATTACK3_RANGE_Y)
			{
				for (auto& pair : pSector->_around[dfMOVE_DIR_UU]->_players)
				{
					Player* otherPlayer = pair.second;
					int dist = _x - otherPlayer->_x;
					if (dist >= 0 && dist <= dfATTACK3_RANGE_X &&
						abs(otherPlayer->_y - _y) <= dfATTACK3_RANGE_Y)
					{
						pDamagedPlayer = otherPlayer;
						pDamagedPlayer->_hp -= dfATTACK3_DAMAGE;

						if (pDamagedPlayer->_hp <= 0 || pDamagedPlayer->_hp > 100)
						{
							//_deadCnt++;
							pDamagedPlayer->_hp = 0;
							pDamagedPlayer->GetSession()->SetSessionDead();
						}

						return;
					}
				}
			}
		}
	}
	else if (direction == dfMOVE_DIR_RR)
	{
		Sector* pSector = _pSector;

		for (auto& pair : pSector->_around[dfMOVE_DIR_INPLACE]->_players)
		{
			Player* otherPlayer = pair.second;
			if (otherPlayer != this)
			{
				int dist = otherPlayer->_x - _x;
				if (dist >= 0 && dist <= dfATTACK3_RANGE_X &&
					abs(otherPlayer->_y - _y) <= dfATTACK3_RANGE_Y)
				{
					pDamagedPlayer = otherPlayer;
					pDamagedPlayer->_hp -= dfATTACK3_DAMAGE;

					if (pDamagedPlayer->_hp <= 0 || pDamagedPlayer->_hp > 100)
					{
						//_deadCnt++;
						pDamagedPlayer->_hp = 0;
						pDamagedPlayer->GetSession()->SetSessionDead();
					}
					return;
				}
			}
		}

		if (_x >= pSector->_xPosMax - dfATTACK3_RANGE_X)
		{
			for (auto& pair : pSector->_around[dfMOVE_DIR_RR]->_players)
			{
				Player* otherPlayer = pair.second;
				int dist = otherPlayer->_x - _x;
				if (dist >= 0 && dist <= dfATTACK3_RANGE_X &&
					abs(otherPlayer->_y - _y) <= dfATTACK3_RANGE_Y)
				{
					pDamagedPlayer = otherPlayer;
					pDamagedPlayer->_hp -= dfATTACK3_DAMAGE;

					if (pDamagedPlayer->_hp <= 0 || pDamagedPlayer->_hp > 100)
					{
						//_deadCnt++;
						pDamagedPlayer->_hp = 0;
						pDamagedPlayer->GetSession()->SetSessionDead();
					}

					return;
				}
			}

			if (_y >= pSector->_yPosMax - dfATTACK3_RANGE_Y)
			{
				for (auto& pair : pSector->_around[dfMOVE_DIR_RU]->_players)
				{
					Player* otherPlayer = pair.second;
					int dist = otherPlayer->_x - _x;
					if (dist >= 0 && dist <= dfATTACK3_RANGE_X &&
						abs(otherPlayer->_y - _y) <= dfATTACK3_RANGE_Y)
					{
						pDamagedPlayer = otherPlayer;
						pDamagedPlayer->_hp -= dfATTACK3_DAMAGE;

						if (pDamagedPlayer->_hp <= 0 || pDamagedPlayer->_hp > 100)
						{
							//_deadCnt++;
							pDamagedPlayer->_hp = 0;
							pDamagedPlayer->GetSession()->SetSessionDead();
						}

						return;
					}
				}
			}
			else if (_y <= pSector->_yPosMin + dfATTACK3_RANGE_Y)
			{
				for (auto& pair : pSector->_around[dfMOVE_DIR_RD]->_players)
				{
					Player* otherPlayer = pair.second;
					int dist = otherPlayer->_x - _x;
					if (dist >= 0 && dist <= dfATTACK3_RANGE_X &&
						abs(otherPlayer->_y - _y) <= dfATTACK3_RANGE_Y)
					{
						pDamagedPlayer = otherPlayer;
						pDamagedPlayer->_hp -= dfATTACK3_DAMAGE;

						if (pDamagedPlayer->_hp <= 0 || pDamagedPlayer->_hp > 100)
						{
							//_deadCnt++;
							pDamagedPlayer->_hp = 0;
							pDamagedPlayer->GetSession()->SetSessionDead();
						}
						return;
					}
				}
			}
		}
		else
		{
			if (_y >= pSector->_yPosMax - dfATTACK3_RANGE_Y)
			{
				for (auto& pair : pSector->_around[dfMOVE_DIR_UU]->_players)
				{
					Player* otherPlayer = pair.second;
					int dist = otherPlayer->_x - _x;
					if (dist >= 0 && dist <= dfATTACK3_RANGE_X &&
						abs(otherPlayer->_y - _y) <= dfATTACK3_RANGE_Y)
					{
						pDamagedPlayer = otherPlayer;
						pDamagedPlayer->_hp -= dfATTACK3_DAMAGE;

						if (pDamagedPlayer->_hp <= 0 || pDamagedPlayer->_hp > 100)
						{
							//_deadCnt++;
							pDamagedPlayer->_hp = 0;
							pDamagedPlayer->GetSession()->SetSessionDead();
						}

						return;
					}
				}
			}
			else if (_y <= pSector->_yPosMin + dfATTACK3_RANGE_Y)
			{
				for (auto& pair : pSector->_around[dfMOVE_DIR_DD]->_players)
				{
					Player* otherPlayer = pair.second;
					int dist = otherPlayer->_x - _x;
					if (dist >= 0 && dist <= dfATTACK3_RANGE_X &&
						abs(otherPlayer->_y - _y) <= dfATTACK3_RANGE_Y)
					{
						pDamagedPlayer = otherPlayer;
						pDamagedPlayer->_hp -= dfATTACK3_DAMAGE;

						if (pDamagedPlayer->_hp <= 0 || pDamagedPlayer->_hp > 100)
						{
							//_deadCnt++;
							pDamagedPlayer->_hp = 0;
							pDamagedPlayer->GetSession()->SetSessionDead();
						}

						return;
					}
				}
			}
		}
	}
}


