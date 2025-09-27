#include "stdafx.h"
#include "ResourceManager.h"

std::shared_ptr<GameObject> ResourceManager::AddGameObject(const std::string& svObjKey, std::shared_ptr<GameObject> pObject)
{
	if (m_pGameObject.contains(svObjKey)) return nullptr;

	m_pGameObject.insert({ svObjKey, pObject });
	return pObject;
}

std::shared_ptr<GameObject> ResourceManager::GetGameObject(const std::string& svObjKey)
{
	auto it = m_pGameObject.find(svObjKey);
	if (it == m_pGameObject.end()) {
		return nullptr;
	}

	return it->second;
}

std::shared_ptr<GameObject> ResourceManager::CopyGameObject(const std::string& svObjKey)
{
	auto it = m_pGameObject.find(svObjKey);
	if (it == m_pGameObject.end()) {
		return nullptr;
	}

	return GameObject::CopyObject(*it->second);
}
