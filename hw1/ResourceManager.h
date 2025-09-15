#pragma once
#include "Mesh.h"
#include "Material.h"

// Mesh, Material, Shader ���� ���ҽ��� �� 1���� �����͸� ������� �� �ֵ��� �����ϴ� Manager
// �ܺο��� ������Ʈ�� �ҷ��ö� �ݵ�� �ѹ� ���ļ� �����ϰ� Set �Ҷ��� ���⼭ ���������� ��
// ���� Shader �� Material ���� static ���� �����ϹǷ� ���� ���⼭ ������ �ʿ� �����

class ResourceManager {
public:
	// Add �Լ��� Key ���� ��ġ�� false �� �����ϵ��� ��
	bool AddMesh(const std::string& svMeshKey, std::shared_ptr<Mesh> pMesh);
	bool AddMaterial(const std::string& strMaterialKey, std::shared_ptr<Material> pMesh);
	
	std::shared_ptr<Mesh> GetMesh(const std::string& svMeshKey);
	std::shared_ptr<Material> GetMaterial(const std::string& strMaterialKey);


private:
	std::unordered_map<std::string, std::shared_ptr<Mesh>>		m_MeshMap;
	std::unordered_map<std::string, std::shared_ptr<Material>>	m_MaterialMap;

};

