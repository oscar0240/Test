#pragma once

class IExecute
{
public:
	virtual void Initialize() = 0;
	virtual void Ready() = 0; 
	virtual void Destroy() = 0;

	virtual void Update() = 0;
	virtual void PreRender() = 0;
	virtual void Render() = 0;
	virtual void PostRender() = 0;

	virtual void ResizeScreen() = 0;
};

/*
모든 랜더링하는 것들은 이 클래스로 상속받을 거다.
Ready()함수 -> 로딩화면 비슷하게 만들려고 만듬
*/
