#pragma once
#include "Systems/IExecute.h"

class Main : public IExecute
{
public:
	//�츮�� window���� Initialize�� ������ �������� �����־��µ� �� ������
	//CPU���� GPU�� �����ͺ��簡 �̷�� ���°�(map,unmap,updateSubresource)�� ���� ������
	//�׷��� �����ͺ���� Ready �Լ��� ����Ѵ�
	//ex)�� �ؽ��ĵ� GPU�� �����ϴ� �ſ��� ready�� �����
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
