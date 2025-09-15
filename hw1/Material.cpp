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

std::shared_ptr<Shader> Material::m_pIlluminatedShader = nullptr;

Material::Material(ComPtr<ID3D12Device> pd3dDevice, ComPtr<ID3D12GraphicsCommandList> pd3dCommandList)
{
	m_pMaterialCBuffer = std::make_shared<ConstantBuffer>(pd3dDevice, pd3dCommandList, ConstantBufferSize<CB_MATERIAL_DATA>::value);
}

void Material::UpdateShaderVariable(ComPtr<ID3D12GraphicsCommandList> pd3dCommandList)
{
	CB_MATERIAL_DATA cbData{};
	{
		cbData.xmf4Ambient = m_pMaterialColors->xmf4Ambient;
		cbData.xmf4Diffuse = m_pMaterialColors->xmf4Diffuse;
		cbData.xmf4Specular = m_pMaterialColors->xmf4Specular;
		cbData.xmf4Emissive = m_pMaterialColors->xmf4Emissive;
	}
	m_pMaterialCBuffer->UpdateData(pd3dCommandList, &cbData);
}

void Material::SetMaterialToPipeline(ComPtr<ID3D12GraphicsCommandList> pd3dCommandList, UINT uiRootParameterIndex)
{
	m_pMaterialCBuffer->SetBufferToPipeline(pd3dCommandList, uiRootParameterIndex);
}

void Material::PrepareShaders(ComPtr<ID3D12Device> pd3dDevice, ComPtr<ID3D12RootSignature> pd3dRootSignature)
{
	m_pIlluminatedShader = std::make_shared<IlluminatedShader>();
	m_pIlluminatedShader->Create(pd3dDevice, pd3dRootSignature);
}
