#pragma once

class ConstantBuffer {
public:
	ConstantBuffer() = default;
	ConstantBuffer(ComPtr<ID3D12Device> pd3dDevice, ComPtr<ID3D12GraphicsCommandList> pd3dCommandList, UINT uiElementSize, bool bCreateView = false);
	~ConstantBuffer();

public:
	void Create(ComPtr<ID3D12Device> pd3dDevice, ComPtr<ID3D12GraphicsCommandList> pd3dCommandList, UINT uiElementSize, bool bCreateView = false);

	template<typename T>
	void UpdateData(ComPtr<ID3D12GraphicsCommandList> pd3dCommandList, T* pData);
	void SetBufferToPipeline(ComPtr<ID3D12GraphicsCommandList> pd3dCommandList, UINT uiRootParameterIndex) const;

	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle() const;
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle() const;

private:
	ComPtr<ID3D12DescriptorHeap>	m_pd3dCBVHeap = nullptr;
	ComPtr<ID3D12Resource>			m_pd3dCBuffer = nullptr;
	void*							m_pMappedPtr = nullptr;

};

template<typename T>
inline void ConstantBuffer::UpdateData(ComPtr<ID3D12GraphicsCommandList> pd3dCommandList, T* pData)
{
	::memcpy(m_pMappedPtr, pData, sizeof(T));
}
