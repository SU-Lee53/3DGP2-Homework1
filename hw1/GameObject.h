#pragma once
#include "Mesh.h"
#include "Material.h"

enum MOVE_DIR : UINT {
	MOVE_DIR_FORWARD		= 1,
	MOVE_DIR_BACKWARD	= MOVE_DIR_FORWARD << 1,
	MOVE_DIR_LEFT		= MOVE_DIR_BACKWARD << 1,
	MOVE_DIR_RIGHT		= MOVE_DIR_LEFT << 1,
	MOVE_DIR_UP			= MOVE_DIR_RIGHT << 1,
	MOVE_DIR_DOWN		= MOVE_DIR_UP << 1,
};

struct CB_OBJECT_DATA {
	XMFLOAT4X4 xmf4GameObject;
	CB_MATERIAL_DATA materialData;
};

class GameObject : public std::enable_shared_from_this<GameObject> {
public:
	GameObject();
public:
	void SetMesh(std::shared_ptr<Mesh> pMesh);
	void SetShader(std::shared_ptr<Shader> pShader);
	void SetShader(int nMaterial, std::shared_ptr<Shader> pShader);
	void SetMaterial(int nMaterial, std::shared_ptr<Material> pMaterial);

	void SetChild(std::shared_ptr<GameObject> pChild);

public:
	virtual void Initialize() {}
	virtual void Update(float fTimeElapsed);
	virtual void Animate(float fTimeElapsed);

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

	const BoundingOrientedBox& GetOBBWorld() const { return m_xmOBBWorld; }
	
public:
	virtual void OnPrepareRender() {}
	virtual void AddToRenderMap();

public:
	void ReleaseUploadBuffers();

public:
	std::string m_strFrameName;

	std::shared_ptr<Mesh> m_pMesh;
	
	std::vector<std::shared_ptr<Material>> m_pMaterials;

	XMFLOAT4X4 m_xmf4x4Transform;
	XMFLOAT4X4 m_xmf4x4World;

	std::shared_ptr<GameObject> m_pParent;
	std::vector<std::shared_ptr<GameObject>> m_pChildren;

	BoundingOrientedBox m_xmOBB;
	BoundingOrientedBox m_xmOBBWorld;


public:
	static std::vector<MATERIALLOADINFO> LoadMaterialsInfoFromFile(ComPtr<ID3D12Device> pd3dDevice, ComPtr<ID3D12GraphicsCommandList> pd3dCommandList, std::ifstream& inFile);
	static std::shared_ptr<MESHLOADINFO> LoadMeshInfoFromFile(std::ifstream& inFile);

	static std::shared_ptr<GameObject> LoadFrameHierarchyFromFile(ComPtr<ID3D12Device> pd3dDevice, ComPtr<ID3D12GraphicsCommandList> pd3dCommandList, ComPtr<ID3D12RootSignature> pd3dRootSignature, std::shared_ptr<GameObject> pParent, std::ifstream& inFile);
	static std::shared_ptr<GameObject> LoadGeometryFromFile(ComPtr<ID3D12Device> pd3dDevice, ComPtr<ID3D12GraphicsCommandList> pd3dCommandList, ComPtr<ID3D12RootSignature> pd3dRootSignature, const std::string& strFilePath);

	static std::shared_ptr<GameObject> CopyObject(const GameObject& srcObject, std::shared_ptr<GameObject> pParent = nullptr);

};

class RotatingObject : public GameObject {
public:
	RotatingObject();
	virtual ~RotatingObject();

private:
	XMFLOAT3					m_xmf3RotationAxis;
	float						m_fRotationSpeed;

public:
	void SetRotationSpeed(float fRotationSpeed) { m_fRotationSpeed = fRotationSpeed; }
	void SetRotationAxis(XMFLOAT3 xmf3RotationAxis) { m_xmf3RotationAxis = xmf3RotationAxis; }

	virtual void Animate(float fTimeElapsed) override;
};

class RevolvingObject : public GameObject {
public:
	RevolvingObject();
	virtual ~RevolvingObject();

private:
	XMFLOAT3					m_xmf3RevolutionAxis;
	float						m_fRevolutionSpeed;

public:
	void SetRevolutionSpeed(float fRevolutionSpeed) { m_fRevolutionSpeed = fRevolutionSpeed; }
	void SetRevolutionAxis(XMFLOAT3 xmf3RevolutionAxis) { m_xmf3RevolutionAxis = xmf3RevolutionAxis; }

	virtual void Animate(float fTimeElapsed) override;
};

class HellicopterObject : public GameObject {
public:
	HellicopterObject();
	virtual ~HellicopterObject();

protected:
	std::shared_ptr<GameObject> m_pMainRotorFrame = NULL;
	std::shared_ptr<GameObject> m_pTailRotorFrame = NULL;

public:
	virtual void Initialize() override;
	virtual void Animate(float fTimeElapsed) override;
};

class ApacheObject : public HellicopterObject {
public:
	ApacheObject();
	virtual ~ApacheObject();

public:
	virtual void Initialize() override;
	virtual void Animate(float fTimeElapsed) override;
};

class GunshipObject : public HellicopterObject {
public:
	GunshipObject();
	virtual ~GunshipObject();

public:
	virtual void Initialize() override;
};

class SuperCobraObject : public HellicopterObject {
public:
	SuperCobraObject();
	virtual ~SuperCobraObject();

public:
	virtual void Initialize() override;
};

class Mi24Object : public HellicopterObject {
public:
	Mi24Object();
	virtual ~Mi24Object();

public:
	virtual void Initialize() override;
};

////////////////////////////////////////////////////////////////////////////////////////////////
//
class HummerObject : public GameObject {
public:
	HummerObject();
	virtual ~HummerObject();
};

////////////////////////////////////////////////////////////////////////////////////////////////
//
class TankObject : public GameObject {
public:
	TankObject();
	virtual ~TankObject();

protected:
	std::shared_ptr<GameObject> m_pTurretFrame = NULL;
	std::shared_ptr<GameObject> m_pCannonFrame = NULL;
	std::shared_ptr<GameObject> m_pGunFrame = NULL;

public:
	virtual void Initialize() override;
	virtual void Animate(float fTimeElapsed) override;
};

class M26Object : public TankObject {
public:
	M26Object();
	virtual ~M26Object();

	virtual void Initialize() override;
};
