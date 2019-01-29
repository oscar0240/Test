#pragma once
#include "Systems/IExecute.h"

class Main : public IExecute
{
public:
	//우리가 window에서 Initialize를 쓰레드 영역으로 묶어주었는데 이 영역에
	//CPU에서 GPU로 데이터복사가 이루어 지는것(map,unmap,updateSubresource)이 들어가면 터진다
	//그래서 데이터복사는 Ready 함수에 써야한다
	//ex)로 텍스쳐도 GPU로 복사하는 거여서 ready에 써야함
	void Initialize() override;
	void Ready() override;
	void Destroy() override;
	void Update() override;
	void PreRender() override;
	void Render() override;
	void PostRender() override;
	void ResizeScreen() override;

private:
	void Push(IExecute* execute);

private:
	vector<IExecute *> executes;
};
