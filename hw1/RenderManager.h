#pragma once

class Mesh;
class Material;
class StructuredBuffer;

#define MAX_INSTANCING_COUNT						10000
#define ASSUMED_MESH_PER_INSTANCE					30
#define ASSUMED_MATERIAL_PER_MESH					2
#define ASSUMED_REQUIRED_STRUCTURED_BUFFER_SIZE		MAX_INSTANCING_COUNT * ASSUMED_MESH_PER_INSTANCE * ASSUMED_MATERIAL_PER_MESH

#define ASSUMED_OBJECT_TYPE_PER_SCENE				2
#define ASSUMED_REQUIRED_DESCRIPTOR_COUNT			Scene::g_uiDescriptorCountPerScene + 1 + ASSUMED_MESH_PER_INSTANCE * ASSUMED_MATERIAL_PER_MESH * ASSUMED_OBJECT_TYPE_PER_SCENE

struct INSTANCE_DATA {
	XMFLOAT4X4 xmf4x4GameObject;
};

struct INSTANCE_KEY {
	std::shared_ptr<Mesh>					pMesh;
	std::vector<std::shared_ptr<Material>>	pMaterials;
	UINT									uiDescriptorCountPerInstance;

	bool operator==(const INSTANCE_KEY& other) const noexcept {
		return pMesh == other.pMesh && pMaterials == other.pMaterials;
	}
};

template<>
struct std::hash<INSTANCE_KEY> {
	size_t operator()(const INSTANCE_KEY& key) const {
		size_t h1 = std::hash<std::shared_ptr<Mesh>>{}(key.pMesh);
		size_t h2 = std::hash<std::shared_ptr<Material>>{}(key.pMaterials[0]);
		return h1 ^ h2;
	}
};


// Shader 에 넘길 SHADER_VISIBLE 한 m_pd3dDescriptorHeap은 아래처럼 구성
// SRV 는 인스턴싱용 StructuredBuffer
// CBV 는 SubSet 별 Material 정보 (Mesh 마다 갯수 가 다름) -> SubSet 순서대로 기록되도록
//  
//  
//                    +---------> m_pd3dDescriptorHeap->GetCPUDescriptorHandleForHeapStart()
//                    |  
//                    +-----+-----+------+-----++-----+-----++-----++-----+-----++-----++-----++-----++-----+----- 
//  Descriptor 구성   | CBV | CBV | SRV  | CBV || CBV | ... || CBV || CBV | ... || CBV || CBV || CBV || CBV | ...
//                    +-----+-----+------+-----++-----+-----++-----++-----+-----++-----++-----++-----++-----+-----
//    Resource 단위   |   Scene   | Inst |      Mesh 1       |      Mesh 2       |          Mesh 3          | ...
//                    +-----------+------+-------------------+-------------------+--------------------------+-----
//                          |
//                          +-----------> 여기는 Scene::Render() 에서 복사함
//


class RenderManager {
public:
	RenderManager(ComPtr<ID3D12Device> pd3dDevice, ComPtr<ID3D12GraphicsCommandList> pd3dCommandList);

	void Add(std::shared_ptr<GameObject> pGameObject);
	void Render(ComPtr<ID3D12GraphicsCommandList> pd3dCommandList);
	void Clear();

	size_t GetMeshCount() const;
	UINT GetDrawCallCount() const { return m_nDrawCalls; };
	ComPtr<ID3D12DescriptorHeap> GetDescriptorHeap() { return m_pd3dDescriptorHeap; }
	void SetDescriptorHeapToPipeline(ComPtr<ID3D12GraphicsCommandList> pd3dCommandList) const;

private:
	std::unordered_map<INSTANCE_KEY, UINT> m_InstanceIndexMap;
	std::vector<std::pair<INSTANCE_KEY, std::vector<INSTANCE_DATA>>> m_InstanceDatas;
	UINT m_nInstanceIndex = 0;
	

	ComPtr<ID3D12Device>			m_pd3dDevice = nullptr;	// GameFramewok::m_pd3dDevice 의 참조
	ComPtr<ID3D12DescriptorHeap>	m_pd3dDescriptorHeap = nullptr;
	StructuredBuffer				m_InstanceDataSBuffer;

	UINT m_nDrawCalls = 0;
};
