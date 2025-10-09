#include "stdafx.h"
#include "GameObject.h"
#include <filesystem>

GameObject::GameObject()
{
	m_xmf4x4Transform = Matrix4x4::Identity();
	m_xmf4x4World = Matrix4x4::Identity();
}

void GameObject::SetMesh(std::shared_ptr<Mesh> pMesh)
{
	m_pMesh = pMesh;
}

void GameObject::SetShader(std::shared_ptr<Shader> pShader)
{
	m_pMaterials.resize(1);
	m_pMaterials[0] = std::make_shared<Material>();
	m_pMaterials[0]->SetShader(pShader);
}

void GameObject::SetShader(int nMaterial, std::shared_ptr<Shader> pShader)
{
	if (m_pMaterials[nMaterial]) {
		m_pMaterials[nMaterial]->SetShader(pShader);
	}
}

void GameObject::SetMaterial(int nMaterial, std::shared_ptr<Material> pMaterial)
{
	m_pMaterials[nMaterial] = pMaterial;
}

void GameObject::SetChild(std::shared_ptr<GameObject> pChild)
{
	if (pChild)
	{
		pChild->m_pParent = shared_from_this();
		m_pChildren.push_back(pChild);
	}
}

void GameObject::Update(float fTimeElapsed)
{
	Animate(fTimeElapsed);
}

void GameObject::Animate(float fTimeElapsed) 
{
	for (auto pChild : m_pChildren) {
		pChild->Animate(fTimeElapsed);
	}
}

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

	for (auto& pChild : m_pChildren) {
		pChild->UpdateTransform(&m_xmf4x4World);
	}
}

std::shared_ptr<GameObject> GameObject::FindFrame(const std::string& strFrameName)
{
	std::shared_ptr<GameObject> pFrameObject;
	if (strFrameName == m_strFrameName) {
		return shared_from_this();
	}

	for (auto& pChild : m_pChildren) {
		if (pFrameObject = pChild->FindFrame(strFrameName)) {
			return pFrameObject;
		}
	}

	return nullptr;
}

void GameObject::AddToRenderMap()
{
	OnPrepareRender();

	if (m_pMesh) {
		RENDER->Add(shared_from_this());
	}

	for (auto& pChild : m_pChildren) {
		pChild->AddToRenderMap();
	}
}

void GameObject::ReleaseUploadBuffers()
{
	if (m_pMesh) m_pMesh->ReleaseUploadBuffers();

	for (auto& pChild : m_pChildren) {
		pChild->ReleaseUploadBuffers();
	}
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

				for (int i = 0; i < materialInfos.size(); ++i) {
					std::shared_ptr<Material> pMaterial = std::make_shared<Material>(pd3dDevice, pd3dCommandList);

					std::shared_ptr<MaterialColors> pMaterialColors = std::make_shared<MaterialColors>(materialInfos[i]);
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
			if (pGameObject) {
				pGameObject->m_pChildren.reserve(nChildren);
			}

			if (nChildren > 0) {
				for (int i = 0; i < nChildren; ++i) {
					std::shared_ptr<GameObject> pChild = GameObject::LoadFrameHierarchyFromFile(pd3dDevice, pd3dCommandList, pd3dRootSignature, pGameObject, inFile);
					if (pChild) {
						pGameObject->m_pChildren.push_back(pChild);
					}
				}
			}
		}
		else if ((strRead == "</Frame>")){
			break;
		}
	}

	return pGameObject;
}

std::shared_ptr<GameObject> GameObject::LoadGeometryFromFile(ComPtr<ID3D12Device> pd3dDevice, ComPtr<ID3D12GraphicsCommandList> pd3dCommandList, ComPtr<ID3D12RootSignature> pd3dRootSignature, const std::string& strFilePath)
{
	std::string strFileName = std::filesystem::path{ strFilePath }.stem().string();

	if (auto pObj = RESOURCE->GetGameObject(strFileName)) {
		return pObj;
	}

	std::ifstream inFile{ strFilePath, std::ios::binary };
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

	if (pGameObject) {
		return RESOURCE->AddGameObject(strFileName, pGameObject);
	}

	return nullptr;
}

std::shared_ptr<GameObject> GameObject::CopyObject(const GameObject& srcObject, std::shared_ptr<GameObject> pParent)
{
	std::shared_ptr<GameObject> pClone = std::make_shared<GameObject>();
	pClone->m_strFrameName = srcObject.m_strFrameName;
	pClone->m_pMesh = srcObject.m_pMesh;
	pClone->m_pMaterials = srcObject.m_pMaterials;
	pClone->m_xmf4x4Transform = srcObject.m_xmf4x4Transform;
	pClone->m_xmf4x4World = srcObject.m_xmf4x4World;

	pClone->m_pParent = pParent;

	pClone->m_pChildren.reserve(srcObject.m_pChildren.size());
	for (auto& pChild : srcObject.m_pChildren) {
		std::shared_ptr<GameObject> pChildClone = CopyObject(*pChild, pClone);
		pClone->m_pChildren.push_back(pChildClone);
	}

	return pClone;
}

///////////////////////
// HellicopterObject //
///////////////////////

HellicopterObject::HellicopterObject()
{
}

HellicopterObject::~HellicopterObject()
{
}

void HellicopterObject::Initialize()
{
}

void HellicopterObject::Animate(float fTimeElapsed)
{
	if (m_pMainRotorFrame)
	{
		XMMATRIX xmmtxRotate = XMMatrixRotationY(XMConvertToRadians(360.0f * 2.0f) * fTimeElapsed);
		m_pMainRotorFrame->m_xmf4x4Transform = Matrix4x4::Multiply(xmmtxRotate, m_pMainRotorFrame->m_xmf4x4Transform);
	}
	if (m_pTailRotorFrame)
	{
		XMMATRIX xmmtxRotate = XMMatrixRotationX(XMConvertToRadians(360.0f * 4.0f) * fTimeElapsed);
		m_pTailRotorFrame->m_xmf4x4Transform = Matrix4x4::Multiply(xmmtxRotate, m_pTailRotorFrame->m_xmf4x4Transform);
	}

	GameObject::Animate(fTimeElapsed);
}

