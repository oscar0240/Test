#pragma once
#include "Framework.h"

#pragma comment(lib, "Framework.lib")

//Fbx SDK
#define FBXSDK_SHARED
#include <fbxsdk.h>
#pragma comment(lib, "libfbxsdk.lib")
using namespace fbxsdk;

/*
�츮�� WVP->Vp��ȯ�� �ϸ� ȭ��ȿ� ���µ� ��µǴ� ȭ���� ������ -1���� 1�� �ȴ�.
����͸��� �ػ󵵰� �ٸ��⶧���� -1���� 1���� ������ ��� ���߿� �ػ󵵸� �����ش�.

WVP->Vp��ȯ�� ���� Output.pos = Input.pos�� �ٷ� ���� P(��������)->Vp�� �Ǹ� 
������ -1���� 1������ ������. �� ������ Vp�� ��ȯ�Ǹ� ȭ���� ������ -1���� 1����

-1���� 1������ ������ NDC�����̶�� �Ѵ� ����ȭ�� ����

*/