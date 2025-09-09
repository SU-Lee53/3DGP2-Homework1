#pragma once
#include "Mesh.h"
#include "Material.h"

class GameObject {

public:
	static std::vector<MATERIALLOADINFO> LoadMaterialsInfoFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, FILE* pInFile);
	static MESHLOADINFO LoadMeshInfoFromFile(FILE* pInFile);

	static std::shared_ptr<GameObject> LoadFrameHierarchyFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, FILE* pInFile);
	static std::shared_ptr<GameObject> LoadGeometryFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, char* pstrFileName);

};

