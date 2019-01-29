#include "Framework.h"
#include "CBuffer.h"

/*
CPU���� GPU�� �����͸� �Ѱ��ְ�(�����͸� �������ִ°�) �ٽ� CPU�� �Ѱ��ִ� ������ Buffer���� �����Ѵ�
buffer�� �����Ҷ� D3D11_BUFFER_DESC desc�� �����
desc.Usage�� �ִµ�
              GPU R / W         CPU R / W
Defualt           Y   Y                     
Dynamic           Y                     Y
Staging           Y   Y             Y   Y 
Immutable         Y 

GPU read�� CPU���� GPU�� ó���� ����� return ���� �� �ִ�
GPU write�� CPU���� GPU�� ���Ⱑ ����
CPU read�� GPU���� CPU�� ó���� ����� return ���� �� �ִ�
CPU write�� GPU���� CPU�� ���Ⱑ ����

Staging�� ���� ������. �ɼ��� ������ �������� �ӵ��� ������
Immutable -> �����Ͱ� ó�� �ʱ�ȭ �� ������ ������ �ʰ� ���Ϲ��� �ʿ䵵 ������(ex) IndexBuffer)

*/

CBuffer::CBuffer(Shader * shader, string bufferName, void * pData, UINT dataSize)
	: shader(shader), name(bufferName), data(pData), dataSize(dataSize)
{
	D3D11_BUFFER_DESC desc;
	//Usage -> Defualt, Dynamic, Staging, Immutable
	desc.Usage = D3D11_USAGE_DYNAMIC;
	desc.ByteWidth = dataSize;
	//BindFlags buffer�� ��� ������� ���� vertex�� bind_vertex_buffer
	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	//D3D11_CPU_ACCESS_WRITE -> dynamic�� ���� ���ش�
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	desc.MiscFlags = 0;
	desc.StructureByteStride = 0;

	//�ϴ� �ʱ�ȭ �ϴµ� ���� �ʿ�ÿ� ���� ���Ƿ� �ٲ���Ҷ� map unmap���� ���� �ִµ�
	HRESULT hr = D3D::GetDevice()->CreateBuffer(&desc, NULL, &buffer);
	assert(SUCCEEDED(hr));

	Change();

	CBuffers::buffers.push_back(this);
}

CBuffer::~CBuffer()
{
	SAFE_RELEASE(buffer);
}

void CBuffer::Change()
{
	bChanged = true;
}

void CBuffer::Changed()
{
	if (bChanged == true)
	{
		D3D11_MAPPED_SUBRESOURCE subResource;
		HRESULT hr = D3D::GetDC()->Map
		(
			buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &subResource
		);
		assert(SUCCEEDED(hr));

		memcpy(subResource.pData, data, dataSize);
		D3D::GetDC()->Unmap(buffer, 0);

		hr = shader->AsConstantBuffer(name)->SetConstantBuffer(buffer);
		assert(SUCCEEDED(hr));

		bChanged = false;
	}
}

///////////////////////////////////////////////////////////////////////////////

vector<CBuffer *> CBuffers::buffers;

void CBuffers::Update()
{
	for (CBuffer* buffer : buffers)
		buffer->Changed();
}
