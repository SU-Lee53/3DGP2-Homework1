#include "stdafx.h"
#include "Material.h"

std::shared_ptr<Shader> Material::m_pIlluminatedShader = NULL;

MaterialColors::MaterialColors(const MATERIALLOADINFO& pMaterialInfo)
{
	xmf4Diffuse = pMaterialInfo.xmf4AlbedoColor;
	xmf4Specular = pMaterialInfo.xmf4SpecularColor; //(r,g,b,a=power)
	xmf4Specular.w = (pMaterialInfo.fGlossiness * 255.0f);
	xmf4Emissive = pMaterialInfo.xmf4EmissiveColor;
}

void Material::UpdateShaderVariable(ID3D12GraphicsCommandList* pd3dCommandList)
{
}
