#pragma once

/*
	- 매 프레임 인스턴싱용 StructuredBuffer 를 만들지 않기위해 처음부터 매우 크게 만듬
	- 인스턴싱 데이터는 ID3D12Resource 의 D3D12_GPU_VIRTUAL_ADDRESS 를 가지고 포인터연산을 통해 
	  서로 다른 인스턴싱 그룹의 데이터를 하나의 버퍼에 전부 담도록 함

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
