#include "DemoApp.h"
#include "Vertex.h"
#include "RenderStates.h"
#include "ShadowMap.h"
#include "AABB.h"
#include "GeoGenerator.h"

struct CBNeverChanges
{
	DirectionalLight dirLight;
	float shadowMapSize;
	XMFLOAT3 pad3;
};

struct CBOnResize
{
	XMMATRIX mProjection;
};

struct CBPerFrame
{
	XMFLOAT3 eyePos;
	float pad;
};

struct CBPerObject
{
	XMMATRIX matWorld;
	XMMATRIX matWVP;
	Material material;
	XMMATRIX matWorldInvTranspose;
	XMMATRIX matLightWVPT;
	int isInstancing;
	XMFLOAT3 padding;
};

struct CBPerObjectShadow
{
	XMMATRIX lightWVP;
	int isInstancing;
	XMFLOAT3 padding;
};

struct CBPerFrameScreenQuad
{
	XMMATRIX wvp;
};

DemoApp::DemoApp(HINSTANCE hInstance)
:DemoBase(hInstance),
m_pVertexShader(0),
m_pPixelShader(0),
m_pShadowMapVS(0),
m_pShadowMapPS(0),
m_pDebugTextureVS(0),
m_pDebugTexturePS(0),
m_pCBNeverChanges(0),
m_pCBOnResize(0),
m_pCBPerFrame(0),
m_pCBPerObject(0),
m_pCBPerObjShadow(0),
m_pCBPerFrameScreenQuad(0),
m_pVertexBuffer(0),
m_pIndexBuffer(0),
m_pInstancedBuffer(0),
m_pGroundVertexBuffer(0),
m_pGroundIndexBuffer(0),
m_pScreenQuadVB(0),
m_pScreenQuadIB(0),
m_pDepthSRV(0),
m_pSampleLinear(0),
m_pSampleShadowMap(0),
mTheta(-0.8f*MathHelper::Pi), 
mPhi(0.4f*MathHelper::Pi), 
mRadius(60.0f),
m_pShadowMap(0),
mShadowMapSize(2048),
m_pAABB(0)
{
	this->mMainWndCaption = L"Demo";
	mLastMousePos.x = 0;
	mLastMousePos.y = 0;
}

DemoApp::~DemoApp()
{
	if (m_pShadowMap)
	{
		delete m_pShadowMap;
		m_pShadowMap = 0;
	}

	if (m_pAABB)
	{
		delete m_pAABB;
		m_pAABB = 0;
	}

	md3dImmediateContext->ClearState();
	ReleaseCOM(m_pVertexBuffer);
	ReleaseCOM(m_pIndexBuffer);
	ReleaseCOM(m_pScreenQuadVB);
	ReleaseCOM(m_pScreenQuadIB);
	ReleaseCOM(m_pGroundVertexBuffer);
	ReleaseCOM(m_pGroundIndexBuffer);
	ReleaseCOM(m_pInstancedBuffer);
	ReleaseCOM(m_pCBNeverChanges);
	ReleaseCOM(m_pCBOnResize);
	ReleaseCOM(m_pCBPerFrame);
	ReleaseCOM(m_pCBPerObject);
	ReleaseCOM(m_pCBPerObjShadow);
	ReleaseCOM(m_pCBPerFrameScreenQuad);
	ReleaseCOM(m_pSampleLinear);
	ReleaseCOM(m_pSampleShadowMap);
	ReleaseCOM(m_pVertexShader);
	ReleaseCOM(m_pPixelShader);
	ReleaseCOM(m_pShadowMapVS);
	ReleaseCOM(m_pShadowMapPS);
	ReleaseCOM(m_pDebugTextureVS);
	ReleaseCOM(m_pDebugTexturePS);
	InputLayouts::DestroyAll();
	RenderStates::DestroyAll();
}
 

