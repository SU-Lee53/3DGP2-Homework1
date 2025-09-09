#include "stdafx.h"
#include "Mesh.h"

Mesh::Mesh(ComPtr<ID3D12Device> pd3dDevice, ComPtr<ID3D12GraphicsCommandList> pd3dCommandList, const MESHLOADINFO& meshLoadInfo)
{
	if (meshLoadInfo.eType & VERTEX_TYPE_POSITION) {
		m_pd3dPositionBuffer = ::CreateBufferResource(
			pd3dDevice, pd3dCommandList, 
			(void*)meshLoadInfo.xmf3Positions.data(), 
			sizeof(XMFLOAT3) * meshLoadInfo.xmf3Positions.size(), 
			D3D12_HEAP_TYPE_DEFAULT, 
			D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
			m_pd3dPositionUploadBuffer
		);
	}




}

IlluminatedMesh::IlluminatedMesh(ComPtr<ID3D12Device> pd3dDevice, ComPtr<ID3D12GraphicsCommandList> pd3dCommandList, const MESHLOADINFO& meshLoadInfo)
	: Mesh(pd3dDevice, pd3dCommandList, meshLoadInfo)
{
}

IlluminatedMesh::~IlluminatedMesh()
{
}

void IlluminatedMesh::ReleaseUploadBuffers()
{
}

void IlluminatedMesh::Render(ComPtr<ID3D12GraphicsCommandList> pd3dCommandList, int nSubSet)
{
}
