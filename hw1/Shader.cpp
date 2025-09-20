#include "stdafx.h"
#include "Shader.h"
#include <fstream>

////////////
// Shader //
////////////

D3D12_RASTERIZER_DESC Shader::CreateRasterizerState()
{
	D3D12_RASTERIZER_DESC desc{};
	{
		desc.FillMode = D3D12_FILL_MODE_SOLID;
		desc.CullMode = D3D12_CULL_MODE_BACK;
		desc.FrontCounterClockwise = FALSE;
		desc.DepthBias = 0;
		desc.DepthBiasClamp = 0.0f;
		desc.SlopeScaledDepthBias = 0.0f;
		desc.DepthClipEnable = TRUE;
		desc.MultisampleEnable = FALSE;
		desc.AntialiasedLineEnable = FALSE;
		desc.ForcedSampleCount = 0;
		desc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
	}

	return desc;
}

D3D12_BLEND_DESC Shader::CreateBlendState()
{
	D3D12_BLEND_DESC desc{};
	{
		desc.AlphaToCoverageEnable = FALSE;
		desc.IndependentBlendEnable = FALSE;
		desc.RenderTarget[0].BlendEnable = FALSE;
		desc.RenderTarget[0].LogicOpEnable = FALSE;
		desc.RenderTarget[0].SrcBlend = D3D12_BLEND_ONE;
		desc.RenderTarget[0].DestBlend = D3D12_BLEND_ZERO;
		desc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
		desc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
		desc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
		desc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
		desc.RenderTarget[0].LogicOp = D3D12_LOGIC_OP_NOOP;
		desc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	}

	return desc;
}

D3D12_DEPTH_STENCIL_DESC Shader::CreateDepthStencilState()
{
	D3D12_DEPTH_STENCIL_DESC desc{};
	{
		desc.DepthEnable = TRUE;
		desc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
		desc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
		desc.StencilEnable = FALSE;
		desc.StencilReadMask = 0x00;
		desc.StencilWriteMask = 0x00;
		desc.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
		desc.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
		desc.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
		desc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_NEVER;
		desc.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
		desc.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
		desc.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
		desc.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_NEVER;
	}

	return desc;
}

D3D12_SHADER_BYTECODE Shader::CreateVertexShader()
{
	return D3D12_SHADER_BYTECODE();
}

D3D12_SHADER_BYTECODE Shader::CreatePixelShader()
{
	return D3D12_SHADER_BYTECODE();
}

void Shader::OnPrepareRender(ComPtr<ID3D12GraphicsCommandList> pd3dCommandList, int nPipelineState)
{
	pd3dCommandList->SetPipelineState(m_pd3dPipelineState.Get());
}

D3D12_SHADER_BYTECODE Shader::CompileShaderFromFile(const std::wstring& wstrFileName, const std::string& strShaderName, const std::string& strShaderProfile, ID3DBlob** ppd3dShaderBlob)
{
	UINT nCompileFlags = 0;
#ifdef _DEBUG
	nCompileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	ComPtr<ID3DBlob> pd3dErrorBlob = nullptr;
	HRESULT hResult = ::D3DCompileFromFile(wstrFileName.data(), NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE, strShaderName.data(), strShaderProfile.data(), nCompileFlags, 0, ppd3dShaderBlob, pd3dErrorBlob.GetAddressOf());
	char* pErrorString = NULL;
	if (pd3dErrorBlob) {
		pErrorString = (char*)pd3dErrorBlob->GetBufferPointer();
		HWND hWnd = ::GetActiveWindow();
		MessageBoxA(hWnd, pErrorString, NULL, 0);
		OutputDebugStringA(pErrorString);
		__debugbreak();
	}
	
	D3D12_SHADER_BYTECODE d3dShaderByteCode{};
	d3dShaderByteCode.BytecodeLength = (*ppd3dShaderBlob)->GetBufferSize();
	d3dShaderByteCode.pShaderBytecode = (*ppd3dShaderBlob)->GetBufferPointer();

	return(d3dShaderByteCode);
}

