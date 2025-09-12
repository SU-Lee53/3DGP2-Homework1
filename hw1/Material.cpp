#include "stdafx.h"
#include "Material.h"

////////////////////
// MaterialColors //
////////////////////

MaterialColors::MaterialColors(const MATERIALLOADINFO& pMaterialInfo)
{
	xmf4Diffuse = pMaterialInfo.xmf4AlbedoColor;
	xmf4Specular = pMaterialInfo.xmf4SpecularColor; //(r,g,b,a=power)
	xmf4Specular.w = (pMaterialInfo.fGlossiness * 255.0f);
	xmf4Emissive = pMaterialInfo.xmf4EmissiveColor;
}

//////////////
// Material //
//////////////

std::shared_ptr<Shader> Material::m_pIlluminatedShader = NULL;

Material::Material(ComPtr<ID3D12Device> pd3dDevice, ComPtr<ID3D12GraphicsCommandList> pd3dCommandList)
{
	m_pMaterialCBuffer = std::make_shared<ConstantBuffer>(pd3dDevice, pd3dCommandList, ConstantBufferSize<CB_MATERIAL_DATA>::value);
}

void Material::UpdateShaderVariable(ID3D12GraphicsCommandList* pd3dCommandList)
{
}
