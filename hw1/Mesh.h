#pragma once
#include "InputLayout.h"

enum VERTEX_TYPE : UINT {
	VERTEX_TYPE_POSITION = 1,
	VERTEX_TYPE_NORMAL = (VERTEX_TYPE_POSITION << 1),
	VERTEX_TYPE_COLOR = (VERTEX_TYPE_NORMAL << 1),


	VERTEX_TYPE_UNDEFINED = 0
};

struct MESHLOADINFO {
	std::string				strMeshName;

	UINT					nType;

	XMFLOAT3				xmf3AABBCenter = XMFLOAT3(0.0f, 0.0f, 0.0f);
	XMFLOAT3				xmf3AABBExtents = XMFLOAT3(0.0f, 0.0f, 0.0f);

	std::vector<XMFLOAT3>	xmf3Positions;
	std::vector<XMFLOAT4>	xmf4Colors;
	std::vector<XMFLOAT3>	xmf3Normals;

	std::vector<UINT>		Indices;

	std::vector<std::vector<UINT>> SubMeshes;
};

class Mesh {
public:
	Mesh(ComPtr<ID3D12Device> pd3dDevice, ComPtr<ID3D12GraphicsCommandList> pd3dCommandList, const MESHLOADINFO& meshLoadInfo);

	VERTEX_TYPE GetType() { return (VERTEX_TYPE)m_nType; }

	virtual void ReleaseUploadBuffers();

protected:
	D3D12_PRIMITIVE_TOPOLOGY	m_d3dPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	UINT						m_nSlot = 0;
	UINT						m_nVertices = 0;
	UINT						m_nOffset = 0;

	UINT						m_nType = VERTEX_TYPE_UNDEFINED;

protected:
	ComPtr<ID3D12Resource>		m_pd3dPositionBuffer = nullptr;
	ComPtr<ID3D12Resource>		m_pd3dPositionUploadBuffer = nullptr;
	D3D12_VERTEX_BUFFER_VIEW	m_d3dPositionBufferView;
	
	UINT									m_nSubMeshes = 0;
	std::vector<int>						m_nSubSetIndices;
	std::vector<ComPtr<ID3D12Resource>>		m_pd3dSubSetIndexBuffers;
	std::vector<ComPtr<ID3D12Resource>>		m_pd3dSubSetIndexUploadBuffers;
	std::vector<D3D12_INDEX_BUFFER_VIEW>	m_d3dSubSetIndexBufferViews;

public:
	virtual void Render(ComPtr<ID3D12GraphicsCommandList> pd3dCommandList) {}
	virtual void Render(ComPtr<ID3D12GraphicsCommandList> pd3dCommandList, int nSubSet, int nInstanceCount = 1);
};

class IlluminatedMesh : public Mesh
{
public:
	IlluminatedMesh(ComPtr<ID3D12Device> pd3dDevice, ComPtr<ID3D12GraphicsCommandList> pd3dCommandList, const MESHLOADINFO& meshLoadInfo);
	virtual ~IlluminatedMesh();

	virtual void ReleaseUploadBuffers() override;

protected:
	ComPtr<ID3D12Resource> m_pd3dNormalBuffer = nullptr;
	ComPtr<ID3D12Resource> m_pd3dNormalUploadBuffer = nullptr;
	D3D12_VERTEX_BUFFER_VIEW	m_d3dNormalBufferView;

public:
	virtual void Render(ComPtr<ID3D12GraphicsCommandList> pd3dCommandList, int nSubSet, int nInstanceCount = 1) override;
};
