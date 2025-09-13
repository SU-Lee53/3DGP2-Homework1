#include "stdafx.h"
#include "Shader.h"

void IlluminatedShader::Create(ComPtr<ID3D12Device> pd3dDevice)
{
}

void IlluminatedShader::CreateRootSignature()
{
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


	D3D12_INPUT_ELEMENT_DESC inputElementDesc[] = {
		{"POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		{"NORMAL", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
	};

	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc;
	inputLayoutDesc.NumElements = _countof(inputElementDesc);
	inputLayoutDesc.pInputElementDescs = inputElementDesc;

	return D3D12_INPUT_LAYOUT_DESC();
}

D3D12_RASTERIZER_DESC IlluminatedShader::CreateRasterizerState()
{
	return D3D12_RASTERIZER_DESC();
}

D3D12_BLEND_DESC IlluminatedShader::CreateBlendState()
{
	return D3D12_BLEND_DESC();
}

D3D12_DEPTH_STENCIL_DESC IlluminatedShader::CreateDepthStencilState()
{
	return D3D12_DEPTH_STENCIL_DESC();
}

D3D12_SHADER_BYTECODE IlluminatedShader::CreateVertexShader()
{
	return D3D12_SHADER_BYTECODE();
}

D3D12_SHADER_BYTECODE IlluminatedShader::CreatePixelShader()
{
	return D3D12_SHADER_BYTECODE();
}
