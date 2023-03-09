#pragma once

#include "Types.h"
#include "CoreGlobal.h"
#include "CoreTLS.h"
#include "CoreMacro.h"

#include <vector>
#include <list>
#include <queue>
#include <stack>
#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>

#include <Windows.h>
#include <iostream>
using namespace std;

#include <WinSock2.h>
#include <MSWSock.h>
#include <WS2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

#include "Lock.h"
#include "Memory.h"
#include "SendBuffer.h"
// 서버코어 내부에서 활용함, 게임서버나 클라이언트에서 참조하길 원하는 내용들



