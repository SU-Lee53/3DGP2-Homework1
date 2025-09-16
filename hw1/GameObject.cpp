#include "stdafx.h"
#include "GameObject.h"



XMFLOAT3 GameObject::GetPosition()
{
	return(XMFLOAT3(m_xmf4x4World._41, m_xmf4x4World._42, m_xmf4x4World._43));
}

XMFLOAT3 GameObject::GetLook()
{
	return(Vector3::Normalize(XMFLOAT3(m_xmf4x4World._31, m_xmf4x4World._32, m_xmf4x4World._33)));
}

XMFLOAT3 GameObject::GetUp()
{
	return(Vector3::Normalize(XMFLOAT3(m_xmf4x4World._21, m_xmf4x4World._22, m_xmf4x4World._23)));
}

XMFLOAT3 GameObject::GetRight()
{
	return(Vector3::Normalize(XMFLOAT3(m_xmf4x4World._11, m_xmf4x4World._12, m_xmf4x4World._13)));
}

void GameObject::SetPosition(float x, float y, float z)
{
	m_xmf4x4Transform._41 = x;
	m_xmf4x4Transform._42 = y;
	m_xmf4x4Transform._43 = z;

	UpdateTransform(NULL);
}

void GameObject::SetPosition(XMFLOAT3 xmf3Position)
{
	SetPosition(xmf3Position.x, xmf3Position.y, xmf3Position.z);
}

void GameObject::SetScale(float x, float y, float z)
{
	XMMATRIX mtxScale = XMMatrixScaling(x, y, z);
	m_xmf4x4Transform = Matrix4x4::Multiply(mtxScale, m_xmf4x4Transform);

	UpdateTransform(NULL);
}

void GameObject::MoveStrafe(float fDistance)
{
	XMFLOAT3 xmf3Position = GetPosition();
	XMFLOAT3 xmf3Right = GetRight();
	xmf3Position = Vector3::Add(xmf3Position, xmf3Right, fDistance);
	GameObject::SetPosition(xmf3Position);
}

void GameObject::MoveUp(float fDistance)
{
	XMFLOAT3 xmf3Position = GetPosition();
	XMFLOAT3 xmf3Up = GetUp();
	xmf3Position = Vector3::Add(xmf3Position, xmf3Up, fDistance);
	GameObject::SetPosition(xmf3Position);
}

void GameObject::MoveForward(float fDistance)
{
	XMFLOAT3 xmf3Position = GetPosition();
	XMFLOAT3 xmf3Look = GetLook();
	xmf3Position = Vector3::Add(xmf3Position, xmf3Look, fDistance);
	GameObject::SetPosition(xmf3Position);
}

void GameObject::Rotate(float fPitch, float fYaw, float fRoll)
{
	XMMATRIX mtxRotate = XMMatrixRotationRollPitchYaw(XMConvertToRadians(fPitch), XMConvertToRadians(fYaw), XMConvertToRadians(fRoll));
	m_xmf4x4Transform = Matrix4x4::Multiply(mtxRotate, m_xmf4x4Transform);

	UpdateTransform(NULL);
}

void GameObject::Rotate(XMFLOAT3* pxmf3Axis, float fAngle)
{
	XMMATRIX mtxRotate = XMMatrixRotationAxis(XMLoadFloat3(pxmf3Axis), XMConvertToRadians(fAngle));
	m_xmf4x4Transform = Matrix4x4::Multiply(mtxRotate, m_xmf4x4Transform);

	UpdateTransform(NULL);
}

void GameObject::Rotate(XMFLOAT4* pxmf4Quaternion)
{
	XMMATRIX mtxRotate = XMMatrixRotationQuaternion(XMLoadFloat4(pxmf4Quaternion));
	m_xmf4x4Transform = Matrix4x4::Multiply(mtxRotate, m_xmf4x4Transform);

	UpdateTransform(NULL);
}

void GameObject::UpdateTransform(XMFLOAT4X4* pxmf4x4Parent)
{
	m_xmf4x4World = (pxmf4x4Parent) ? Matrix4x4::Multiply(m_xmf4x4Transform, *pxmf4x4Parent) : m_xmf4x4Transform;
	m_xmOBB.Transform(m_xmOBBWorld, XMLoadFloat4x4(&m_xmf4x4World));

	for (auto& pChild : m_pChildren) {
		pChild->UpdateTransform(&m_xmf4x4World);
	}
}

