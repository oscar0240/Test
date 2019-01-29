#include "Framework.h"
#include "Window.h"
#include "IExecute.h"
#include "Renders/Progress.h"

//static변수는 전역에 초기화
IExecute* Window::mainExecute = NULL;
bool Window::bInitialize = false;
mutex* Window::criticalSection = NULL;

WPARAM Window::Run(IExecute * main)
{
	mainExecute = main;
	Create();

	D3DDesc desc;
	D3D::GetDesc(&desc);

	D3D::Create();
	DirectWrite::Create();
	Keyboard::Create();
	Mouse::Create();

	Time::Create();
	Time::Get()->Start();

	ImGui::Create(desc.Handle, D3D::GetDevice(), D3D::GetDC());
	ImGui::StyleColorsDark();

	criticalSection = new mutex();
	//람다식 찾아보기 (쓰레드안에는 함수가 들어가야하는데 람다식을 사용해서 익명함수를 넣어줌)
	//쓰레드 영역 
	thread t = thread([&]()
	{
		mainExecute->Initialize();

		//임계영역
		criticalSection->lock();
		//initialize가 끝나면 끝났다고 true
		bInitialize = true;
		criticalSection->unlock();
	});

	Progress::Create();

	MSG msg = { 0 };
	while (true)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
				break;

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			//굳이 criticalSection를쓴 이유는 bInitialize를 쓰면 또 락을 걸어야해서 상관이 없는 criticalSection변수를 씀
			if (criticalSection != NULL)
			{
				bool temp = false;

				criticalSection->lock();
				temp = bInitialize;
				criticalSection->unlock();

				//bInitialize = false라는건 쓰레드 영역이 아직 안끝남
				if (temp == false)
					ProgressRender();
				//bInitialize = true 쓰레드 영역이 끝남
				else
				{
					t.join();
					SAFE_DELETE(criticalSection);

					Progress::Delete();

					//쓰레드영역이 끄나면 실행
					mainExecute->Ready();
				}
			}
			else
			{
				MainRender();
			}
		}
	}
	mainExecute->Destroy();

	ImGui::Delete();
	Time::Delete();
	Mouse::Delete();
	Keyboard::Delete();
	DirectWrite::Delete();
	D3D::Delete();

	Destroy();

	return msg.wParam;
}

void Window::Create()
{
	D3DDesc desc;
	D3D::GetDesc(&desc);


	WNDCLASSEX wndClass;
	wndClass.cbClsExtra = 0;
	wndClass.cbWndExtra = 0;
	wndClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndClass.hIcon = LoadIcon(NULL, IDI_WINLOGO);
	wndClass.hIconSm = wndClass.hIcon;
	wndClass.hInstance = desc.Instance;
	wndClass.lpfnWndProc = (WNDPROC)WndProc;
	wndClass.lpszClassName = desc.AppName.c_str();
	wndClass.lpszMenuName = NULL;
	wndClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wndClass.cbSize = sizeof(WNDCLASSEX);

	WORD wHr = RegisterClassEx(&wndClass);
	assert(wHr != 0);

	if (desc.bFullScreen == true)
	{
		DEVMODE devMode = { 0 };
		devMode.dmSize = sizeof(DEVMODE);
		devMode.dmPelsWidth = (DWORD)desc.Width;
		devMode.dmPelsHeight = (DWORD)desc.Height;
		devMode.dmBitsPerPel = 32;
		devMode.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

		ChangeDisplaySettings(&devMode, CDS_FULLSCREEN);
	}

	desc.Handle = CreateWindowEx
	(
		WS_EX_APPWINDOW
		, desc.AppName.c_str()
		, desc.AppName.c_str()
		, WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_OVERLAPPEDWINDOW
		, CW_USEDEFAULT
		, CW_USEDEFAULT
		, CW_USEDEFAULT
		, CW_USEDEFAULT
		, NULL
		, (HMENU)NULL
		, desc.Instance
		, NULL
	);
	assert(desc.Handle != NULL);
	D3D::SetDesc(desc);


	RECT rect = { 0, 0, (LONG)desc.Width, (LONG)desc.Height };

	UINT centerX = (GetSystemMetrics(SM_CXSCREEN) - (UINT)desc.Width) / 2;
	UINT centerY = (GetSystemMetrics(SM_CYSCREEN) - (UINT)desc.Height) / 2;

	AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);
	MoveWindow
	(
		desc.Handle
		, centerX, centerY
		, rect.right - rect.left, rect.bottom - rect.top
		, TRUE
	);
	ShowWindow(desc.Handle, SW_SHOWNORMAL);
	SetForegroundWindow(desc.Handle);
	SetFocus(desc.Handle);

	ShowCursor(true);
}

void Window::Destroy()
{
	D3DDesc desc;
	D3D::GetDesc(&desc);

	if (desc.bFullScreen == true)
		ChangeDisplaySettings(NULL, 0);

	DestroyWindow(desc.Handle);

	UnregisterClass(desc.AppName.c_str(), desc.Instance);
}

LRESULT CALLBACK Window::WndProc(HWND handle, UINT message, WPARAM wParam, LPARAM lParam)
{
	Mouse::Get()->InputProc(message, wParam, lParam);

	if (ImGui::WndProc((UINT*)handle, message, wParam, lParam))
		return true;

	if (message == WM_SIZE)
	{
		ImGui::Invalidate();
		{
			if (mainExecute != NULL)
			{
				float width = (float)LOWORD(lParam);
				float height = (float)HIWORD(lParam);

				if (DirectWrite::Get() != NULL)
					DirectWrite::DeleteSurface();

				if (D3D::Get() != NULL)
					D3D::Get()->ResizeScreen(width, height);

				if (DirectWrite::Get() != NULL)
					DirectWrite::CreateSurface();

				mainExecute->ResizeScreen();
			}
		}
		ImGui::Validate();
	}

	if (message == WM_CLOSE || message == WM_DESTROY)
	{
		PostQuitMessage(0);

		return 0;
	}

	return DefWindowProc(handle, message, wParam, lParam);
}

//쓰레드안에 있는 Initialize가 실행되는 동안에는 이 함수가 실행된다
void Window::ProgressRender()
{
	Time::Get()->Update();

	if (ImGui::IsMouseHoveringAnyWindow() == false)
	{
		Keyboard::Get()->Update();
		Mouse::Get()->Update();
	}
	ImGui::Update();


	D3D::Get()->SetRenderTarget();
	D3D::Get()->Clear(D3DXCOLOR(0, 0, 0, 1));
	{
		Progress::Get()->Render();

		ImGui::Render();
	}
	D3D::Get()->Present();
}

//쓰레드가 끝나면 이 함수가 실행
void Window::MainRender()
{
	Time::Get()->Update();

	if (ImGui::IsMouseHoveringAnyWindow() == false)
	{
		Keyboard::Get()->Update();
		Mouse::Get()->Update();
	}

	mainExecute->Update();
	ImGui::Update();


	mainExecute->PreRender();

	D3D::Get()->SetRenderTarget();
	D3D::Get()->Clear(D3DXCOLOR(0, 0, 0, 1));
	{
		mainExecute->Render();
		ImGui::Render();

		DirectWrite::GetDC()->BeginDraw();
		{
			mainExecute->PostRender();
		}
		DirectWrite::GetDC()->EndDraw();
	}
	D3D::Get()->Present();
}
