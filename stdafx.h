#pragma once
#pragma warning(disable: 4275)
#pragma warning(disable: 4005)
/*
#define SUPER_CALC_ON
#include "MemoryMan/platform.h"
#include "MemoryMan/MemoryMan.h"
#pragma comment(lib, "lib/debuglib/MemoryMan.lib")
#undef pure*/

#include <stdlib.h>

#define _HAS_EXCEPTIONS 0

#include <string>
#include <sstream>
#include <math.h>
#include <vector>
using namespace std;

#include <assert.h>
#include <time.h>

double myGetPreciseTime();