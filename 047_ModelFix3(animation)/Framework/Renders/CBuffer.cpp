#include "Framework.h"
#include "CBuffer.h"

/*
CPU에서 GPU로 데이터를 넘겨주고(데이터를 복사해주는것) 다시 CPU로 넘겨주는 과정은 Buffer들이 수행한다
buffer를 생성할때 D3D11_BUFFER_DESC desc를 만들고
desc.Usage가 있는데
              GPU R / W         CPU R / W
Defualt           Y   Y                     
Dynamic           Y                     Y
Staging           Y   Y             Y   Y 
Immutable         Y 

GPU read란 CPU에서 GPU로 처리된 결과를 return 받을 수 있다
GPU write란 CPU에서 GPU로 쓰기가 가능
CPU read란 GPU에서 CPU로 처리된 결과를 return 받을 수 있다
CPU write란 GPU에서 CPU로 쓰기가 가능

Staging이 제일 느리다. 옵션이 적으면 적을수록 속도가 빠르다
Immutable -> 데이터가 처음 초기화 한 값에서 변하지 않고 리턴받을 필요도 없을때(ex) IndexBuffer)

*/

CBuffer::CBuffer(Shader * shader, string bufferName, void * pData, UINT dataSize)
	: shader(shader), name(bufferName), data(pData), dataSize(dataSize)
{
	D3D11_BUFFER_DESC desc;
	//Usage -> Defualt, Dynamic, Staging, Immutable
	desc.Usage = D3D11_USAGE_DYNAMIC;
	desc.ByteWidth = dataSize;
	//BindFlags buffer가 어디에 연결될지 결정 vertex면 bind_vertex_buffer
	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	//D3D11_CPU_ACCESS_WRITE -> dynamic을 쓸때 써준다
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	desc.MiscFlags = 0;
	desc.StructureByteStride = 0;

	//일단 초기화 하는데 내가 필요시에 값을 임의로 바꿔야할때 map unmap으로 값을 넣는데
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
