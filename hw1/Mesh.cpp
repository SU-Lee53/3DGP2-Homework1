#include "stdafx.h"
#include "Mesh.h"

//////////
// Mesh //
//////////

Mesh::Mesh(ComPtr<ID3D12Device> pd3dDevice, ComPtr<ID3D12GraphicsCommandList> pd3dCommandList, const MESHLOADINFO& meshLoadInfo)
{
	m_nVertices = meshLoadInfo.xmf3Positions.size();
	m_nType = meshLoadInfo.nType;

	m_pd3dPositionBuffer = ::CreateBufferResource(
		pd3dDevice,
		pd3dCommandList,
		(void*)meshLoadInfo.xmf3Positions.data(),
		sizeof(XMFLOAT3) * m_nVertices,
		D3D12_HEAP_TYPE_DEFAULT,
		D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
		m_pd3dPositionUploadBuffer.GetAddressOf()
	);

	m_d3dPositionBufferView.BufferLocation = m_pd3dPositionBuffer->GetGPUVirtualAddress();
	m_d3dPositionBufferView.StrideInBytes = sizeof(XMFLOAT3);
	m_d3dPositionBufferView.SizeInBytes = sizeof(XMFLOAT3) * m_nVertices;

	m_nSubMeshes = meshLoadInfo.SubMeshes.size();
	if (m_nSubMeshes > 0) {
		m_nSubSetIndices.resize(m_nSubMeshes);
		m_pd3dSubSetIndexBuffers.resize(m_nSubMeshes);
		m_pd3dSubSetIndexUploadBuffers.resize(m_nSubMeshes);
		m_d3dSubSetIndexBufferViews.resize(m_nSubMeshes);

		for (int i = 0; i < m_nSubMeshes; ++i) {
			m_nSubSetIndices[i] = meshLoadInfo.SubMeshes[i].size();
			m_pd3dSubSetIndexBuffers[i] = ::CreateBufferResource(
				pd3dDevice,
				pd3dCommandList,
				(void*)meshLoadInfo.SubMeshes[i].data(),
				sizeof(UINT) * m_nSubSetIndices[i],
				D3D12_HEAP_TYPE_DEFAULT,
				D3D12_RESOURCE_STATE_INDEX_BUFFER,
				m_pd3dSubSetIndexUploadBuffers[i].GetAddressOf()
			);

			m_d3dSubSetIndexBufferViews[i].BufferLocation = m_pd3dSubSetIndexBuffers[i]->GetGPUVirtualAddress();
			m_d3dSubSetIndexBufferViews[i].Format = DXGI_FORMAT_R32_UINT;
			m_d3dSubSetIndexBufferViews[i].SizeInBytes = sizeof(UINT) * m_nSubSetIndices[i];
		}
	}

}

void Mesh::ReleaseUploadBuffers()
{
	if (m_pd3dPositionUploadBuffer) {
		m_pd3dPositionUploadBuffer.Reset();
	}

	if (m_nSubMeshes && m_pd3dSubSetIndexBuffers[0]) {
		for (int i = 0; i < m_nSubMeshes; ++i) {
			if (m_pd3dSubSetIndexUploadBuffers[i]) {
				m_pd3dSubSetIndexUploadBuffers[i].Reset();
			}
		}

		m_pd3dSubSetIndexUploadBuffers.clear();
	}
}

void Mesh::AddToRenderMap(ComPtr<ID3D12GraphicsCommandList> pd3dCommandList, int nSubSet, int nInstanceCount)
{
	pd3dCommandList->IASetPrimitiveTopology(m_d3dPrimitiveTopology);
	pd3dCommandList->IASetVertexBuffers(m_nSlot, 1, &m_d3dPositionBufferView);
	if ((m_nSubMeshes > 0) && (nSubSet < m_nSubMeshes))
	{
		pd3dCommandList->IASetIndexBuffer(&(m_d3dSubSetIndexBufferViews[nSubSet]));
		pd3dCommandList->DrawIndexedInstanced(m_nSubSetIndices[nSubSet], nInstanceCount, 0, 0, 0);
	}
	else
	{
		pd3dCommandList->DrawInstanced(m_nVertices, nInstanceCount, m_nOffset, 0);
	}
}

/////////////////////
// IlluminatedMesh //
/////////////////////

IlluminatedMesh::IlluminatedMesh(ComPtr<ID3D12Device> pd3dDevice, ComPtr<ID3D12GraphicsCommandList> pd3dCommandList, const MESHLOADINFO& meshLoadInfo)
	: Mesh(pd3dDevice, pd3dCommandList, meshLoadInfo)
{
	m_pd3dNormalBuffer = ::CreateBufferResource(
		pd3dDevice,
		pd3dCommandList,
		(void*)meshLoadInfo.xmf3Normals.data(),
		sizeof(XMFLOAT3) * m_nVertices,
		D3D12_HEAP_TYPE_DEFAULT,
		D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
		m_pd3dNormalUploadBuffer.GetAddressOf()
	);

	m_d3dNormalBufferView.BufferLocation = m_pd3dNormalBuffer->GetGPUVirtualAddress();
	m_d3dNormalBufferView.StrideInBytes = sizeof(XMFLOAT3);
	m_d3dNormalBufferView.SizeInBytes = sizeof(XMFLOAT3) * m_nVertices;
}

IlluminatedMesh::~IlluminatedMesh()
{
}

void IlluminatedMesh::ReleaseUploadBuffers()
{
	Mesh::ReleaseUploadBuffers();

	if (m_pd3dNormalUploadBuffer) {
		m_pd3dNormalUploadBuffer.Reset();
	}
}

void IlluminatedMesh::AddToRenderMap(ComPtr<ID3D12GraphicsCommandList> pd3dCommandList, int nSubSet, int nInstanceCount)
{
	pd3dCommandList->IASetPrimitiveTopology(m_d3dPrimitiveTopology);
	D3D12_VERTEX_BUFFER_VIEW pVertexBufferViews[2] = { m_d3dPositionBufferView, m_d3dNormalBufferView };
	pd3dCommandList->IASetVertexBuffers(m_nSlot, 2, pVertexBufferViews);

	if ((m_nSubMeshes > 0) && (nSubSet < m_nSubMeshes))
	{
		pd3dCommandList->IASetIndexBuffer(&(m_d3dSubSetIndexBufferViews[nSubSet]));
		pd3dCommandList->DrawIndexedInstanced(m_nSubSetIndices[nSubSet], nInstanceCount, 0, 0, 0);
	}
	else
	{
		pd3dCommandList->DrawInstanced(m_nVertices, nInstanceCount, m_nOffset, 0);
	}
}
