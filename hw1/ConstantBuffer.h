#pragma once

class ConstantBuffer {
	ConstantBuffer(ComPtr<ID3D12Device> pd3dDevice, ComPtr<ID3D12GraphicsCommandList> pd3dCommandList, UINT uiElementSize);
	~ConstantBuffer();

	template<typename T>
	void UpdateShaderVariables(ComPtr<ID3D12GraphicsCommandList> pd3dCommandList, T* pData, UINT uiRootParameterIndex);

private:
	ComPtr<ID3D12Resource> m_pd3dCBuffer;
	void* m_pMappedPtr;

};

template<typename T>
inline void ConstantBuffer::UpdateShaderVariables(ComPtr<ID3D12GraphicsCommandList> pd3dCommandList, T* pData, UINT uiRootParameterIndex)
{
	::memcpy(m_pMappedPtr, pData, sizeof(T));
	pd3dCommandList->SetGraphicsRootConstantBufferView(uiRootParameterIndex, m_pd3dCBuffer->GetGPUVirtualAddress());

}