//////////////////
// ApacheObject //
//////////////////

ApacheObject::ApacheObject()
{
}

ApacheObject::~ApacheObject()
{
}

void ApacheObject::Initialize()
{
	m_pMainRotorFrame = FindFrame("rotor");
	m_pTailRotorFrame = FindFrame("black_m_7");
}

void ApacheObject::Animate(float fTimeElapsed)
{
	if (m_pMainRotorFrame)
	{
		XMMATRIX xmmtxRotate = XMMatrixRotationY(XMConvertToRadians(360.0f * 2.0f) * fTimeElapsed);
		m_pMainRotorFrame->m_xmf4x4Transform = Matrix4x4::Multiply(xmmtxRotate, m_pMainRotorFrame->m_xmf4x4Transform);
	}
	if (m_pTailRotorFrame)
	{
		XMMATRIX xmmtxRotate = XMMatrixRotationY(XMConvertToRadians(360.0f * 4.0f) * fTimeElapsed);
		m_pTailRotorFrame->m_xmf4x4Transform = Matrix4x4::Multiply(xmmtxRotate, m_pTailRotorFrame->m_xmf4x4Transform);
	}

	GameObject::Animate(fTimeElapsed);
}

///////////////////
// GunshipObject //
///////////////////

GunshipObject::GunshipObject()
{
}

GunshipObject::~GunshipObject()
{
}

void GunshipObject::Initialize()
{
	m_pMainRotorFrame = FindFrame("Rotor");
	m_pTailRotorFrame = FindFrame("Back_Rotor");
}

//////////////////////
// SuperCobraObject //
//////////////////////

SuperCobraObject::SuperCobraObject()
{
}

SuperCobraObject::~SuperCobraObject()
{
}

void SuperCobraObject::Initialize()
{
	m_pMainRotorFrame = FindFrame("MainRotor_LOD0");
	m_pTailRotorFrame = FindFrame("TailRotor_LOD0");
}

////////////////
// Mi24Object //
////////////////

Mi24Object::Mi24Object()
{
}

Mi24Object::~Mi24Object()
{
}

void Mi24Object::Initialize()
{
	m_pMainRotorFrame = FindFrame("Top_Rotor");
	m_pTailRotorFrame = FindFrame("Tail_Rotor");
}

//////////////////
// HummerObject //
//////////////////

HummerObject::HummerObject()
{
}

HummerObject::~HummerObject()
{
}

void HummerObject::Initialize()
{
	m_pLFWheelFrame = FindFrame("wheel_LF");
	m_pLRWheelFrame = FindFrame("wheel_LR");
	m_pRFWheelFrame = FindFrame("Wheel_RF");
	m_pRRWheelFrame = FindFrame("wheel_RR");
}

void HummerObject::Animate(float fTimeElapsed)
{
	if (m_pLFWheelFrame)
	{
		XMMATRIX xmmtxRotate = XMMatrixRotationX(XMConvertToRadians(360.0f * 2.0f) * fTimeElapsed);
		m_pLFWheelFrame->m_xmf4x4Transform = Matrix4x4::Multiply(xmmtxRotate, m_pLFWheelFrame->m_xmf4x4Transform);
	}
	
	if (m_pLRWheelFrame)
	{
		XMMATRIX xmmtxRotate = XMMatrixRotationX(XMConvertToRadians(360.0f * 2.0f) * fTimeElapsed);
		m_pLRWheelFrame->m_xmf4x4Transform = Matrix4x4::Multiply(xmmtxRotate, m_pLRWheelFrame->m_xmf4x4Transform);
	}
	
	if (m_pRFWheelFrame)
	{
		XMMATRIX xmmtxRotate = XMMatrixRotationX(XMConvertToRadians(360.0f * 2.0f) * fTimeElapsed);
		m_pRFWheelFrame->m_xmf4x4Transform = Matrix4x4::Multiply(xmmtxRotate, m_pRFWheelFrame->m_xmf4x4Transform);
	}
	
	if (m_pRRWheelFrame)
	{
		XMMATRIX xmmtxRotate = XMMatrixRotationX(XMConvertToRadians(360.0f * 2.0f) * fTimeElapsed);
		m_pRRWheelFrame->m_xmf4x4Transform = Matrix4x4::Multiply(xmmtxRotate, m_pRRWheelFrame->m_xmf4x4Transform);
	}

	GameObject::Animate(fTimeElapsed);
}

////////////////
// TankObject //
////////////////

TankObject::TankObject()
{
}

TankObject::~TankObject()
{
}

void TankObject::Initialize()
{
}

void TankObject::Animate(float fTimeElapsed)
{
	if (m_pTurretFrame)
	{
		XMMATRIX xmmtxRotate = XMMatrixRotationY(XMConvertToRadians(30.0f) * fTimeElapsed);
		m_pTurretFrame->m_xmf4x4Transform = Matrix4x4::Multiply(xmmtxRotate, m_pTurretFrame->m_xmf4x4Transform);
	}

	GameObject::Animate(fTimeElapsed);
}

///////////////
// M26Object //
///////////////

M26Object::M26Object()
{
}

M26Object::~M26Object()
{
}

void M26Object::Initialize()
{
	m_pTurretFrame = FindFrame("TURRET");
	m_pCannonFrame = FindFrame("cannon");
	m_pGunFrame = FindFrame("gun");
}
