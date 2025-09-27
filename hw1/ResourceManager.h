#pragma once
//#include "Mesh.h"
//#include "Material.h"
#include "GameObject.h"

// Mesh, Material, Shader 등의 리소스를 단 1개의 포인터만 들고있을 수 있도록 관리하는 Manager
// 외부에서 오브젝트를 불러올때 반드시 한번 거쳐서 저장하고 Set 할때도 여기서 가져가도록 함
// 현재 Shader 는 Material 에서 static 으로 관리하므로 굳이 여기서 관리할 필요 없어보임

// 09.27
// 위 내용 전부 취소
// Apache 읽어오던중 다른 Mesh가 같은 이름을 달고있어 모델 계층들이 엄한 Mesh를 들고다니는 사고 발생
// GameObject 를 통으로 보관하도록 수정

class ResourceManager {
public:
	// Add 함수는 Key 값이 겹치면 nullptr 를 리턴하도록 함
	std::shared_ptr<GameObject> AddGameObject(const std::string& svObjKey, std::shared_ptr<GameObject> pObject);
	std::shared_ptr<GameObject> GetGameObject(const std::string& svObjKey);
	std::shared_ptr<GameObject> CopyGameObject(const std::string& svObjKey);


private:
	// std::unordered_map<std::string, std::shared_ptr<Mesh>>		m_MeshMap;
	// std::unordered_map<std::string, std::shared_ptr<Material>>	m_MaterialMap;

	std::unordered_map<std::string, std::shared_ptr<GameObject>> m_pGameObject;

};