// ======= Dont need to change =======
void DemoApp::OnResize()
{
	DemoBase::OnResize();
	m_Proj = XMMatrixPerspectiveFovLH(XM_PIDIV4, mClientWidth / (float)mClientHeight, 0.01f, 100.0f);
	md3dImmediateContext->VSSetConstantBuffers(1, 1, &m_pCBOnResize);
	md3dImmediateContext->PSSetConstantBuffers(1, 1, &m_pCBOnResize);
}

void DemoApp::OnMouseDown(WPARAM btnState, int x, int y)
{
	mLastMousePos.x = x;
	mLastMousePos.y = y;

	SetCapture(mhMainWnd);
}

void DemoApp::OnMouseUp(WPARAM btnState, int x, int y)
{
	ReleaseCapture();
}

void DemoApp::OnMouseMove(WPARAM btnState, int x, int y)
{
	if ((btnState & MK_LBUTTON) != 0)
	{
		// Make each pixel correspond to a quarter of a degree.
		float dx = XMConvertToRadians(0.25f*static_cast<float>(x - mLastMousePos.x));
		float dy = XMConvertToRadians(0.25f*static_cast<float>(y - mLastMousePos.y));

		// Update angles based on input to orbit camera around box.
		mTheta -= dx;
		mPhi -= dy;

		// Restrict the angle mPhi.
		mPhi = MathHelper::Clamp(mPhi, 0.1f, MathHelper::Pi - 0.1f);
	}
	else if ((btnState & MK_RBUTTON) != 0)
	{
		// Make each pixel correspond to 0.01 unit in the scene.
		float dx = 0.1f*static_cast<float>(x - mLastMousePos.x);
		float dy = 0.1f*static_cast<float>(y - mLastMousePos.y);

		// Update the camera radius based on input.
		mRadius += dx - dy;

		// Restrict the radius.
		mRadius = MathHelper::Clamp(mRadius, 35.0f, 75.0f);
	}

	mLastMousePos.x = x;
	mLastMousePos.y = y;
}

//============================

void DemoApp::CreateLights()
{
	mDirLight.Ambient = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
	mDirLight.Diffuse = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
	mDirLight.Specular = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
	mDirLight.Direction = XMFLOAT3(1.0f, -1.0f, 0.5f);
}

void DemoApp::CreateShaders()
{
	//Default VS
	ID3DBlob *pVSBlob = NULL;
	HR(LoadShaderBinaryFromFile("Shaders//vs.fxo", &pVSBlob));
	HR(md3dDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), NULL, &m_pVertexShader));
	InputLayouts::InitLayout(md3dDevice, pVSBlob, Vertex::POSNOR_INS);

	//Shadow map VS
	ID3DBlob *pShadowVSBlob = NULL;
	HR(LoadShaderBinaryFromFile("Shaders//shadowvs.fxo", &pShadowVSBlob));
	HR(md3dDevice->CreateVertexShader(pShadowVSBlob->GetBufferPointer(), pShadowVSBlob->GetBufferSize(), NULL, &m_pShadowMapVS));

	//Screen Quad VS
	ID3DBlob *pDebugTextureVSBlob = NULL;
	HR(LoadShaderBinaryFromFile("Shaders//debugtexturevs.fxo", &pDebugTextureVSBlob));
	HR(md3dDevice->CreateVertexShader(pDebugTextureVSBlob->GetBufferPointer(), pDebugTextureVSBlob->GetBufferSize(), NULL, &m_pDebugTextureVS));
	InputLayouts::InitLayout(md3dDevice, pDebugTextureVSBlob, Vertex::POSNORTEX);

	//Default PS
	ID3DBlob *pPSBlob = NULL;
	HR(LoadShaderBinaryFromFile("Shaders//ps.fxo", &pPSBlob));
	HR(md3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &m_pPixelShader));

	//Shadow map PS 
	ID3DBlob *pShadowPSBlob = NULL;
	HR(LoadShaderBinaryFromFile("Shaders//shadowps.fxo", &pShadowPSBlob));
	HR(md3dDevice->CreatePixelShader(pShadowPSBlob->GetBufferPointer(), pShadowPSBlob->GetBufferSize(), NULL, &m_pShadowMapPS));

	//Screen Quad PS
	ID3DBlob *pDebugTexturePSBlob = NULL;
	HR(LoadShaderBinaryFromFile("Shaders//debugtextureps.fxo", &pDebugTexturePSBlob));
	HR(md3dDevice->CreatePixelShader(pDebugTexturePSBlob->GetBufferPointer(), pDebugTexturePSBlob->GetBufferSize(), NULL, &m_pDebugTexturePS));


	ReleaseCOM(pVSBlob);
	ReleaseCOM(pPSBlob);
	ReleaseCOM(pShadowVSBlob);
	ReleaseCOM(pShadowPSBlob);
	ReleaseCOM(pDebugTextureVSBlob);
	ReleaseCOM(pDebugTexturePSBlob);
}

