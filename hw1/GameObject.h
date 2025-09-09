#pragma once
#include "Mesh.h"
#include "Material.h"

class GameObject {

private:
	std::string m_strFrameName;

	std::shared_ptr<Mesh>	m_pMesh;
	
	std::vector<std::shared_ptr<Material>> m_pMaterials;

	XMFLOAT4X4 xmf4x4Transform;
	XMFLOAT4X4 xmf4x4World;

	std::shared_ptr<GameObject> m_pParent;
	std::vector<std::shared_ptr<GameObject>> m_pChildren;


public:
	static std::vector<MATERIALLOADINFO> LoadMaterialsInfoFromFile(ComPtr<ID3D12Device> pd3dDevice, ComPtr<ID3D12GraphicsCommandList> pd3dCommandList, std::ifstream& inFile);
	static std::shared_ptr<MESHLOADINFO> LoadMeshInfoFromFile(std::ifstream& inFile);

	static std::shared_ptr<GameObject> LoadFrameHierarchyFromFile(ComPtr<ID3D12Device> pd3dDevice, ComPtr<ID3D12GraphicsCommandList> pd3dCommandList, std::ifstream& inFile);
	static std::shared_ptr<GameObject> LoadGeometryFromFile(ComPtr<ID3D12Device> pd3dDevice, ComPtr<ID3D12GraphicsCommandList> pd3dCommandList, std::string strFileName);

};

