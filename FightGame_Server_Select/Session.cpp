#include "Session.h"


void Session::SetSessionDead()
{
	if (GetSessionAlive())
	{
		_bAlive = false;

		NetworkManager& nm = NetworkManager::GetInstance();
		nm._disconnectSessionIDs[nm._disconnectCnt] = GetID();
		nm._disconnectCnt++;
	}
}