void DemoApp::CreateScreenQuad()
{
	Vertex::VertexPNT vertex[4];

	vertex[0] = Vertex::VertexPNT(
		-1.0f, -1.0f, 0.0f,
		0.0f, 0.0f, -1.0f,
		0.0f, 1.0f);

	vertex[1] = Vertex::VertexPNT(
		-1.0f, +1.0f, 0.0f,
		0.0f, 0.0f, -1.0f,
		0.0f, 0.0f);

	vertex[2] = Vertex::VertexPNT(
		+1.0f, +1.0f, 0.0f,
		0.0f, 0.0f, -1.0f,
		1.0f, 0.0f);

	vertex[3] = Vertex::VertexPNT(
		+1.0f, -1.0f, 0.0f,
		0.0f, 0.0f, -1.0f,
		1.0f, 1.0f);


	DWORD index[6] = 
	{
		0, 1, 2, 0, 2, 3
	};

	D3D11_BUFFER_DESC vertexBufferDesc;
	ZeroMemory(&vertexBufferDesc, sizeof(vertexBufferDesc));

	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof(Vertex::VertexPNT)* 4;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA vertexBufferData;
	ZeroMemory(&vertexBufferData, sizeof(vertexBufferData));
	vertexBufferData.pSysMem = &vertex[0];
	HR(md3dDevice->CreateBuffer(&vertexBufferDesc, &vertexBufferData, &m_pScreenQuadVB));


	D3D11_BUFFER_DESC indexBufferDesc;
	ZeroMemory(&indexBufferDesc, sizeof(indexBufferDesc));
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(DWORD)* 6;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = &index[0];
	HR(md3dDevice->CreateBuffer(&indexBufferDesc, &iinitData, &m_pScreenQuadIB));

}

