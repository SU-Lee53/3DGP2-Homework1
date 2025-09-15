#include "stdafx.h"
#include "ResourceManager.h"

bool ResourceManager::AddMesh(const std::string& strMeshKey, std::shared_ptr<Mesh> pMesh)
{
	if (m_MeshMap.contains(strMeshKey)) return false;

	m_MeshMap.insert({ strMeshKey, pMesh });
	return true;
}

bool ResourceManager::AddMaterial(const std::string& strMaterialKey, std::shared_ptr<Material> pMesh)
{
	if (m_MaterialMap.contains(strMaterialKey)) return false;

	m_MaterialMap.insert({ strMaterialKey, pMesh });
	return true;
}

std::shared_ptr<Mesh> ResourceManager::GetMesh(const std::string& strMeshKey)
{
	auto it = m_MeshMap.find(strMeshKey);
	if (it == m_MeshMap.end()) {
		return nullptr;
	}

	return it->second;
}

std::shared_ptr<Material> ResourceManager::GetMaterial(const std::string& strMaterialKey)
{
	auto it = m_MaterialMap.find(strMaterialKey);
	if (it == m_MaterialMap.end()) {
		return nullptr;
	}

	return it->second;
}
