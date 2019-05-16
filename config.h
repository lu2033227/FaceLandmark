#include "stdafx.h"
#include "mtimes.h"

#define _sbin 8
#define _pointNum 32
#define _biasNum 23 //46
#define _binSizeRoot 15 //31
#define _binSizePart 5 //10
#define _featDim1 64 //136
#define _featDim2 4961 //10541
#define _featNum 317504 //1433576
#define _featPoint 155
#define INFINITY 100000000
#define MEASURETIME 1
#define COLORHOG 1
#define _iterNum 3
#define fastMode 1
#define addbiasnum 3.5

#define partnum 8

#define _loopNum 1000

#define USEDOUBLE 0

#if USEDOUBLE
    typedef double sType;
#else
    typedef float sType;
#endif


#define BBOXMode 1
#define DETECTMode 0
#define DETECTIONLIST 1
#define FINEGRAINMODE 0