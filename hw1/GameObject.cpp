#include "stdafx.h"
#include "GameObject.h"







/////////////////////
// Load From Files //
/////////////////////

std::string ReadStringFromFile(std::ifstream& inFile)
{
	BYTE nStrLength = 0;
	UINT nReads = 0;
	inFile.read((char*)&nStrLength, sizeof(BYTE));

	std::unique_ptr<char[]> pcstrRead;
	pcstrRead = std::make_unique<char[]>(nStrLength);
	inFile.read(pcstrRead.get(), nStrLength);

	return std::string(pcstrRead.get(), nStrLength);	// [pcstrRead, pcstrRead + nStrLength)
}

std::vector<MATERIALLOADINFO> GameObject::LoadMaterialsInfoFromFile(ComPtr<ID3D12Device> pd3dDevice, ComPtr<ID3D12GraphicsCommandList> pd3dCommandList, std::ifstream& inFile)
{
	return std::vector<MATERIALLOADINFO>();
}

std::shared_ptr<MESHLOADINFO> GameObject::LoadMeshInfoFromFile(std::ifstream& inFile)
{
	std::string strRead;

	std::shared_ptr<MESHLOADINFO> pMeshInfo = std::make_shared<MESHLOADINFO>();

	while (true) {
		strRead = ::ReadStringFromFile(inFile);
		if (strRead == "<Bounds>:") {
			inFile.read((char*)&pMeshInfo->xmf3AABBCenter, sizeof(XMFLOAT3));
			inFile.read((char*)&pMeshInfo->xmf3AABBExtents, sizeof(XMFLOAT3));
		}
		else if (strRead == "<Positions>:") {
			int nPositions;
			inFile.read((char*)&nPositions, sizeof(int));
			if (nPositions > 0) {
				pMeshInfo->nType |= VERTEX_TYPE_POSITION;
				pMeshInfo->xmf3Positions.resize(nPositions);
				inFile.read((char*)pMeshInfo->xmf3Positions.data(), sizeof(decltype(pMeshInfo->xmf3Positions)::value_type) * nPositions);
			}
		}
		else if (strRead == "<Colors>:") {
			int nColors;
			inFile.read((char*)&nColors, sizeof(int));
			if (nColors > 0) {
				pMeshInfo->nType |= VERTEX_TYPE_POSITION;
				pMeshInfo->xmf3Positions.resize(nColors);
				inFile.read((char*)pMeshInfo->xmf4Colors.data(), sizeof(decltype(pMeshInfo->xmf4Colors)::value_type) * nColors);
			}
		}
		else if (strRead == "<Normals>:") {
			int nNormals;
			inFile.read((char*)&nNormals, sizeof(int));
			if (nNormals > 0) {
				pMeshInfo->nType |= VERTEX_TYPE_POSITION;
				pMeshInfo->xmf3Positions.resize(nNormals);
				inFile.read((char*)pMeshInfo->xmf3Normals.data(), sizeof(decltype(pMeshInfo->xmf3Normals)::value_type) * nNormals);
			}
		}
		else if (strRead == "<Indices>:") {
			int nIndices;
			inFile.read((char*)&nIndices, sizeof(int));
			if (nIndices > 0)
			{
				pMeshInfo->nIndices.resize(nIndices);
				inFile.read((char*)pMeshInfo->nIndices.data(), sizeof(decltype(pMeshInfo->nIndices)::value_type) * nIndices);
			}
		}
		else if (strRead == "<SubMeshes>:") {
			int nSubMeshes;
			inFile.read((char*)&nSubMeshes, sizeof(int));
			if (nSubMeshes > 0) {
				pMeshInfo->nSubMeshes.resize(nSubMeshes);
				for (int i = 0; i < pMeshInfo->nSubMeshes.size(); ++i) {
					strRead = ::ReadStringFromFile(inFile);
					if (strRead == "<SubMesh>") {
						// TODO : TOMORROW
					}

				}
			}
		}
		else if ((strRead == "</Mesh>"))
		{
			break;
		}


	}

	return pMeshInfo;
}

