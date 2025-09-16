#include "stdafx.h"
#include "StructuredBuffer.h"

StructuredBuffer::StructuredBuffer(ComPtr<ID3D12Device> pd3dDevice, ComPtr<ID3D12GraphicsCommandList> pd3dCommandList, UINT nDatas, size_t elementSize, bool bCreateView)
{
	Create(pd3dDevice, pd3dCommandList, nDatas, elementSize, bCreateView);
}

StructuredBuffer::~StructuredBuffer()
{
	m_pd3dSBuffer->Unmap(0, NULL);
}

void StructuredBuffer::Create(ComPtr<ID3D12Device> pd3dDevice, ComPtr<ID3D12GraphicsCommandList> pd3dCommandList, UINT nDatas, size_t elementSize, bool bCreateView)
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


	if (bCreateView) {
		HRESULT hr;

		D3D12_DESCRIPTOR_HEAP_DESC heapDesc;
		{
			heapDesc.NumDescriptors = 1;
			heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
			heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
			heapDesc.NodeMask = 0;
		}

		hr = pd3dDevice->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(m_pd3dSRVHeap.GetAddressOf()));
		if (FAILED(hr)) {
			__debugbreak();
		}

		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
		{
			srvDesc.Format = DXGI_FORMAT_UNKNOWN;
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
			srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			srvDesc.Buffer.FirstElement = 0;
			srvDesc.Buffer.NumElements = m_nDatas;
			srvDesc.Buffer.StructureByteStride = elementSize;
			srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
		}
		pd3dDevice->CreateShaderResourceView(m_pd3dSBuffer.Get(), &srvDesc, m_pd3dSRVHeap->GetCPUDescriptorHandleForHeapStart());
	}
}

void StructuredBuffer::SetBufferToPipeline(ComPtr<ID3D12GraphicsCommandList> pd3dCommandList, UINT uiOffset, UINT uiElementSize, UINT uiRootParameterIndex) const
{
	D3D12_GPU_VIRTUAL_ADDRESS d3dGPUAddress = m_pd3dSBuffer->GetGPUVirtualAddress();
	d3dGPUAddress += (uiOffset * uiElementSize);

	pd3dCommandList->SetGraphicsRootShaderResourceView(uiRootParameterIndex, d3dGPUAddress);
}

D3D12_CPU_DESCRIPTOR_HANDLE StructuredBuffer::GetCPUDescriptorHandle(UINT offset) const
{
	assert(m_pd3dSRVHeap);
	D3D12_CPU_DESCRIPTOR_HANDLE d3dCPUHandle = m_pd3dSRVHeap->GetCPUDescriptorHandleForHeapStart();
	d3dCPUHandle.ptr += (GameFramework::g_uiDescriptorHandleIncrementSize * offset);
	return d3dCPUHandle;
}

D3D12_GPU_DESCRIPTOR_HANDLE StructuredBuffer::GetGPUDescriptorHandle(UINT offset) const
{
	assert(m_pd3dSRVHeap);
	D3D12_GPU_DESCRIPTOR_HANDLE d3dGPUHandle = m_pd3dSRVHeap->GetGPUDescriptorHandleForHeapStart();
	d3dGPUHandle.ptr += (GameFramework::g_uiDescriptorHandleIncrementSize * offset);
	return d3dGPUHandle;
}
