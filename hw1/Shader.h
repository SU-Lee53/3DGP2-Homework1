#pragma once

class Shader {
public:
	virtual void Create(ComPtr<ID3D12Device> pd3dDevice, ComPtr<ID3D12RootSignature> pd3dRootSignature) {}

	virtual void OnPrepareRender(ComPtr<ID3D12GraphicsCommandList> pd3dCommandList, int nPipelineState = 0);

protected:
	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout() { return D3D12_INPUT_LAYOUT_DESC{}; }
	virtual D3D12_RASTERIZER_DESC CreateRasterizerState();
	virtual D3D12_BLEND_DESC CreateBlendState();
	virtual D3D12_DEPTH_STENCIL_DESC CreateDepthStencilState();

	virtual D3D12_SHADER_BYTECODE CreateVertexShader();
	virtual D3D12_SHADER_BYTECODE CreatePixelShader();

	D3D12_SHADER_BYTECODE CompileShaderFromFile(const std::wstring& wstrFileName, const std::string& pszShaderName, const std::string& pszShaderProfile, ID3DBlob** ppd3dShaderBlob);
	D3D12_SHADER_BYTECODE ReadCompiledShaderFromFile(const std::wstring& wsvFileName, ID3DBlob** ppd3dShaderBlob);


protected:
	ComPtr<ID3DBlob> m_pd3dVertexShaderBlob = nullptr;
	ComPtr<ID3DBlob> m_pd3dPixelShaderBlob	= nullptr;

	ComPtr<ID3D12PipelineState>	m_pd3dPipelineState = nullptr;

	std::vector<D3D12_INPUT_ELEMENT_DESC> m_d3dInputElements;

	D3D12_GRAPHICS_PIPELINE_STATE_DESC	m_d3dPipelineStateDesc;

};

class IlluminatedShader : public Shader {
public:
	virtual void Create(ComPtr<ID3D12Device> pd3dDevice, ComPtr<ID3D12RootSignature> pd3dRootSignature) override;

protected:
	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout() override;

	virtual D3D12_SHADER_BYTECODE CreateVertexShader() override;
	virtual D3D12_SHADER_BYTECODE CreatePixelShader() override;

};
