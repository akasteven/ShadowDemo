#include "DemoApp.h"
#include "Vertex.h"
#include "RenderStates.h"
#include "ShadowMap.h"
#include "GeoGenerator.h"
#include "ConstantBufferDef.h"

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
m_pGroundSRV(0),
m_pPillarSRV(0),
m_pSampleLinear(0),
m_pSampleShadowMap(0),
m_pShadowMap(0),
mShadowMapSize(2048),
instanceCnt(100),
pillarSize(4.0f),
grndWidth(100.0f)
{
	grndLength = (instanceCnt + 4)* pillarSize * 2.0f;
	mRadius = grndLength * 0.55f;
	mTheta = float(-0.47f*MathHelper::Pi);
	mPhi = float(0.47f*MathHelper::Pi);
	this->mMainWndCaption = L"Demo";
}

DemoApp::~DemoApp()
{
	if (m_pShadowMap)
	{
		delete m_pShadowMap;
		m_pShadowMap = 0;
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
	ReleaseCOM(m_pGroundSRV);
	ReleaseCOM(m_pPillarSRV);

	InputLayouts::DestroyAll();
	RenderStates::DestroyAll();
}

void DemoApp::OnResize()
{
	DemoBase::OnResize();
	camera->Setup(XM_PIDIV4, mClientWidth / (float)mClientHeight, 0.01f, 1000.0f);
	//mProj = XMMatrixPerspectiveFovLH(XM_PIDIV4, mClientWidth / (float)mClientHeight, 0.01f, 1000.0f);
	md3dImmediateContext->VSSetConstantBuffers(1, 1, &m_pCBOnResize);
	md3dImmediateContext->PSSetConstantBuffers(1, 1, &m_pCBOnResize);
}

void DemoApp::CreateLights()
{
	mDirLight.Ambient = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
	mDirLight.Diffuse = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
	mDirLight.Specular = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
	mDirLight.Direction = XMFLOAT3(1.0f, -1.0f, 0.5f);
}

void DemoApp::CreateShaders()
{
	ID3DBlob *pBlob = NULL;

	//Default VS
	HR(LoadShaderBinaryFromFile("Shaders//vs.fxo", &pBlob));
	HR(md3dDevice->CreateVertexShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), NULL, &m_pVertexShader));
	InputLayouts::InitLayout(md3dDevice, pBlob, Vertex::POSNORTEX_INS);

	//Shadow map VS
	pBlob = NULL;
	HR(LoadShaderBinaryFromFile("Shaders//shadowvs.fxo", &pBlob));
	HR(md3dDevice->CreateVertexShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), NULL, &m_pShadowMapVS));

	//Screen Quad VS
	pBlob = NULL;
	HR(LoadShaderBinaryFromFile("Shaders//debugtexturevs.fxo", &pBlob));
	HR(md3dDevice->CreateVertexShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), NULL, &m_pDebugTextureVS));
	InputLayouts::InitLayout(md3dDevice, pBlob, Vertex::POSNORTEX);

	//Default PS
	pBlob = NULL;
	HR(LoadShaderBinaryFromFile("Shaders//ps.fxo", &pBlob));
	HR(md3dDevice->CreatePixelShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), NULL, &m_pPixelShader));

	//Shadow map PS 
	pBlob = NULL;
	HR(LoadShaderBinaryFromFile("Shaders//shadowps.fxo", &pBlob));
	HR(md3dDevice->CreatePixelShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), NULL, &m_pShadowMapPS));

	//Screen Quad PS
	pBlob = NULL;
	HR(LoadShaderBinaryFromFile("Shaders//debugtextureps.fxo", &pBlob));
	HR(md3dDevice->CreatePixelShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), NULL, &m_pDebugTexturePS));

	ReleaseCOM(pBlob);
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
	//Pillars per vertex data
	GeoGenerator::Mesh pillar;
	GeoGenerator::GenCuboid(pillarSize, pillarSize * 8.0f, pillarSize, pillar);

	//Vertex
	D3D11_BUFFER_DESC vertexDesc;
	ZeroMemory(&vertexDesc, sizeof(vertexDesc));
	vertexDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexDesc.ByteWidth = sizeof(Vertex::VertexPNT) * pillar.vertices.size();
	vertexDesc.Usage = D3D11_USAGE_IMMUTABLE;
	vertexDesc.CPUAccessFlags = 0;
	vertexDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA data;
	ZeroMemory(&data, sizeof(data));
	data.pSysMem = &pillar.vertices[0];
	HR(md3dDevice->CreateBuffer(&vertexDesc, &data, &m_pVertexBuffer));

	//Index
	D3D11_BUFFER_DESC indexDesc;
	ZeroMemory(&indexDesc, sizeof(indexDesc));
	indexDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexDesc.ByteWidth = sizeof(DWORD)* pillar.indices.size();
	indexDesc.Usage = D3D11_USAGE_IMMUTABLE;
	indexDesc.CPUAccessFlags = 0;
	indexDesc.MiscFlags = 0;

	data.pSysMem = &pillar.indices[0];
	HR(md3dDevice->CreateBuffer(&indexDesc, &data, &m_pIndexBuffer));

	//Pillars per instance data
	std::vector<Vertex::VertexIns_Mat> matWorld;
	Vertex::VertexIns_Mat trans;
	for (int i = 0; i < instanceCnt; i++)
	{
		trans.mat = XMMatrixTranslation(0.0f, pillarSize  * 4.0f , i *pillarSize * 2.0f - instanceCnt * pillarSize);
		matWorld.push_back(trans);
	}
	vertexDesc.ByteWidth = sizeof(Vertex::VertexIns_Mat) * matWorld.size();
	data.pSysMem = &matWorld[0];
	HR(md3dDevice->CreateBuffer(&vertexDesc, &data, &m_pInstancedBuffer));

	//Create ground geo
	GeoGenerator::Mesh ground;
	GeoGenerator::GenCuboid(grndWidth, 2, grndLength, ground);

	//Ground Vertex
	vertexDesc.ByteWidth = sizeof(Vertex::VertexPNT) * ground.vertices.size();
	data.pSysMem = &ground.vertices[0];
	HR(md3dDevice->CreateBuffer(&vertexDesc, &data, &m_pGroundVertexBuffer));

	//Ground Index
	indexDesc.ByteWidth = sizeof(DWORD)* ground.indices.size();
	data.pSysMem = &ground.indices[0];
	HR(md3dDevice->CreateBuffer(&indexDesc, &data, &m_pGroundIndexBuffer));

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
	HR(D3DX11CreateShaderResourceViewFromFile(md3dDevice, L"..//Resources//Ground.jpg", 0, 0, &m_pGroundSRV, 0));
	HR(D3DX11CreateShaderResourceViewFromFile(md3dDevice, L"..//Resources//Grey.jpg", 0, 0, &m_pPillarSRV, 0));

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

	camera->Setup(XM_PIDIV4, mClientWidth / (float)mClientHeight, 0.01f, 1000.0f);
	camera->SetPosition(XMFLOAT3(30.0f, 30.0f, -450.0f));

	BuildShadowMapMatrices();
}

