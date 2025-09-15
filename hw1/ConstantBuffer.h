#pragma once

class ConstantBuffer {
public:
	ConstantBuffer(ComPtr<ID3D12Device> pd3dDevice, ComPtr<ID3D12GraphicsCommandList> pd3dCommandList, UINT uiElementSize);
	~ConstantBuffer();

	template<typename T>
	void UpdateData(ComPtr<ID3D12GraphicsCommandList> pd3dCommandList, T* pData);
	void SetBufferToPipeline(ComPtr<ID3D12GraphicsCommandList> pd3dCommandList, UINT uiRootParameterIndex);

private:
	ComPtr<ID3D12Resource> m_pd3dCBuffer;
	void* m_pMappedPtr;

};

template<typename T>
inline void ConstantBuffer::UpdateData(ComPtr<ID3D12GraphicsCommandList> pd3dCommandList, T* pData)
{
	::memcpy(m_pMappedPtr, pData, sizeof(T));
}
