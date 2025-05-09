#pragma once

#pragma comment(lib, "ws2_32")
#pragma comment(lib, "winmm")
#pragma comment(lib, "DbgHelp.Lib")

// 소켓 관련
#include <WinSock2.h>
#include <WS2tcpip.h>

// 윈도우 API
#include <windows.h>
#include <Psapi.h>
#include <strsafe.h>

// C 런타임 라이브러리
#include <cstdio>
#include <cstdlib>
#include <ctime>

// 컨테이너 자료구조
#include <deque>
#include <vector>
#include <unordered_map>
#include <map>

// 기타 - 언어설정 등
#include <locale>
#include <DbgHelp.h>

// 직접 정의한 라이브러리
/*#include "Define.h"
#include "CObjectPool.h"
#include "CRingBuffer.h"
#include "CSerializableBuffer.h"
#include "CLogger.h"
#include "CProfiler.h"
#include "CrashDump.h"*/

// warning disable
#pragma warning(disable : 26110) // Lock 래핑 경고
#pragma warning(disable : 26495)
#pragma warning(disable : 4244)
#pragma warning(disable : 6011) // Crash 위한 nullptr 참조
