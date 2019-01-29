#pragma once

struct D3DDesc
{
	wstring AppName;
	HINSTANCE Instance;
	HWND Handle;
	float Width;
	float Height;
	bool bVsync;
	bool bFullScreen;
};

class D3D
{
public:
	static D3D* Get();

	static void Create();
	static void Delete();

	static ID3D11Device* GetDevice()
	{
		return device;
	}

	static ID3D11DeviceContext* GetDC()
	{
		return deviceContext;
	}

	static IDXGISwapChain* GetSwapChain()
	{
		return swapChain;
	}

	static void GetDesc(D3DDesc* desc)
	{
		*desc = d3dDesc;
	}

	static void SetDesc(D3DDesc& desc)
	{
		d3dDesc = desc;
	}

	void SetRenderTarget(ID3D11RenderTargetView* rtv = NULL, ID3D11DepthStencilView* dsv = NULL);

	void Clear(D3DXCOLOR color = D3DXCOLOR(0xFF555566), ID3D11RenderTargetView* rtv = NULL, ID3D11DepthStencilView* dsv = NULL);
	void Present();

	void ResizeScreen(float width, float height);

private:
	D3D();
	~D3D();

	void SetGpuInfo();
	void CreateSwapChainAndDevice();

	void CreateBackBuffer(float width, float height);
	void DeleteBackBuffer();

private:
	static D3D* instance;

	static D3DDesc d3dDesc;
	static ID3D11Device* device;
	static ID3D11DeviceContext* deviceContext;
	static IDXGISwapChain* swapChain;

	ID3D11Debug* debugDevice;

	UINT gpuMemorySize;
	wstring gpuDescription;

	UINT numerator;
	UINT denominator;

	ID3D11Texture2D* backBuffer;
	ID3D11DepthStencilView* depthStencilView;
	ID3D11RenderTargetView* renderTargetView;
};

/*
<Direct3D 기초>
Direct3D->하드웨어 또는 소프트웨어로 3D를 표현하는 그래픽API
3D 렌더링 순서->정점변환(월드 변환, 뷰 변환, 정규 변환, 뷰포트 변환)
Rasterizing(기하 정보를 이용해서 픽셀을 생성)
픽셀처리(텍스쳐 샘플링, addressing, Filtering, Alpha, Depth, Stencil)
소프트웨어 렌더링->장치에 독립, 3D렌더링 과정을 CPU에서 처리 Rasterizing, 픽셀 처리에서 과부하
하드웨어 렌더링->장치에 의존, 그래픽 하드웨어에게 처리를 맡김, vertex pixel Shader 사용 가능
가상 머신 개념을 두어 하드웨어 처리에서 필요한 값을 소프트웨어에서 설정
*/

