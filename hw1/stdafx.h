// header.h: 표준 시스템 포함 파일
// 또는 프로젝트 특정 포함 파일이 들어 있는 포함 파일입니다.
//

#pragma once

#include "targetver.h"
#define WIN32_LEAN_AND_MEAN             // 거의 사용되지 않는 내용을 Windows 헤더에서 제외합니다.
// Windows 헤더 파일
#include <windows.h>
// C 런타임 헤더 파일입니다.
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

// STL
#include <iostream>
#include <algorithm>
#include <vector>
#include <array>
#include <fstream>
#include <memory>
#include <concepts>

#include <string>
#ifdef UNICODE
using TSTRING = std::wstring;
#else
using TSTRING = std::string;
#endif

// Direct3D related headers
#include <wrl.h>
#include <shellapi.h>	

// D3D12
#include <d3d12.h>
#include <dxgi1_4.h>
#include <dxgidebug.h>

// D3DCompiler
#include <d3dcompiler.h>

// DirectXMath
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <DirectXColors.h>
#include <DirectXCollision.h>

using namespace Microsoft::WRL;
using namespace DirectX;

// Import libraries
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")


#include "GameFramework.h"



ComPtr<ID3D12Resource> CreateBufferResource(ComPtr<ID3D12Device> pd3dDevice, ComPtr<ID3D12GraphicsCommandList> pd3dCommandList, void* pData, UINT nBytes, D3D12_HEAP_TYPE d3dHeapType, D3D12_RESOURCE_STATES d3dResourceStates, ComPtr<ID3D12Resource>& pd3dUploadBuffer);

template<typename T>
struct ConstantBufferSize {
	constexpr static size_t value = (sizeof(T) + 255) & (~255);
};