D3D12_SHADER_BYTECODE Shader::ReadCompiledShaderFromFile(const std::wstring& wstrFileName, ID3DBlob** ppd3dShaderBlob)
{
	std::ifstream in { wstrFileName.data(), std::ios::binary};
	
	if (!in) {
		__debugbreak();
	}

	in.seekg(0, std::ios::end);
	int nFileSize = in.tellg();
	in.seekg(0, std::ios::beg);

	std::unique_ptr<BYTE[]> pByteCode = std::make_unique<BYTE[]>(nFileSize);
	in.read((char*)pByteCode.get(), nFileSize);

	D3D12_SHADER_BYTECODE d3dShaderByteCode{};

	HRESULT hr = D3DCreateBlob(nFileSize, ppd3dShaderBlob);
	if (FAILED(hr)) {
		__debugbreak();
	}

	::memcpy((*ppd3dShaderBlob)->GetBufferPointer(), pByteCode.get(), nFileSize);

	d3dShaderByteCode.BytecodeLength = (*ppd3dShaderBlob)->GetBufferSize();
	d3dShaderByteCode.pShaderBytecode = (*ppd3dShaderBlob)->GetBufferPointer();

	return d3dShaderByteCode;
}

///////////////////////
// IlluminatedShader //
///////////////////////

void IlluminatedShader::Create(ComPtr<ID3D12Device> pd3dDevice, ComPtr<ID3D12RootSignature> pd3dRootSignature)
{
	D3D12_GRAPHICS_PIPELINE_STATE_DESC d3dPipelineDesc{};
	{
		d3dPipelineDesc.pRootSignature = pd3dRootSignature.Get();
		d3dPipelineDesc.VS = CreateVertexShader();
		d3dPipelineDesc.PS = CreatePixelShader();
		d3dPipelineDesc.RasterizerState = CreateRasterizerState();
		d3dPipelineDesc.BlendState = CreateBlendState();
		d3dPipelineDesc.DepthStencilState = CreateDepthStencilState();
		d3dPipelineDesc.InputLayout = CreateInputLayout();
		d3dPipelineDesc.SampleMask = UINT_MAX;
		d3dPipelineDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		d3dPipelineDesc.NumRenderTargets = 1;
		d3dPipelineDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		d3dPipelineDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
		d3dPipelineDesc.SampleDesc.Count = 1;
		d3dPipelineDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
	}

	HRESULT hr = pd3dDevice->CreateGraphicsPipelineState(&d3dPipelineDesc, IID_PPV_ARGS(m_pd3dPipelineState.GetAddressOf()));
	if (FAILED(hr)) {
		__debugbreak();
	}
}

D3D12_INPUT_LAYOUT_DESC IlluminatedShader::CreateInputLayout()
{

	/*
	typedef struct D3D12_INPUT_ELEMENT_DESC
	{
		LPCSTR SemanticName;
		UINT SemanticIndex;
		DXGI_FORMAT Format;
		UINT InputSlot;
		UINT AlignedByteOffset;
		D3D12_INPUT_CLASSIFICATION InputSlotClass;
		UINT InstanceDataStepRate;
	} 	D3D12_INPUT_ELEMENT_DESC;
	*/

	m_d3dInputElements = {
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
	};

	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc;
	inputLayoutDesc.NumElements = m_d3dInputElements.size();
	inputLayoutDesc.pInputElementDescs = m_d3dInputElements.data();

	return inputLayoutDesc;
}

D3D12_SHADER_BYTECODE IlluminatedShader::CreateVertexShader()
{
	return CompileShaderFromFile(L"Shaders.hlsl", "VSMain", "vs_5_1", m_pd3dVertexShaderBlob.GetAddressOf());
}

D3D12_SHADER_BYTECODE IlluminatedShader::CreatePixelShader()
{
	return CompileShaderFromFile(L"Shaders.hlsl", "PSMain", "ps_5_1", m_pd3dPixelShaderBlob.GetAddressOf());
}
