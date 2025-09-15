#pragma once
#include "Shader.h"

struct MATERIALLOADINFO {
	XMFLOAT4 xmf4AlbedoColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	XMFLOAT4 xmf4EmissiveColor = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	XMFLOAT4 xmf4SpecularColor = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
			 
	float	 fGlossiness = 0.0f;
	float	 fSmoothness = 0.0f;
	float	 fSpecularHighlight = 0.0f;
	float	 fMetallic = 0.0f;
	float	 fGlossyReflection = 0.0f;
			 
	UINT	 eType = 0x00;
};

struct MaterialColors {
	MaterialColors() {}
	MaterialColors(const MATERIALLOADINFO& pMaterialInfo);

	XMFLOAT4 xmf4Ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	XMFLOAT4 xmf4Diffuse = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	XMFLOAT4 xmf4Specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f); //(r,g,b,a=power)
	XMFLOAT4 xmf4Emissive = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
};

class Shader;

struct CB_MATERIAL_DATA {
	XMFLOAT4 xmf4Ambient;
	XMFLOAT4 xmf4Diffuse;
	XMFLOAT4 xmf4Specular;
	XMFLOAT4 xmf4Emissive;
};

class Material {
public:
	Material(ComPtr<ID3D12Device> pd3dDevice, ComPtr<ID3D12GraphicsCommandList> pd3dCommandList);

public:
	void SetMaterialColors(std::shared_ptr<MaterialColors> pMaterialColors) { m_pMaterialColors = pMaterialColors; }
	void SetShader(std::shared_ptr<Shader> pShader) { m_pShader = pShader; }
	void SetIlluminatedShader() { SetShader(m_pIlluminatedShader); }

	void UpdateShaderVariable(ComPtr<ID3D12GraphicsCommandList> pd3dCommandList);
	void SetMaterialToPipeline(ComPtr<ID3D12GraphicsCommandList> pd3dCommandList, UINT uiRootParameterIndex);

	void OnPrepareRender(ComPtr<ID3D12GraphicsCommandList> pd3dCommandList) {
		m_pShader->OnPrepareRender(pd3dCommandList);
	}

private:
	std::shared_ptr<Shader> m_pShader;
	std::shared_ptr<MaterialColors> m_pMaterialColors;
	std::shared_ptr<ConstantBuffer> m_pMaterialCBuffer;

private:
	static std::shared_ptr<Shader> m_pIlluminatedShader;

public:
	static void PrepareShaders(ComPtr<ID3D12Device> pd3dDevice, ComPtr<ID3D12RootSignature> pd3dRootSignature);

};