void DemoApp::CreateGeometry()
{
	//Create pillars per vertex data
	GeoGenerator::Mesh pillar;
	GeoGenerator::GenCuboid(4, 16, 4, pillar);

	D3D11_BUFFER_DESC vertexDesc;
	ZeroMemory(&vertexDesc, sizeof(vertexDesc));
	vertexDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexDesc.ByteWidth = sizeof(Vertex::VertexPN) * pillar.vertices.size();
	vertexDesc.Usage = D3D11_USAGE_IMMUTABLE;
	vertexDesc.CPUAccessFlags = 0;
	vertexDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA vertexData;
	ZeroMemory(&vertexData, sizeof(vertexData));
	vertexData.pSysMem = &pillar.vertices[0];
	HR(md3dDevice->CreateBuffer(&vertexDesc, &vertexData, &m_pVertexBuffer));

	D3D11_BUFFER_DESC indexDesc;
	ZeroMemory(&indexDesc, sizeof(indexDesc));
	indexDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexDesc.ByteWidth = sizeof(DWORD)* pillar.indices.size();
	indexDesc.Usage = D3D11_USAGE_IMMUTABLE;
	indexDesc.CPUAccessFlags = 0;
	indexDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA indexData;
	ZeroMemory(&indexData, sizeof(indexData));
	indexData.pSysMem = &pillar.indices[0];
	HR(md3dDevice->CreateBuffer(&indexDesc, &indexData, &m_pIndexBuffer));

	//Create pillars per instance data
	std::vector<Vertex::VertexIns_Mat> matWorld;
	Vertex::VertexIns_Mat trans;
	for (int i = 0; i < 6; i++)
	{
		trans.mat = XMMatrixTranslation(0.0f, 8.0f, i * 8.0f - 24.0f);
		matWorld.push_back(trans);
	}

	D3D11_BUFFER_DESC instanceDesc;
	ZeroMemory(&instanceDesc, sizeof(instanceDesc));
	instanceDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	instanceDesc.ByteWidth = sizeof(Vertex::VertexIns_Mat) * matWorld.size();
	instanceDesc.Usage = D3D11_USAGE_IMMUTABLE;
	instanceDesc.CPUAccessFlags = 0;
	instanceDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA instanceData;
	ZeroMemory(&instanceData, sizeof(instanceData));
	instanceData.pSysMem = &matWorld[0];
	HR(md3dDevice->CreateBuffer(&instanceDesc, &instanceData, &m_pInstancedBuffer));

	//Create ground
	GeoGenerator::Mesh ground;
	GeoGenerator::GenCuboid(100, 2, 100, ground);

	ZeroMemory(&vertexDesc, sizeof(vertexDesc));
	vertexDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexDesc.ByteWidth = sizeof(Vertex::VertexPN) * ground.vertices.size();
	vertexDesc.Usage = D3D11_USAGE_IMMUTABLE;
	vertexDesc.CPUAccessFlags = 0;
	vertexDesc.MiscFlags = 0;

	ZeroMemory(&vertexData, sizeof(vertexData));
	vertexData.pSysMem = &ground.vertices[0];
	HR(md3dDevice->CreateBuffer(&vertexDesc, &vertexData, &m_pGroundVertexBuffer));

	ZeroMemory(&indexDesc, sizeof(indexDesc));
	indexDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexDesc.ByteWidth = sizeof(DWORD)* ground.indices.size();
	indexDesc.Usage = D3D11_USAGE_IMMUTABLE;
	indexDesc.CPUAccessFlags = 0;
	indexDesc.MiscFlags = 0;

	ZeroMemory(&indexData, sizeof(indexData));
	indexData.pSysMem = &ground.indices[0];
	HR(md3dDevice->CreateBuffer(&indexDesc, &indexData, &m_pGroundIndexBuffer));

	CreateScreenQuad();	
}

void DemoApp::CreateContantBuffers()
{
	D3D11_BUFFER_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.CPUAccessFlags = 0;
	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

	desc.ByteWidth = sizeof(CBNeverChanges);
	HR(md3dDevice->CreateBuffer(&desc, 0, &m_pCBNeverChanges));

	desc.ByteWidth = sizeof(CBOnResize);
	HR(md3dDevice->CreateBuffer(&desc, 0, &m_pCBOnResize));

	desc.ByteWidth = sizeof(CBPerFrame);
	HR(md3dDevice->CreateBuffer(&desc, 0, &m_pCBPerFrame));

	desc.ByteWidth = sizeof(CBPerObject);
	HR(md3dDevice->CreateBuffer(&desc, 0, &m_pCBPerObject));

	desc.ByteWidth = sizeof(CBPerObjectShadow);
	HR(md3dDevice->CreateBuffer(&desc, 0, &m_pCBPerObjShadow));

	desc.ByteWidth = sizeof(CBPerFrameScreenQuad);
	HR(md3dDevice->CreateBuffer(&desc, 0, &m_pCBPerFrameScreenQuad));
}

void DemoApp::CreateSamplerStates()
{
	D3D11_SAMPLER_DESC desc; 
	ZeroMemory(&desc, sizeof(desc));
	desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP; 
	desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	desc.MinLOD = 0;
	desc.MaxLOD = D3D11_FLOAT32_MAX;
	HR(md3dDevice->CreateSamplerState(&desc, &m_pSampleLinear));

	ZeroMemory(&desc, sizeof(desc));
	desc.Filter = D3D11_FILTER_COMPARISON_MIN_LINEAR_MAG_MIP_POINT;
	desc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	desc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	desc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
	desc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
	HR(md3dDevice->CreateSamplerState(&desc, &m_pSampleShadowMap));
}

