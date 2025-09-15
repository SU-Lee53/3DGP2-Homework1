#include "stdafx.h"
#include "StructuredBuffer.h"

StructuredBuffer::StructuredBuffer(ComPtr<ID3D12Device> pd3dDevice, ComPtr<ID3D12GraphicsCommandList> pd3dCommandList, UINT nDatas, size_t elementSize)
{
	m_nDatas = nDatas;

	m_pd3dSBuffer = ::CreateBufferResource(
		pd3dDevice,
		pd3dCommandList,
		NULL,
		m_nDatas * sizeof(elementSize),
		D3D12_HEAP_TYPE_UPLOAD,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		NULL
	);

	m_pd3dSBuffer->Map(0, NULL, (void**)&m_pMappedPtr);
}

StructuredBuffer::~StructuredBuffer()
{
	m_pd3dSBuffer->Unmap(0, NULL);
}

void StructuredBuffer::SetBufferToPipeline(ComPtr<ID3D12GraphicsCommandList> pd3dCommandList, UINT uiOffset, UINT uiElementSize, UINT uiRootParameterIndex)
{
	D3D12_GPU_VIRTUAL_ADDRESS d3dGPUAddress = m_pd3dSBuffer->GetGPUVirtualAddress();
	d3dGPUAddress += (uiOffset * uiElementSize);

	pd3dCommandList->SetGraphicsRootShaderResourceView(uiRootParameterIndex, d3dGPUAddress);
}