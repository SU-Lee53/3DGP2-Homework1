#pragma once
//#include "Mesh.h"
//#include "Material.h"
#include "GameObject.h"

// Mesh, Material, Shader ���� ���ҽ��� �� 1���� �����͸� ������� �� �ֵ��� �����ϴ� Manager
// �ܺο��� ������Ʈ�� �ҷ��ö� �ݵ�� �ѹ� ���ļ� �����ϰ� Set �Ҷ��� ���⼭ ���������� ��
// ���� Shader �� Material ���� static ���� �����ϹǷ� ���� ���⼭ ������ �ʿ� �����

// 09.27
// �� ���� ���� ���
// Apache �о������ �ٸ� Mesh�� ���� �̸��� �ް��־� �� �������� ���� Mesh�� ���ٴϴ� ��� �߻�
// GameObject �� ������ �����ϵ��� ����

class ResourceManager {
public:
	std::shared_ptr<GameObject> AddGameObject(const std::string& strObjKey, std::shared_ptr<GameObject> pObject);
	std::shared_ptr<GameObject> GetGameObject(const std::string& strObjKey);
	std::shared_ptr<GameObject> CopyGameObject(const std::string& strObjKey);


private:
	// std::unordered_map<std::string, std::shared_ptr<Mesh>>		m_MeshMap;
	// std::unordered_map<std::string, std::shared_ptr<Material>>	m_MaterialMap;

	std::unordered_map<std::string, std::shared_ptr<GameObject>> m_pGameObjects;

};