void DemoApp::SetUpSceneConsts()
{
	//Set Invariant Constant Buffer
	CBNeverChanges cbNeverChanges;
	cbNeverChanges.dirLight = mDirLight;
	cbNeverChanges.shadowMapSize = static_cast<float>(mShadowMapSize);
	md3dImmediateContext->UpdateSubresource(m_pCBNeverChanges, 0, NULL, &cbNeverChanges, 0, 0);
	md3dImmediateContext->VSSetConstantBuffers(0, 1, &m_pCBNeverChanges);
	md3dImmediateContext->PSSetConstantBuffers(0, 1, &m_pCBNeverChanges);

	m_Proj = XMMatrixPerspectiveFovLH(XM_PIDIV4, mClientWidth / (float)mClientHeight, 0.01f, 1000.0f);

	BuildShadowMapMatrices();
}

void DemoApp::CreateRenderStates()
{
	RenderStates::InitAll(md3dDevice);
}

void DemoApp::BuildShadowMapMatrices()
{
	float aabbRadius = 90.0f;  // m_pAABB->GetRadius();
	XMVECTOR lightDir = XMLoadFloat3(&mDirLight.Direction);
	XMVECTOR lightPos = -1.0f * lightDir * aabbRadius;
	XMFLOAT3 tar = XMFLOAT3(0.0f, 0.0f, 0.0f);
	XMVECTOR targetPos = XMLoadFloat3(&tar);  //  &m_pAABB->Center);
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	mLightView = XMMatrixLookAtLH(lightPos, targetPos, up);

	// Transform bounding sphere to light space.
	XMFLOAT3 aabbCenterLightSpace;
	XMStoreFloat3(&aabbCenterLightSpace, XMVector3TransformCoord(targetPos, mLightView));

	//// Ortho frustum in light space encloses scene.
	float l = aabbCenterLightSpace.x - aabbRadius;
	float b = aabbCenterLightSpace.y - aabbRadius;
	float n = aabbCenterLightSpace.z - aabbRadius;
	float r = aabbCenterLightSpace.x + aabbRadius;
	float t = aabbCenterLightSpace.y + aabbRadius;
	float f = aabbCenterLightSpace.z + aabbRadius;
	mLightProj = XMMatrixOrthographicOffCenterLH(l, r, b, t, n, f);

	// Transform NDC space [-1,+1]^2 to texture space [0,1]^2
	mLightViewport = XMMATRIX(
		0.5f, 0.0f, 0.0f, 0.0f,
		0.0f, -0.5f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.5f, 0.5f, 0.0f, 1.0f);

	 mLightVPT = mLightView*mLightProj*mLightViewport;
}

void DemoApp::RenderMiniWindow()
{
	//Set Buffers, Layout, Topology and Render States
	UINT stride = sizeof(Vertex::VertexPNT);
	UINT offset = 0;
	md3dImmediateContext->IASetVertexBuffers(0, 1, &m_pScreenQuadVB, &stride, &offset);
	md3dImmediateContext->IASetIndexBuffer(m_pScreenQuadIB, DXGI_FORMAT_R32_UINT, 0);
	md3dImmediateContext->IASetInputLayout(InputLayouts::VertexPNT);
	md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	md3dImmediateContext->RSSetState(RenderStates::NoCullRS);

	//Resize mini window and translate to upper left corner
	XMMATRIX scale = XMMatrixScaling(1.0f / AspectRatio(), 1.0f, 1.0f);
	XMMATRIX world = XMMATRIX(
		0.25f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.25f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.25 / AspectRatio() - 1, 0.75f, 0.0f, 1.0f);

	CBPerFrameScreenQuad cbScreenQuadPerFrame;
	cbScreenQuadPerFrame.wvp = XMMatrixTranspose(scale * world);
	md3dImmediateContext->UpdateSubresource(m_pCBPerFrameScreenQuad, 0, NULL, &cbScreenQuadPerFrame, 0, 0);

	md3dImmediateContext->VSSetShader(m_pDebugTextureVS, NULL, 0);
	md3dImmediateContext->VSSetConstantBuffers(0, 1, &m_pCBPerFrameScreenQuad);

	md3dImmediateContext->PSSetShader(m_pDebugTexturePS, NULL, 0);
	md3dImmediateContext->PSSetShaderResources(0, 1, &m_pDepthSRV);
	md3dImmediateContext->PSSetSamplers(0, 1, &m_pSampleLinear);
	md3dImmediateContext->DrawIndexed(6, 0, 0);
}

