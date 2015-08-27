#include "SkyBox.h"
#include "GeoGenerator.h"
#include <vector>
#include "D3DUtil.h"
#include "Camera.h"
#include "RenderStates.h"

SkyBox::SkyBox(ID3D11Device *device, float radius, UINT sliceCnt, UINT stackCnt, std::string cubeMapFileName)
:m_pVB(0),
m_pIB(0),
m_pSkySRV(0),
m_pVS(0),
m_pPS(0),
m_pCBPerFrame(0),
mRadius(radius),
idxCnt(0)
{
	GeoGenerator::Mesh skyMesh;
	GeoGenerator::GenSphere(mRadius, sliceCnt, stackCnt, skyMesh);

	idxCnt = skyMesh.indices.size();
	std::vector<Vertex::VertexBase> vertices(skyMesh.vertices.size());
	
	for (UINT i = 0; i < skyMesh.vertices.size(); i++)
		vertices[i].Pos = skyMesh.vertices[i].Pos;

	D3D11_BUFFER_DESC vDesc;
	ZeroMemory(&vDesc, sizeof(vDesc));
	vDesc.ByteWidth = sizeof(Vertex::VertexPNT) * vertices.size();
	vDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vDesc.Usage = D3D11_USAGE_IMMUTABLE;
	vDesc.CPUAccessFlags = 0; 
	vDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA vData;
	ZeroMemory(&vData, sizeof(vData));
	vData.pSysMem = &vertices[0];
	HR(device->CreateBuffer(&vDesc, &vData, &m_pVB));

	D3D11_BUFFER_DESC iDesc;
	ZeroMemory(&vDesc, sizeof(iDesc));
	iDesc.ByteWidth = sizeof(DWORD) * skyMesh.indices.size();
	iDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	iDesc.Usage = D3D11_USAGE_IMMUTABLE;
	iDesc.CPUAccessFlags = 0;
	iDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA iData;
	ZeroMemory(&iData, sizeof(iData));
	iData.pSysMem = &skyMesh.indices[0];
	HR(device->CreateBuffer(&iDesc, &iData, &m_pIB));

	ID3DBlob *pBlob = NULL;
	HR(LoadShaderBinaryFromFile("Shaders//SkyBoxVS.fxo", &pBlob));
	HR(device->CreateVertexShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), NULL, &m_pVS));
	InputLayouts::InitLayout(device, pBlob, Vertex::POS);

	pBlob = NULL;
	HR(LoadShaderBinaryFromFile("Shaders//SkyBoxPS.fxo", &pBlob));
	HR(device->CreatePixelShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), NULL, &m_pPS));
	ReleaseCOM(pBlob);


	D3D11_BUFFER_DESC cbDesc;
	ZeroMemory(&cbDesc, sizeof(cbDesc));
	cbDesc.Usage = D3D11_USAGE_DEFAULT;
	cbDesc.CPUAccessFlags = 0;
	cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

	cbDesc.ByteWidth = sizeof(CBPerFrame);
	HR(device->CreateBuffer(&cbDesc, 0, &m_pCBPerFrame));

	D3D11_SAMPLER_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	desc.MinLOD = 0;
	desc.MaxLOD = D3D11_FLOAT32_MAX;
	HR(device->CreateSamplerState(&desc, &m_pSampleTriLinear));

	HR(D3DX11CreateShaderResourceViewFromFile(device, L"..//Resources//Sunsetcube.dds", 0, 0, &m_pSkySRV, 0));
}


SkyBox::~SkyBox()
{
	ReleaseCOM(m_pVB);
	ReleaseCOM(m_pIB);
	ReleaseCOM(m_pSkySRV);
	ReleaseCOM(m_pVS);
	ReleaseCOM(m_pPS);
	ReleaseCOM(m_pCBPerFrame);
	ReleaseCOM(m_pSampleTriLinear);
}

void SkyBox::Draw(ID3D11DeviceContext *context, Camera *camera)
{
	XMFLOAT3 eyePos = camera->GetPos();
	XMMATRIX trans = XMMatrixTranslation(eyePos.x, eyePos.y, eyePos.z);
	XMMATRIX WVP = XMMatrixTranspose(XMMatrixMultiply( trans, camera->GetViewProjMatrix()));
	mCBPerFrame.worldViewProj = WVP;
	context->UpdateSubresource(m_pCBPerFrame, 0, NULL, &mCBPerFrame, 0, 0);

	UINT stride = sizeof(Vertex::VertexBase);
	UINT offset = 0;

	context->IASetVertexBuffers(0, 1, &m_pVB, &stride, &offset);
	context->IASetIndexBuffer(m_pIB, DXGI_FORMAT_R32_UINT, 0);
	context->IASetInputLayout(InputLayouts::VertexP);
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	context->RSSetState(RenderStates::NoCullRS);

	context->VSSetShader(m_pVS, NULL, 0);
	context->VSSetConstantBuffers(0, 1, &m_pCBPerFrame);
	context->PSSetShader(m_pPS, NULL, 0);
	context->PSSetShaderResources(0, 1, &m_pSkySRV);
	context->PSSetSamplers(0, 1, &m_pSampleTriLinear);

	context->DrawIndexed(idxCnt, 0, 0);
}