std::shared_ptr<GameObject> GameObject::FindFrame(const std::string& svFrameName)
{
	std::shared_ptr<GameObject> pFrameObject;
	if (svFrameName == m_strFrameName) {
		return shared_from_this();
	}

	for (auto& pChild : m_pChildren) {
		if (pFrameObject = pChild->FindFrame(svFrameName)) {
			return pFrameObject;
		}
	}

	return nullptr;
}

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
	std::string strRead;
	
	int nMaterials;
	int materialIndex;
	inFile.read((char*)(&nMaterials), sizeof(int));
	
	std::vector<MATERIALLOADINFO> materials;
	materials.resize(nMaterials);

	while (true) {
		strRead = ::ReadStringFromFile(inFile);

		if (strRead == "<Material>:")
		{
			inFile.read((char*)(&materialIndex), sizeof(int));
		}
		else if (strRead == "<AlbedoColor>:")
		{
			inFile.read((char*)(&materials[materialIndex].xmf4AlbedoColor), sizeof(XMFLOAT4));
		}
		else if (strRead == "<EmissiveColor>:")
		{
			inFile.read((char*)(&materials[materialIndex].xmf4EmissiveColor), sizeof(XMFLOAT4));
		}
		else if (strRead == "<SpecularColor>:")
		{
			inFile.read((char*)(&materials[materialIndex].xmf4SpecularColor), sizeof(XMFLOAT4));
		}
		else if (strRead == "<Glossiness>:")
		{
			inFile.read((char*)(&materials[materialIndex].fGlossiness), sizeof(float));
		}
		else if (strRead == "<Smoothness>:")
		{
			inFile.read((char*)(&materials[materialIndex].fSmoothness), sizeof(float));
		}
		else if (strRead == "<Metallic>:")
		{
			inFile.read((char*)(&materials[materialIndex].fMetallic), sizeof(float));
		}
		else if (strRead == "<SpecularHighlight>:")
		{
			inFile.read((char*)(&materials[materialIndex].fSpecularHighlight), sizeof(float));
		}
		else if (strRead == "<GlossyReflection>:")
		{
			inFile.read((char*)(&materials[materialIndex].fGlossyReflection), sizeof(float));
		}
		else if (strRead == "</Materials>")
		{
			break;
		}
	}

	return materials;
}

