#pragma once

/*
	- �� ������ �ν��Ͻ̿� StructuredBuffer �� ������ �ʱ����� ó������ �ſ� ũ�� ����
	- �ν��Ͻ� �����ʹ� ID3D12Resource �� D3D12_GPU_VIRTUAL_ADDRESS �� ������ �����Ϳ����� ���� 
	  ���� �ٸ� �ν��Ͻ� �׷��� �����͸� �ϳ��� ���ۿ� ���� �㵵�� ��

*/

class StructuredBuffer {
public:
	StructuredBuffer(ComPtr<ID3D12Device> pd3dDevice, ComPtr<ID3D12GraphicsCommandList> pd3dCommandList, UINT nDatas, size_t elementSize);
	~StructuredBuffer();

	template<typename T>
	void UpdateData(std::vector<T> data, UINT offset = 0);

	template<typename T>
	void UpdateData(const T& const data, UINT offset, UINT nDatas);
	
	template<typename T>
	void UpdateData(const T& const data, UINT index);

	void SetBufferToPipeline(ComPtr<ID3D12GraphicsCommandList> pd3dCommandList, UINT uiOffset, UINT uiElementSize, UINT uiRootParameterIndex);

private: 
	ComPtr<ID3D12Resource> m_pd3dSBuffer;
	UINT m_nDatas = 0;
	void* m_pMappedPtr;

};

template<typename T>
inline void StructuredBuffer::UpdateData(std::vector<T> data, UINT offset)
{
	assert(data.size() < m_nDatas);
	T* pMappedPtr = reinterpret_cast<T*>(m_pMappedPtr);
	::memcpy(pMappedPtr + offset, data.data(), data.size() * sizeof(T));
}

template<typename T>
inline void StructuredBuffer::UpdateData(const T& const data, UINT offset, UINT nDatas)
{
	assert(offset + nDatas < m_nDatas);
	T* pMappedPtr = reinterpret_cast<T*>(m_pMappedPtr);
	::memcpy(pMappedPtr + offset, &data, nDatas * sizeof(T));
}

template<typename T>
inline void StructuredBuffer::UpdateData(const T& const data, UINT index)
{
	assert(index < m_nDatas);
	T* pMappedPtr = reinterpret_cast<T*>(m_pMappedPtr);
	::memcpy(pMappedPtr + index, &data, sizeof(T));
}