void DemoApp::RenderShadowMap()
{
	//Draw pillars
	UINT strides[2] = { sizeof(Vertex::VertexPN), sizeof(Vertex::VertexIns_Mat) };
	UINT offsets[2] = {0, 0};
	ID3D11Buffer * buffers[2] = { m_pVertexBuffer, m_pInstancedBuffer };

	md3dImmediateContext->IASetVertexBuffers(0, 2, buffers, strides, offsets);
	md3dImmediateContext->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	md3dImmediateContext->IASetInputLayout(InputLayouts::VertexPN_INS);
	md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	md3dImmediateContext->RSSetState(RenderStates::ShadowMapDepthRS);

	CBPerObjectShadow cbPerObjShadow;
	cbPerObjShadow.lightWVP = XMMatrixTranspose(m_World * mLightView * mLightProj);
	cbPerObjShadow.isInstancing = 1;
	md3dImmediateContext->UpdateSubresource(m_pCBPerObjShadow, 0, NULL, &cbPerObjShadow, 0, 0);
	md3dImmediateContext->VSSetShader(m_pShadowMapVS, NULL, 0);
	md3dImmediateContext->VSSetConstantBuffers(3, 1, &m_pCBPerObjShadow);
	md3dImmediateContext->PSSetShader(m_pShadowMapPS, NULL, 0);
	md3dImmediateContext->DrawIndexedInstanced(36, 6, 0, 0, 0);

	UINT stride = sizeof(Vertex::VertexPN);
	UINT offset = 0;
	md3dImmediateContext->IASetVertexBuffers(0, 1, &m_pGroundVertexBuffer, &stride, &offset);
	md3dImmediateContext->IASetIndexBuffer(m_pGroundIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	cbPerObjShadow.lightWVP = XMMatrixTranspose(m_World * mLightView * mLightProj);
	cbPerObjShadow.isInstancing = 0;
	md3dImmediateContext->UpdateSubresource(m_pCBPerObjShadow, 0, NULL, &cbPerObjShadow, 0, 0);
	md3dImmediateContext->DrawIndexed(36, 0, 0);

}

bool DemoApp::Init()
{
	if (!DemoBase::Init())
		return false;

	m_pShadowMap = new ShadowMap(md3dDevice, mShadowMapSize, mShadowMapSize);
	m_pAABB = new AABB();
	CreateLights();
	CreateShaders();
	CreateGeometry();
	CreateContantBuffers();
	CreateSamplerStates();
	CreateRenderStates();
	SetUpSceneConsts();

	return true;
}

void DemoApp::UpdateScene(float dt)
{
	float x = mRadius*sinf(mPhi)*cosf(mTheta);
	float z = mRadius*sinf(mPhi)*sinf(mTheta) + 20.0f;
	float y = mRadius*cosf(mPhi);

	//Update Per Frame Constant Buffer
	CBPerFrame cbPerFrame;
	cbPerFrame.eyePos = XMFLOAT3(x, y, z);
	md3dImmediateContext->UpdateSubresource(m_pCBPerFrame, 0, NULL, &cbPerFrame, 0, 0);
	md3dImmediateContext->VSSetConstantBuffers(2, 1, &m_pCBPerFrame);
	md3dImmediateContext->PSSetConstantBuffers(2, 1, &m_pCBPerFrame);
	m_World = XMMatrixIdentity();

	XMVECTOR pos = XMVectorSet(x, y, z, 1.0f);
	XMVECTOR target = XMVectorZero();
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	m_View = XMMatrixLookAtLH(pos, target, up);
}

void DemoApp::DrawScene()
{
	assert(md3dImmediateContext);
	assert(mSwapChain);

	//Render shadow map
	m_pShadowMap->BindShadowMapDSV(md3dImmediateContext);
	RenderShadowMap();
	m_pDepthSRV = m_pShadowMap->DepthShaderResourceView();

	//Restore render targets
	md3dImmediateContext->RSSetState(0);
	ID3D11RenderTargetView* renderTargets[1] = { mRenderTargetView };
	md3dImmediateContext->OMSetRenderTargets(1, renderTargets, mDepthStencilView);
	md3dImmediateContext->RSSetViewports(1, &mScreenViewport);

	//Clear Render Targets
	float clearColor[4] = { 199.0f / 255.0f, 197.0f / 255.0f, 206.0f / 255.0f, 1.0f };
	//float clearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
	md3dImmediateContext->ClearRenderTargetView(mRenderTargetView, clearColor);
	md3dImmediateContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	//Set Buffers, Layout, Topology and Render States
	UINT strides[2] = { sizeof(Vertex::VertexPN), sizeof(Vertex::VertexIns_Mat) };
	UINT offsets[2] = { 0, 0 };
	ID3D11Buffer * buffers[2] = { m_pVertexBuffer, m_pInstancedBuffer };

	md3dImmediateContext->IASetVertexBuffers(0, 2, buffers, strides, offsets);
	md3dImmediateContext->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	md3dImmediateContext->IASetInputLayout(InputLayouts::VertexPN_INS);
	md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	md3dImmediateContext->RSSetState(RenderStates::NoCullRS);

	////Update Per Object Constant Buffer
	CBPerObject cbPerObj;
	cbPerObj.matWorld = XMMatrixTranspose(m_World);
	cbPerObj.matWorldInvTranspose = XMMatrixTranspose(MathHelper::InverseTranspose(m_World));
	cbPerObj.matWVP = XMMatrixTranspose(m_World * m_View * m_Proj);
	cbPerObj.matLightWVPT = XMMatrixTranspose(m_World * mLightVPT);
	cbPerObj.isInstancing = 1;
	cbPerObj.material.Ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	cbPerObj.material.Diffuse = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
	cbPerObj.material.Specular = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
	cbPerObj.material.Reflect = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	md3dImmediateContext->UpdateSubresource(m_pCBPerObject, 0, NULL, &cbPerObj, 0, 0);

	//Set Shaders and Resources
	md3dImmediateContext->VSSetShader(m_pVertexShader, NULL, 0);
	md3dImmediateContext->VSSetConstantBuffers(3, 1, &m_pCBPerObject);

	md3dImmediateContext->PSSetShader(m_pPixelShader, NULL, 0);
	md3dImmediateContext->PSSetConstantBuffers(3, 1, &m_pCBPerObject);

	md3dImmediateContext->PSSetShaderResources(2, 1, &m_pDepthSRV);
	md3dImmediateContext->PSSetSamplers(1, 1, &m_pSampleShadowMap);

	md3dImmediateContext->DrawIndexedInstanced(36, 6, 0, 0, 0);

	UINT stride = sizeof(Vertex::VertexPN);
	UINT offset = 0;

	md3dImmediateContext->IASetVertexBuffers(0, 1, &m_pGroundVertexBuffer, &stride, &offset);
	md3dImmediateContext->IASetIndexBuffer(m_pGroundIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	
	md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	md3dImmediateContext->RSSetState(RenderStates::NoCullRS);

	cbPerObj.isInstancing = 0;
	md3dImmediateContext->UpdateSubresource(m_pCBPerObject, 0, NULL, &cbPerObj, 0, 0);
	md3dImmediateContext->DrawIndexed(36, 0, 0);

	//Render mini window displaying shadow map
	RenderMiniWindow();

	HR(mSwapChain->Present(0, 0));

}


