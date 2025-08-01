#pragma once

#include "Session.h"
#include "Player.h"
#include "Protocol.h"
#include "ObjectPool.h"
#include <stack>
#include <unordered_map>
#include <vector>

#include "SystemLog.h"

using namespace std;

/*========================
*		FUNCTION
========================*/


/*========================
*		CLASS
========================*/

class Player;

class NetworkManager
{
private:
	NetworkManager();
	~NetworkManager();
	SOCKET					_listensock = INVALID_SOCKET;
	FD_SET					_rset;
	FD_SET					_wset;
	timeval					_time;
	int						_addrlen;
public:
	// 복사 생성자 삭제
	NetworkManager(const NetworkManager&) = delete;
	NetworkManager& operator=(const NetworkManager&) = delete;
private:
	ObjectPool<Session>* _pSessionPool;
	Session* _Sessions[dfSESSION_MAX] = { nullptr };  // Use Session ID for Index
	Session* _rSessions[dfSESSION_MAX];
	Session* _wSessions[dfSESSION_MAX];
	stack <Session*> _usableSession;

	// 패킷 배칭을 위한 구조체
	struct BatchPacket {
		char* data;
		int size;
		Session* target;
		
		BatchPacket(char* d, int s, Session* t) : data(d), size(s), target(t) {}
		~BatchPacket() {
			if (data != nullptr) {
				delete[] data;
				data = nullptr;
			}
		}
		// 복사 생성자와 대입 연산자 삭제 (RAII 위반 방지)
		BatchPacket(const BatchPacket&) = delete;
		BatchPacket& operator=(const BatchPacket&) = delete;
		// 이동 생성자와 이동 대입 연산자 허용
		BatchPacket(BatchPacket&& other) noexcept : data(other.data), size(other.size), target(other.target) {
			other.data = nullptr;
		}
		BatchPacket& operator=(BatchPacket&& other) noexcept {
			if (this != &other) {
				if (data != nullptr) delete[] data;
				data = other.data;
				size = other.size;
				target = other.target;
				other.data = nullptr;
			}
			return *this;
		}
	};
	
	// 배칭 관련 멤버 변수
	std::vector<BatchPacket> _batchPackets;
	static const int MAX_BATCH_SIZE = 1000; // 배치 크기 제한
	

public:
	/*========================
	*	CLASS FUNCTION
	========================*/
	//싱글톤 인스턴스 반환
	static NetworkManager& GetInstance()
	{
		static NetworkManager _NetworkMgr;
		return _NetworkMgr;
	}
	inline bool HandleCSPackets(Player* pPlayer, BYTE action_type);
	inline bool HandleCSPacket_MoveStart(Player* pPlayer);
	inline bool HandleCSPacket_MoveStop(Player* pPlayer);
	inline bool HandleCSPacket_Attack1(Player* pPlayer);
	inline bool HandleCSPacket_Attack2(Player* pPlayer);
	inline bool HandleCSPacket_Attack3(Player* pPlayer);
	inline bool HandleCSPacket_ECHO(Player* pPlayer);
	inline void SendPacketUnicast(char* msg, int msgSize, Session* pSession);
	inline bool GetCSPacket_ECHO(SerializePacket* pPacket, RingBuffer* recvRBuffer, int& time);
	inline void SelectModel(int rStarIdx, int rCount, int wStartIdx, int wCount);
	void NetworkUpdate();
	// 배칭 관련 함수들
	void AddBatchPacket(char* msg, int size, Session* session);
	void FlushBatchPackets();
	void OptimizedSendPacketUnicast(const std::vector<std::pair<char*, int>>& packets, Session* session);
private:
	inline void AcceptProc();
	inline void RecvProc(Session* session);
	inline void SendProc(Session* session);

// session 관리
	DWORD _sessionIDs = 0;
	int _usableCnt = 0;
	DWORD _usableSessionID[dfSESSION_MAX];
	inline void DisconnectDeadSessions();
public:
	int _disconnectCnt = 0;
	DWORD _disconnectSessionIDs[dfSESSION_MAX];
};
