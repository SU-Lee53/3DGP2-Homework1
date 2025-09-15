#include "stdafx.h"
#include "ConstantBuffer.h"

ConstantBuffer::ConstantBuffer(ComPtr<ID3D12Device> pd3dDevice, ComPtr<ID3D12GraphicsCommandList> pd3dCommandList, UINT uiElementSize)
{
	m_pd3dCBuffer = ::CreateBufferResource(
		pd3dDevice,
		pd3dCommandList,
		nullptr,
		uiElementSize,
		D3D12_HEAP_TYPE_UPLOAD,
		D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
		nullptr
	);

	m_pd3dCBuffer->Map(0, NULL, &m_pMappedPtr);
}

ConstantBuffer::~ConstantBuffer()
{
	m_pd3dCBuffer->Unmap(0, NULL);
}

void ConstantBuffer::SetBufferToPipeline(ComPtr<ID3D12GraphicsCommandList> pd3dCommandList, UINT uiRootParameterIndex)
{
	pd3dCommandList->SetGraphicsRootConstantBufferView(uiRootParameterIndex, m_pd3dCBuffer->GetGPUVirtualAddress());
}
