#pragma once

#include <Windows.h>
#include <assert.h>

//STL
#include <string>
#include <vector>
#include <list>
#include <map>
#include <unordered_map>
#include <functional>
#include <iterator>
#include <thread>
#include <mutex>
using namespace std;

//DirectWrite
#include <d2d1_1.h>
#include <dwrite.h>
#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")

//Direct3D
#include <dxgi1_2.h>
#include <d3dcommon.h>
#include <d3dcompiler.h>
#include <d3d11shader.h>
#include <d3d11.h>
#include <d3dx10math.h>
#include <d3dx11async.h>
#include <d3dx11effect.h>

#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dx9.lib")
#pragma comment(lib, "d3dx10.lib")
#pragma comment(lib, "d3dx11.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "Effects11d.lib")

//ImGui
#include <imgui.h>
#include <imguiDx11.h>
#pragma comment(lib, "imgui.lib")

//DirectXTex
#include <DirectXTex.h>
#pragma comment(lib, "directxtex.lib")

#define SAFE_RELEASE(p){ if(p){ (p)->Release(); (p) = NULL; } }
#define SAFE_DELETE(p){ if(p){ delete (p); (p) = NULL; } }
#define SAFE_DELETE_ARRAY(p){ if(p){ delete [] (p); (p) = NULL; } }


const wstring Assets = L"../../_Assets/";
const wstring Shaders = L"../../_Shaders/";
const wstring Textures = L"../../_Textures/";
const wstring Models = L"../../_Models/";


#include "./Systems/D3D.h"
#include "./Systems/DirectWrite.h"
#include "./Systems/Keyboard.h"
#include "./Systems/Mouse.h"
#include "./Systems/Time.h"

#include "./Renders/VertexLayouts.h"
#include "./Renders/Shader.h"
#include "./Renders/Texture.h"
#include "./Renders/CBuffer.h"
#include "./Renders/Context.h"
#include "./Renders/Material.h"
#include "./Renders/CsResource.h"

#include "./Viewer/Viewport.h"
#include "./Viewer/RenderTargetView.h"
#include "./Viewer/DepthStencilView.h"
#include "./Viewer/Camera.h"
#include "./Viewer/Perspective.h"

#include "./Utilities/Math.h"
#include "./Utilities/String.h"
#include "./Utilities/Path.h"

#include "./Boundings/Ray.h"
#include "./Boundings/BBox.h"
#include "./Boundings/BSphere.h"

#include "./Draw/DebugLine.h"
#include "./Draw/MeshCube.h"
#include "./Draw/MeshCylinder.h"
#include "./Draw/MeshGrid.h"
#include "./Draw/MeshSphere.h"
#include "./Draw/MeshQuad.h"

#include "./Objects/GameModel.h"
#include "./Objects/GameAnimator.h"

/*
<Library 활용>
Framework(lib파일)라는 Libarary 만듬 프로그램하면서 공통적인것들을 넣음
우리는 정적 Library로 만듬 
동적라이브러리는 장점이 어떤 언어로 작성해도 어느 프로그램에서도 사용가능
단점이 링크걸어서 인보킹해야하고, 다른 프로그램에서 쓰게되면 무슨 함수를 가지고 있는지 공개해주어야한다

만든 Framework를 물고 갈 Editor(exe파일)들을 만든다(맵,모델,파티클,게임 등등)
실제로 실행되는것은 Editor들

이렇게 만드면 장점이 바꿀것이 있으면 Framework만 바꾸면 그 밑에 있는 Editor
들은 다 바뀐다. 관리가 편해짐

나중에 Editor들이 굉장히 많아지는데 Editor뿐만 아니라 거기서 나온 공통적인 것들을
framework에서 다루는데 그 여러 Editor + 공통적인것들을 gameEditor에 보내서 실행을
시켜줘야한다. gameEditor에 넣어서 실행시키는것이 정말 중요하다. 그래서 이런 구조 사용
*/