void DemoApp::CreateRenderStates()
{
	RenderStates::InitAll(md3dDevice);
}

void DemoApp::BuildShadowMapMatrices()
{
	//Calculate the radius of the scene's bounding sphere. Approximately use the half of ground plane diagonal.
	float aabbRadius = sqrt(grndLength * grndLength + grndWidth * grndWidth) / 2.0f ;

	//Set up light parameter
	XMVECTOR lightDir = XMLoadFloat3(&mDirLight.Direction);
	XMVECTOR lightPos = -1.0f * lightDir * aabbRadius;
	XMFLOAT3 tar = XMFLOAT3(0.0f, 0.0f, 0.0f);
	XMVECTOR targetPos = XMLoadFloat3(&tar); 
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
	UINT strides[2] = { sizeof(Vertex::VertexPNT), sizeof(Vertex::VertexIns_Mat) };
	UINT offsets[2] = {0, 0};
	ID3D11Buffer * buffers[2] = { m_pVertexBuffer, m_pInstancedBuffer };

	md3dImmediateContext->IASetVertexBuffers(0, 2, buffers, strides, offsets);
	md3dImmediateContext->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	md3dImmediateContext->IASetInputLayout(InputLayouts::VertexPNT_INS);
	md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	md3dImmediateContext->RSSetState(RenderStates::ShadowMapDepthRS);

	CBPerObjectShadow cbPerObjShadow;
	cbPerObjShadow.lightWVP = XMMatrixTranspose(mWorld * mLightView * mLightProj);
	cbPerObjShadow.isInstancing = 1;
	md3dImmediateContext->UpdateSubresource(m_pCBPerObjShadow, 0, NULL, &cbPerObjShadow, 0, 0);
	md3dImmediateContext->VSSetShader(m_pShadowMapVS, NULL, 0);
	md3dImmediateContext->VSSetConstantBuffers(3, 1, &m_pCBPerObjShadow);
	md3dImmediateContext->PSSetShader(m_pShadowMapPS, NULL, 0);

	md3dImmediateContext->DrawIndexedInstanced(36, instanceCnt, 0, 0, 0);

	UINT stride = sizeof(Vertex::VertexPNT);
	UINT offset = 0;
	md3dImmediateContext->IASetVertexBuffers(0, 1, &m_pGroundVertexBuffer, &stride, &offset);
	md3dImmediateContext->IASetIndexBuffer(m_pGroundIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	cbPerObjShadow.lightWVP = XMMatrixTranspose(mWorld * mLightView * mLightProj);
	cbPerObjShadow.isInstancing = 0;
	md3dImmediateContext->UpdateSubresource(m_pCBPerObjShadow, 0, NULL, &cbPerObjShadow, 0, 0);

	md3dImmediateContext->DrawIndexed(36, 0, 0);
}

bool DemoApp::Init()
{
	if (!DemoBase::Init())
		return false;

	m_pShadowMap = new ShadowMap(md3dDevice, mShadowMapSize, mShadowMapSize);
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
	if (GetAsyncKeyState('W') & 0x8000)
		camera->MoveForward(10.0f * dt);
	if (GetAsyncKeyState('S') & 0x8000)
		camera->MoveForward(-10.0f*dt);
	if (GetAsyncKeyState('D') & 0x8000)
		camera->MoveRight(10.0f *dt);
	if (GetAsyncKeyState('A') & 0x8000)
		camera->MoveRight(-10.0f *dt);
	if (GetAsyncKeyState('E') & 0x8000)
		camera->Elevate(10.0f *dt);
	if (GetAsyncKeyState('Q') & 0x8000)
		camera->Elevate(-10.0f *dt);

	//Update Per Frame Constant Buffer
	CBPerFrame cbPerFrame;
	cbPerFrame.eyePos = camera->GetPos();
	md3dImmediateContext->UpdateSubresource(m_pCBPerFrame, 0, NULL, &cbPerFrame, 0, 0);
	md3dImmediateContext->VSSetConstantBuffers(2, 1, &m_pCBPerFrame);
	md3dImmediateContext->PSSetConstantBuffers(2, 1, &m_pCBPerFrame);
	mWorld = XMMatrixIdentity();

	camera->Update();
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
	md3dImmediateContext->ClearRenderTargetView(mRenderTargetView, clearColor);
	md3dImmediateContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	
	//Set Render State
	md3dImmediateContext->RSSetState(RenderStates::CullClockwiseRS);

	//Set Vertex and Index Buffers
	UINT strides[2] = { sizeof(Vertex::VertexPNT), sizeof(Vertex::VertexIns_Mat) };
	UINT offsets[2] = { 0, 0 };
	ID3D11Buffer * buffers[2] = { m_pVertexBuffer, m_pInstancedBuffer };
	md3dImmediateContext->IASetVertexBuffers(0, 2, buffers, strides, offsets);
	md3dImmediateContext->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	//Update Per Object Constant Buffer
	CBPerObject cbPerObj;
	cbPerObj.matWorld = XMMatrixTranspose(mWorld);
	cbPerObj.matWorldInvTranspose = XMMatrixTranspose(MathHelper::InverseTranspose(mWorld));
	cbPerObj.matWVP = XMMatrixTranspose(mWorld * camera->GetViewProjMatrix());
	cbPerObj.matLightWVPT = XMMatrixTranspose(mWorld * mLightVPT);
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
	md3dImmediateContext->PSSetShaderResources(0, 1, &m_pPillarSRV);
	md3dImmediateContext->PSSetShaderResources(2, 1, &m_pDepthSRV);
	md3dImmediateContext->PSSetSamplers(0, 1, &m_pSampleLinear);
	md3dImmediateContext->PSSetSamplers(1, 1, &m_pSampleShadowMap);

	//Draw Pillars
	md3dImmediateContext->DrawIndexedInstanced(36, instanceCnt, 0, 0, 0);

	UINT stride = sizeof(Vertex::VertexPNT);
	UINT offset = 0;
	md3dImmediateContext->IASetVertexBuffers(0, 1, &m_pGroundVertexBuffer, &stride, &offset);
	md3dImmediateContext->IASetIndexBuffer(m_pGroundIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	cbPerObj.isInstancing = 0;
	md3dImmediateContext->UpdateSubresource(m_pCBPerObject, 0, NULL, &cbPerObj, 0, 0);
	md3dImmediateContext->PSSetShaderResources(0, 1, &m_pGroundSRV);

	//Draw Ground
	md3dImmediateContext->DrawIndexed(36, 0, 0);

	//Render mini window displaying shadow map
	RenderMiniWindow();

	HR(mSwapChain->Present(0, 0));

}


