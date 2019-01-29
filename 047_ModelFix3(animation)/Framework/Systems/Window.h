#pragma once

class Window
{
public:
	static WPARAM Run(class IExecute* main);

private:
	static void Create();
	static void Destroy();

	static LRESULT CALLBACK WndProc(HWND handle, UINT message, WPARAM wParam, LPARAM lParam);

	static void ProgressRender();
	static void MainRender();

private:
	static class IExecute* mainExecute;

	static bool bInitialize;
	static mutex* criticalSection;

	//initialize영역은 쓰레드 영역에서 실행하고 
	//ready영역은 쓰레드 영역이 아닌곳에서 실행(buffer만들고 GPU로 복사해주는건 Ready에)
};