std::shared_ptr<GameObject> GameObject::LoadFrameHierarchyFromFile(ComPtr<ID3D12Device> pd3dDevice, ComPtr<ID3D12GraphicsCommandList> pd3dCommandList, std::ifstream& inFile)
{
	std::string strRead;

	int nFrames = 0;
	std::shared_ptr<GameObject> pGameObject;

	while (true) {
		strRead = ::ReadStringFromFile(inFile);

		if (strRead == "<Frame>:") {
			pGameObject = std::make_shared<GameObject>();

			inFile.read((char*)&nFrames, sizeof(int));
			pGameObject->m_strFrameName = ::ReadStringFromFile(inFile);
		}
		else if (strRead == "<Transform>:") {
			XMFLOAT3 xmf3Position, xmf3Rotation, xmf3Scale;
			XMFLOAT4 xmf4Quaternion;
			inFile.read((char*)&xmf3Position, sizeof(XMFLOAT3));
			inFile.read((char*)&xmf3Rotation, sizeof(XMFLOAT3)); //Euler Angle
			inFile.read((char*)&xmf3Scale, sizeof(XMFLOAT3));
			inFile.read((char*)&xmf4Quaternion, sizeof(XMFLOAT4)); //Quaternion
		}
		else if (strRead == "<TransformMatrix>:") {
			inFile.read((char*)&pGameObject->xmf4x4Transform, sizeof(XMFLOAT4X4));
		}
		else if (strRead == "<Mesh>:") {
			std::shared_ptr<MESHLOADINFO> pMeshLoadInfo = GameObject::LoadMeshInfoFromFile(inFile);
			if (pMeshLoadInfo) {
				std::shared_ptr<Mesh> pMesh;
				if (pMeshLoadInfo->eType & VERTEX_TYPE_NORMAL) {
					pMesh = std::make_shared<IlluminatedMesh>(pd3dDevice, pd3dCommandList, *pMeshLoadInfo);
				}
				if (pMesh) {
					pGameObject->m_pMesh = pMesh;
				}
			}
		}
		else if (strRead == "<Materials>:") {
			std::vector<MATERIALLOADINFO> materialInfos = GameObject::LoadMaterialsInfoFromFile(pd3dDevice, pd3dCommandList, inFile);
			
			if (materialInfos.size() != 0) {
				pGameObject->m_pMaterials.reserve(materialInfos.size());

				for (const auto& materialInfo : materialInfos) {
					std::shared_ptr<Material> pMaterial = std::make_shared<Material>();

					std::shared_ptr<MaterialColors> pMaterialColors = std::make_shared<MaterialColors>(materialInfo);
					pMaterial->SetMaterialColors(pMaterialColors);
					
					if (pGameObject->m_pMesh->GetType() & VERTEX_TYPE_NORMAL) {
						pMaterial->SetIlluminatedShader();
					}

					pGameObject->m_pMaterials.push_back(pMaterial);
				}
			}
			
		}
		else if (strRead == "<Children>:") {
			int nChildren;
			inFile.read((char*)&nChildren, sizeof(int));
			pGameObject->m_pChildren.reserve(nChildren);
			if (nChildren > 0) {
				for (int i = 0; i < nChildren; ++i) {
					std::shared_ptr<GameObject> pChild = GameObject::LoadFrameHierarchyFromFile(pd3dDevice, pd3dCommandList, inFile);
					pGameObject->m_pChildren.push_back(pChild);
				}
			}


		}
		else if ((strRead == "</Frame>")){
			break;
		}
	}

	return pGameObject;
}

std::shared_ptr<GameObject> GameObject::LoadGeometryFromFile(ComPtr<ID3D12Device> pd3dDevice, ComPtr<ID3D12GraphicsCommandList> pd3dCommandList, std::string strFileName)
{
	std::ifstream inFile{ strFileName, std::ios::binary };
	if (!inFile) {
		__debugbreak();
	}

	std::shared_ptr<GameObject> pGameObject;
	
	std::string strRead;
	while (true) {
		strRead = ::ReadStringFromFile(inFile);

		if (strRead == "<Hierarchy>:") {
			pGameObject = GameObject::LoadFrameHierarchyFromFile(pd3dDevice, pd3dCommandList, inFile);
		}
		else if (strRead == "</Hierarchy>:") {
			break;
		}
	}

	return std::shared_ptr<GameObject>();
}
