#include "Vertex.h"

using namespace Vertex;

const D3D11_INPUT_ELEMENT_DESC InputLayoutDesc::VertexPT[2] =
{
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
};

const D3D11_INPUT_ELEMENT_DESC InputLayoutDesc::VertexPNT[3] =
{
	{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
	{"NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
	{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0}
};

const D3D11_INPUT_ELEMENT_DESC InputLayoutDesc::VertexPNTTan[4] =
{
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 },

};

ID3D11InputLayout* InputLayouts::VertexPT = 0;
ID3D11InputLayout* InputLayouts::VertexPNT = 0;
ID3D11InputLayout * InputLayouts::VertexPNTTan = 0;

void InputLayouts::InitLayout(ID3D11Device* device, ID3DBlob * blob, VERTEXFORMAT format)
{
	switch (format)
	{
	case POS:
		break;
	case POSCOL:
		break;
	case POSTEX:
		HR(device->CreateInputLayout(InputLayoutDesc::VertexPT, 2, blob->GetBufferPointer(),
			blob->GetBufferSize(), &VertexPT));
		break;
	case POSNORTEX:
		HR(device->CreateInputLayout(InputLayoutDesc::VertexPNT, 3, blob->GetBufferPointer(),
			blob->GetBufferSize(), &VertexPNT));
		break;
	case  POSNORTEXTAN :
		HR(device->CreateInputLayout(InputLayoutDesc::VertexPNTTan, 4, blob->GetBufferPointer(),
			blob->GetBufferSize(), &VertexPNTTan));
	default:
		break;
	}
}

void InputLayouts::DestroyAll()
{
	ReleaseCOM(VertexPT);
	ReleaseCOM(VertexPNT);
	ReleaseCOM(VertexPNTTan);
}