/*
<DirecX11 특징 및 파이프라인>
1. Compute Shader
11부터는 CS를 지원해서 그래픽 처리 이외에도 데이터를 병렬처리할 수 있어서 GPU를 범용적으로 사용가능

2. Tessellation
테셀레이션은 정밀도가 낮은 프리미티브(DirectX에서 렌더링 하는 도형의 최소단위(점,선,삼각형))데이터들을
분할하여 보다 정밀하게 프리미티브를 생성하여 출력하는 기능
(11의 그래픽 파이프라인에는 테셀레이션을 수행하는 1개의 스테이지와 2개의 Shader(헐쉐이더, 도메인쉐이더)추가)

3. 멀티 쓰레드 렌더링
DirectX의 디바이스의 기능은 디바이스와 디바이스 컨텍스트로 분할된다. 디바이스 컨텍스트에는 mmediate Context와 Deferred Context있음
Immediate Context -> 메인 스레드에서ㅌ 렌더링하기 위해 사용하는 녀석으로, 서브리소스 매핑, 서브리소스 업데이트 
등의 작업부터 시작하여 명령 리스트 실행도 담당하는 어떻게 보면 중앙 집중형 컨텍스트이다. 이 녀석이 없다면 실질적인 렌더링을 수행할 수 없다.
Deferred Context -> 서브 스레드에서 렌더링하기 위해 사용하는 녀석으로, 다른 작업은 다 할 수 있지만 서브리소스 편집을 수행할 수 없다. 
다만 동적 리소스의 경우 Map 작업은 수행할 수 있다. Deferred Context에서 작업한 결과는 당장 렌더링되지는 않는다.
즉. 이미디에이트 컨텍스트는 디바이스에 직접 렌더링하는 녀석으로 디바이스에 1개만 존재한다 그래서 싱글 쓰레드는 이녀석만 쓰는것
    디퍼드 컨텍스트는 메인 렌더링 쓰레드 이외에 워커 쓰레드에서 사용하는 컨텍스트
	멀티 쓰레드란 이 디퍼드 컨텍스트를 사용하는것

4. 동적 Shader 링크
일반적으로 렌더링에서는 각종 material을 여러가지 조합을 통해서 렌더링한다 그래서 여러가지 기능의 다른 Shader가 필요하다.
11이전에는 모든 기능을 갖춘 쉐이더 만들기, 필요한 조합의 쉐이더만을 사용하는 방법 이렇게 2개를 사용했는데 퍼포먼스와 관리면에서 힘들어서
11부터 동적 Shader 링크를 사용해서 쉐이더를 렌더링 파이프라인에 할당할때 쉐이더 코드를 드라이버에 최적화 할 수 있게 되었음.

5. WARP(Windows Advanced Rasterizer Platform)
실제 어플리케이션에서 사용 가능한 고속의 멀티코어 스케일링 래스터라이저
어플리케이션에서 필요한 하드웨어 디바이스를 얻을 수 없을때 WARP디바이스를 선택할 수 있다.

6. Direct3D 9/10/10.1레벨의 하드웨어 지원
D3D_FEATURE_LEVEL_9_1부터 11_1까지의 디바이스를 호환할 수 있다.

7. Shader Model 5.0 지원
X10부터 통합형 쉐이더(Unified-Shader)아키텍처가 채택. CS를 포함한 모든 쉐이더를 하나의 HLSL로 작성할 수 있다.

8. 리소스
11에서는 RW버퍼(텍스처), StructuredBuffer, byteAddressBuffer, UnorderedAccessBuffer 등의 새로운 리소스 정의됨.
또 4GB보다 큰 리소스를 지원

9. DirectXGI
DirectXGI은 DirectX11등의 그래픽 기능이나 어플리케이션으로부터 오는 표시를 받아 Kernel-Mode Driver나 하드웨어와 주고 받는 역할

10. 스왑 체인(Swap Chain) [나중에 rendertoTexture 수업때 쌤 설명 보기]
DirectXGI의 중요한 기능으로 스왑체인이 있다.
11에서는 렌더링에 front buffer와 back buffer를 사용한다. 
front buffer는 디스플레이상 표시되고 있는 화면 데이터를 가지고 있는 버퍼이다. 이걸 일단 백 버퍼에 쓰고, 렌더링이 끝나면
디스플레이에 표시되는 버퍼의 내용을 한번 갱신한다. 또 갱신 작업중에는 백 버퍼에도 렌더링을 할 수 없다.
그래서 퍼포먼스 저하를 막기 위해 3개 이상의 버퍼를 준비하는 것이 가능하다.
이 같이 프런트 버퍼나 백 버퍼를 포함한 여러개의 버퍼의 집합과 이 버퍼의 전환 방식을 [스왑체인]이라고 한다.

11. DirectX 11의 렌더링 파이프라인
Shader : 컴퓨터 그래픽스 분야에서 셰이더(shader)는 소프트웨어 명령의 집합으로 주로 그래픽 하드웨어의 렌더링 효과를 계산하는 데 쓰인다.
         셰이더는 그래픽 처리 장치(GPU)의 프로그래밍이 가능한 렌더링 파이프라인을 프로그래밍하는 데 쓰인다.

Vertex Buffer와 Index Buffer : 각 프리미티브(원초적자료형)정보는 버텍스,인덱스버퍼에 저장하여 파이프라인에 넘긴다.
                               버텍스버퍼에는 각 버텍스의 좌표, material정보(색 등)을 저장한다.

Input Assembler(IA) : 파이프라인에서 첫 스테이지에 있다. 리소르로부터 데이터를 읽어 들여 파이프라인안에 데이터를 제공
                      동시에 시스템 생성값(primitiveID, indicesID, vertexID등)을 생성하여 제공
					  이곳에는 버텍스 버퍼, 인덱스 버퍼, 입력 레이아웃 오브젝트, 프리미티브 타입 드을 설정

Vertex Shader(VS) : 쉐이더 입력으로 버텍스 데이터를 1개 취해, 좌표 변환등을 수행한 후, 버텍스 데이터 1개를 출력한다. 

Hull Shader(HS) : 11에 새롭게 추가된 테셀레이터 기능을 구성하는 쉐이더의 하나 쉐이더 입력으로 1~32개의 controllpoint를
                  취해, 컨트롤포인트,배치 정수, 테셀레이션 계수를 출력한다. 출력된 데이터는 다음 단계인 테셀레이터와 도메인에 넘겨짐

Tessellator : 11에 새롭게 추가된 기능, 정밀도가 낮은 프리미티브 데이터를 분할해서 정밀하게 해주는 구간
              쿼드 배치, 삼각형 배치, 선을 취해, 보다 정밀한 다수의 삼각형,선,점을 출력한다.

Domain Shader(DS) : 입력으로써 HS와 Tessellator에 출력을 취해, 쉐이더 출력으로서 배치 내의 각 Vertex좌표를 출력함.

[나중에 수업때 한 영상 보기]
Geometry Shader(GS) : 지오메트리 셰이더는 버텍스 셰이더에서는 할 수 없는 점이나, 선, 삼각형 등의 도형을 생성할 수 있는 기능이 있다.
                      지오메트리 셰이더 프로그램은 버텍스 셰이더가 수행되고 난 뒤에 수행된다. 지오메트리 셰이더 프로그램은 
					  버텍스 셰이더를 거쳐온 도형 정보를 입력받는데, 예를 들어 정점 3개가 지오메트리 셰이더에 들어오면, 
					  셰이더는 정점을 모두 없앨 수도 있고 더 많은 도형을 만들어 내보낼 수도 있다. 지오메트리 셰이더를 지나간 도형 정보는 
					  레스터라이즈를 거친 뒤 픽셀 셰이더를 통과하게 된다. 
					  지오메트리 셰이더는 테셀레이션이나 그림자 효과, 큐브 맵을 한번의 처리로 렌더링하는 데에 주로 쓰인다.

Stream Output : GS나 VS로부터 출력을 리소스 내의 버퍼에 쓰는 스테이지이다 버퍼에 쓰여진 데이터는 CPU에서 읽어들여 사용하거나
                파이프라인안의 입력으로서 사용할 수 있다.

Rasterize : 보이지 않는 프리미티브를 없애거나(컬링), 버텍스 값을 프리미티브 전체로 보완하여 프리미티브를 픽셀 데이터로 분해한다.

Pixel Shader : 픽셀 셰이더는 렌더링 될 각각의 픽셀들의 색을 계산한다. 때문에 픽셀 셰이더는 최종적으로 픽셀이 어떻게 보일지를 결정한다. 
               픽셀 셰이더는 간단하게 언제나 같은 색을 출력하는 간단한 일에서부터, 텍스처로부터 색을 읽어오거나, 빛을 적용하는 것,
			   범프 매핑, 그림자, 반사광, 투명처리 등 복잡한 현상 등을 수행할 수 있다.
			   픽셀 셰이더는 각각의 픽셀들이 렌더링될때 수행되기 때문에, 다른 픽셀들과 아무런 연관이 없다. 
			   픽셀 셰이더는 오직 한 픽셀만을 연산하기 때문에, 주변의 픽셀이나, 그리는 도형에 대한 정보를 알 수 없다.
			   이 때문에 픽셀 셰이더는 스스로 매우 복잡한 효과를 만들어 낼 수는 없다.
			   픽셀 셰이더는 픽셀의 색 말고도 깊이(Z버퍼에 쓰인다)나 또 다른 색(다른 렌더 목표물에 쓰인다)을 출력할 수 있다.

Output Merger : 픽셀 쉐이더로부터 출력된 픽셀 데이터나 깊이/스텐실 버퍼의 값을 사용하여 최종적으로 렌더링될 색을 결정한다.
                이때, 깊이/스텐실 테스트를 수행하여 실제로 렌더링할지 안할지를 결정

Depth/Stencil Buffer : 깊이 버퍼와 스텐실 버퍼는 깊이/스텐실 버퍼라는 1개의 리소스로 묶어서 취급한다.
                       [자세한 내용은 나중에 수업시간에]

*/