std::shared_ptr<MESHLOADINFO> GameObject::LoadMeshInfoFromFile(std::ifstream& inFile)
{
	std::string strRead;

	std::shared_ptr<MESHLOADINFO> pMeshInfo = std::make_shared<MESHLOADINFO>();

	int nVertices;
	inFile.read((char*)(&nVertices), sizeof(int));
	pMeshInfo->strMeshName = ::ReadStringFromFile(inFile);

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
				inFile.read((char*)pMeshInfo->xmf3Positions.data(), sizeof(XMFLOAT3) * nPositions);
			}
		}
		else if (strRead == "<Colors>:") {
			int nColors;
			inFile.read((char*)&nColors, sizeof(int));
			if (nColors > 0) {
				pMeshInfo->nType |= VERTEX_TYPE_COLOR;
				pMeshInfo->xmf4Colors.resize(nColors);
				inFile.read((char*)pMeshInfo->xmf4Colors.data(), sizeof(XMFLOAT4) * nColors);
			}
		}
		else if (strRead == "<Normals>:") {
			int nNormals;
			inFile.read((char*)&nNormals, sizeof(int));
			if (nNormals > 0) {
				pMeshInfo->nType |= VERTEX_TYPE_NORMAL;
				pMeshInfo->xmf3Normals.resize(nNormals);
				inFile.read((char*)pMeshInfo->xmf3Normals.data(), sizeof(XMFLOAT3) * nNormals);
			}
		}
		else if (strRead == "<Indices>:") {
			int Indices;
			inFile.read((char*)&Indices, sizeof(int));
			if (Indices > 0)
			{
				pMeshInfo->Indices.resize(Indices);
				inFile.read((char*)pMeshInfo->Indices.data(), sizeof(UINT) * Indices);
			}
		}
		else if (strRead == "<SubMeshes>:") {
			int SubMeshes;
			inFile.read((char*)&SubMeshes, sizeof(int));
			if (SubMeshes > 0) {
				pMeshInfo->SubMeshes.resize(SubMeshes);
				for (int i = 0; i < pMeshInfo->SubMeshes.size(); ++i) {
					strRead = ::ReadStringFromFile(inFile);
					if (strRead == "<SubMesh>:") {
						int nIndex;
						int nSubSetIndices;
						inFile.read((char*)&nIndex, sizeof(int));
						inFile.read((char*)&nSubSetIndices, sizeof(int));
						if (nSubSetIndices > 0) {
							pMeshInfo->SubMeshes[i].resize(nSubSetIndices);
							inFile.read((char*)pMeshInfo->SubMeshes[i].data(), sizeof(UINT) * nSubSetIndices);
						}
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

std::shared_ptr<GameObject> GameObject::LoadFrameHierarchyFromFile(ComPtr<ID3D12Device> pd3dDevice, ComPtr<ID3D12GraphicsCommandList> pd3dCommandList, ComPtr<ID3D12RootSignature> pd3dRootSignature, std::shared_ptr<GameObject> pParent, std::ifstream& inFile)
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

			if (pParent) {
				pGameObject->m_pParent = pParent;
			}
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
			inFile.read((char*)&pGameObject->m_xmf4x4Transform, sizeof(XMFLOAT4X4));
		}
		else if (strRead == "<Mesh>:") {
			std::shared_ptr<MESHLOADINFO> pMeshLoadInfo = GameObject::LoadMeshInfoFromFile(inFile);
			if (pMeshLoadInfo) {
				std::shared_ptr<Mesh> pMesh;
				if (pMeshLoadInfo->nType & VERTEX_TYPE_NORMAL) {
					pMesh = std::make_shared<IlluminatedMesh>(pd3dDevice, pd3dCommandList, *pMeshLoadInfo);
					RESOURCE->AddMesh(pMeshLoadInfo->strMeshName, pMesh);
				}
				if (pMesh) {
					pGameObject->m_pMesh = RESOURCE->GetMesh(pMeshLoadInfo->strMeshName);
					BoundingOrientedBox::CreateFromPoints(pGameObject->m_xmOBB, pMeshLoadInfo->xmf3Positions.size(), pMeshLoadInfo->xmf3Positions.data(), sizeof(XMFLOAT3));
				}
			}
		}
		else if (strRead == "<Materials>:") {
			std::vector<MATERIALLOADINFO> materialInfos = GameObject::LoadMaterialsInfoFromFile(pd3dDevice, pd3dCommandList, inFile);
			
			if (materialInfos.size() != 0) {
				pGameObject->m_pMaterials.reserve(materialInfos.size());

				for (int i = 0; i < materialInfos.size(); ++i) {
					std::shared_ptr<Material> pMaterial = std::make_shared<Material>(pd3dDevice, pd3dCommandList);

					std::shared_ptr<MaterialColors> pMaterialColors = std::make_shared<MaterialColors>(materialInfos[i]);
					pMaterial->SetMaterialColors(pMaterialColors);
					
					if (pGameObject->m_pMesh->GetType() & VERTEX_TYPE_NORMAL) {
						pMaterial->SetIlluminatedShader();
					}

					std::string strMaterialKey = std::format("MAT_{}_{}", pGameObject->m_strFrameName, i);
					RESOURCE->AddMaterial(strMaterialKey, pMaterial);

					pGameObject->m_pMaterials.push_back(RESOURCE->GetMaterial(strMaterialKey));
				}
			}
			
		}
		else if (strRead == "<Children>:") {
			int nChildren;
			inFile.read((char*)&nChildren, sizeof(int));
			pGameObject->m_pChildren.reserve(nChildren);
			if (nChildren > 0) {
				for (int i = 0; i < nChildren; ++i) {
					std::shared_ptr<GameObject> pChild = GameObject::LoadFrameHierarchyFromFile(pd3dDevice, pd3dCommandList, pd3dRootSignature, pGameObject, inFile);
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

std::shared_ptr<GameObject> GameObject::LoadGeometryFromFile(ComPtr<ID3D12Device> pd3dDevice, ComPtr<ID3D12GraphicsCommandList> pd3dCommandList, ComPtr<ID3D12RootSignature> pd3dRootSignature, const std::string& strFileName)
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
			pGameObject = GameObject::LoadFrameHierarchyFromFile(pd3dDevice, pd3dCommandList, pd3dRootSignature, nullptr, inFile);
		}
		else if (strRead == "</Hierarchy>") {
			break;
		}
	}

	return pGameObject;
}
