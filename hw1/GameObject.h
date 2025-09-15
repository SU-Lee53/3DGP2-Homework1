#pragma once
#include "Mesh.h"
#include "Material.h"

struct CB_OBJECT_DATA {
	XMFLOAT4X4 xmf4GameObject;
	CB_MATERIAL_DATA materialData;
};

class GameObject : public std::enable_shared_from_this<GameObject> {

public:
	XMFLOAT3 GetPosition();
	XMFLOAT3 GetLook();
	XMFLOAT3 GetUp();
	XMFLOAT3 GetRight();

	void SetPosition(float x, float y, float z);
	void SetPosition(XMFLOAT3 xmf3Position);
	void SetScale(float x, float y, float z);

	void MoveStrafe(float fDistance = 1.0f);
	void MoveUp(float fDistance = 1.0f);
	void MoveForward(float fDistance = 1.0f);

	void Rotate(float fPitch = 10.0f, float fYaw = 10.0f, float fRoll = 10.0f);
	void Rotate(XMFLOAT3* pxmf3Axis, float fAngle);
	void Rotate(XMFLOAT4* pxmf4Quaternion);

	std::shared_ptr<GameObject> GetParent() { return m_pParent; }
	void UpdateTransform(XMFLOAT4X4* pxmf4x4Parent = NULL);
	std::shared_ptr<GameObject> FindFrame(const std::string& svFrameName);

	UINT GetMeshType() { return((m_pMesh) ? m_pMesh->GetType() : 0); }

public:
	XMFLOAT4X4 GetWorldMatrix() const { return m_xmf4x4World; }
	std::shared_ptr<Mesh> GetMesh() const { return m_pMesh; }
	std::vector<std::shared_ptr<Material>>& GetMaterials() { return m_pMaterials; }

private:
	std::string m_strFrameName;

	std::shared_ptr<Mesh> m_pMesh;
	
	std::vector<std::shared_ptr<Material>> m_pMaterials;

	XMFLOAT4X4 m_xmf4x4Transform;
	XMFLOAT4X4 m_xmf4x4World;

	std::shared_ptr<GameObject> m_pParent;
	std::vector<std::shared_ptr<GameObject>> m_pChildren;


public:
	static std::vector<MATERIALLOADINFO> LoadMaterialsInfoFromFile(ComPtr<ID3D12Device> pd3dDevice, ComPtr<ID3D12GraphicsCommandList> pd3dCommandList, std::ifstream& inFile);
	static std::shared_ptr<MESHLOADINFO> LoadMeshInfoFromFile(std::ifstream& inFile);

	static std::shared_ptr<GameObject> LoadFrameHierarchyFromFile(ComPtr<ID3D12Device> pd3dDevice, ComPtr<ID3D12GraphicsCommandList> pd3dCommandList, ComPtr<ID3D12RootSignature> pd3dRootSignature, std::shared_ptr<GameObject> pParent, std::ifstream& inFile);
	static std::shared_ptr<GameObject> LoadGeometryFromFile(ComPtr<ID3D12Device> pd3dDevice, ComPtr<ID3D12GraphicsCommandList> pd3dCommandList, ComPtr<ID3D12RootSignature> pd3dRootSignature, const std::string& strFileName);

};

