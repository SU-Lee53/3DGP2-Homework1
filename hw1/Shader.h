#pragma once

class Shader {
public:
	virtual void Create(ComPtr<ID3D12Device> pd3dDevice) {}

protected:
	virtual void CreateRootSignature() {}
	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout() {}
	virtual D3D12_RASTERIZER_DESC CreateRasterizerState() {}
	virtual D3D12_BLEND_DESC CreateBlendState() {}
	virtual D3D12_DEPTH_STENCIL_DESC CreateDepthStencilState() {}

	virtual D3D12_SHADER_BYTECODE CreateVertexShader() {}
	virtual D3D12_SHADER_BYTECODE CreatePixelShader() {}

	D3D12_SHADER_BYTECODE CompileShaderFromFile(std::wstring_view wstrFileName, std::string_view pszShaderName, std::string_view pszShaderProfile) {}
	D3D12_SHADER_BYTECODE ReadCompiledShaderFromFile(std::wstring_view pszFileName) {}


protected:
	ComPtr<ID3DBlob> m_pd3dVertexShaderBlob = nullptr;
	ComPtr<ID3DBlob> m_pd3dPixelShaderBlob	= nullptr;

	ComPtr<ID3D12RootSignature> m_pd3dRootSignature = nullptr;
	ComPtr<ID3D12PipelineState>	m_pd3dPipelineStates = nullptr;

	D3D12_GRAPHICS_PIPELINE_STATE_DESC	m_d3dPipelineStateDesc;
};

class IlluminatedShader {
public:
	virtual void Create(ComPtr<ID3D12Device> pd3dDevice) override;

protected:
	virtual void CreateRootSignature();
	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout();
	virtual D3D12_RASTERIZER_DESC CreateRasterizerState();
	virtual D3D12_BLEND_DESC CreateBlendState();
	virtual D3D12_DEPTH_STENCIL_DESC CreateDepthStencilState();

	virtual D3D12_SHADER_BYTECODE CreateVertexShader();
	virtual D3D12_SHADER_BYTECODE CreatePixelShader();

	D3D12_SHADER_BYTECODE CompileShaderFromFile(std::wstring_view wstrFileName, std::string_view pszShaderName, std::string_view pszShaderProfile);
	D3D12_SHADER_BYTECODE ReadCompiledShaderFromFile(std::wstring_view pszFileName);



};

