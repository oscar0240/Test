#pragma once
#include "Framework.h"

#pragma comment(lib, "Framework.lib")

//Fbx SDK
#define FBXSDK_SHARED
#include <fbxsdk.h>
#pragma comment(lib, "libfbxsdk.lib")
using namespace fbxsdk;

/*
우리가 WVP->Vp변환을 하면 화면안에 들어가는데 출력되는 화면의 범위가 -1에서 1로 된다.
모니터마다 해상도가 다르기때문에 -1부터 1까지 범위를 잡고 나중에 해상도를 곱해준다.

WVP->Vp변환을 빼고 Output.pos = Input.pos를 바로 들어가면 P(포지션이)->Vp로 되면 
공간이 -1부터 1까지로 잡힌다. 즉 무조건 Vp로 변환되면 화면의 범위는 -1부터 1까지

-1부터 1까지의 공간을 NDC공간이라고 한다 정규화된 공간

*/