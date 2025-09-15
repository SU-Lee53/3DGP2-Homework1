#pragma once
#include "Mesh.h"
#include "Material.h"

// Mesh, Material, Shader 등의 리소스를 단 1개의 포인터만 들고있을 수 있도록 관리하는 Manager
// 외부에서 오브젝트를 불러올때 반드시 한번 거쳐서 저장하고 Set 할때도 여기서 가져가도록 함
// 현재 Shader 는 Material 에서 static 으로 관리하므로 굳이 여기서 관리할 필요 없어보임

class ResourceManager {
public:
	// Add 함수는 Key 값이 겹치면 false 를 리턴하도록 함
	bool AddMesh(const std::string& svMeshKey, std::shared_ptr<Mesh> pMesh);
	bool AddMaterial(const std::string& strMaterialKey, std::shared_ptr<Material> pMesh);
	
	std::shared_ptr<Mesh> GetMesh(const std::string& svMeshKey);
	std::shared_ptr<Material> GetMaterial(const std::string& strMaterialKey);


private:
	std::unordered_map<std::string, std::shared_ptr<Mesh>>		m_MeshMap;
	std::unordered_map<std::string, std::shared_ptr<Material>>	m_MaterialMap;

};

