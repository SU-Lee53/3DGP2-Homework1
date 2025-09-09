#pragma once

struct DiffusedVertex {
	XMFLOAT3 xmf3Position;
	XMFLOAT4 xmf4Color;

	static D3D12_INPUT_LAYOUT_DESC GetInputLayout() {
		return { inputLayout.data(), (unsigned int)inputLayout.size() };
	}

	static std::vector<D3D12_INPUT_ELEMENT_DESC> inputLayout;
};

struct IlluminatedVertex {
	XMFLOAT3 xmf3Position;
	XMFLOAT3 xmf3Normal;

	static D3D12_INPUT_LAYOUT_DESC GetInputLayout() {
		return { inputLayout.data(), (unsigned int)inputLayout.size() };
	}

	static std::vector<D3D12_INPUT_ELEMENT_DESC> inputLayout;
};

// ������ Ÿ�� ���� Ÿ��üũ concept
template<typename C>
concept VertexType = requires (C c) {
	std::same_as<D3D12_INPUT_LAYOUT_DESC, decltype(std::declval(C().GetInputLayout()))>;
};
