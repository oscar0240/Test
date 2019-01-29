#pragma once

class Progress
{
public:
	//싱글톤으로 만듬(게임 전체에서 영향을 미칠만한 것만 싱글톤으로 만드는것이 좋다)
	static Progress* Get();

	static void Create();
	static void Delete();

	void Render();

	void Rate(float val) { rate = val; }

private:
	static Progress* instance;

	Progress();
	~Progress();

private:
	float rate;

};
