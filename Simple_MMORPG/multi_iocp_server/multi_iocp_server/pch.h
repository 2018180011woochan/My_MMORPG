#pragma once

#include <iostream>
#include <array>
#include <WS2tcpip.h>
#include <MSWSock.h>

#include <thread>
#include <vector>
#include <mutex>
#include <unordered_set>
#include <queue>
#include <chrono>
#include <string>
#include <atomic>

#include <windows.h>  
#include <locale.h>

#define UNICODE  
#include <sqlext.h>  

constexpr int RANGE = 15;
constexpr int MONSTER_RANGE = 5;

#pragma comment(lib, "WS2_32.lib")
#pragma comment(lib, "MSWSock.lib")
using namespace std;

#include "Include.h"

