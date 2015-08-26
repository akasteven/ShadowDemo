#pragma once

#include "DemoBase.h"

class ShadowMap;

class DemoApp :
	public DemoBase
{
public:
	DemoApp(HINSTANCE hInstance);
	~DemoApp();

	bool Init();
	void OnResize();
	void UpdateScene(float dt);
	void DrawScene();

private:

	//Common setups
	void CreateGeometry();
	void CreateScreenQuad();
	void CreateShaders();
	void CreateContantBuffers();
	void CreateRenderStates();
	void CreateSamplerStates();
	void SetUpSceneConsts();
	void CreateLights();

	//Shadow Map
	void RenderShadowMap();
	void BuildShadowMapMatrices();

	//Screen debug mini window
	void RenderMiniWindow();

private:

	//Shaders
	ID3D11VertexShader * m_pVertexShader;
	ID3D11PixelShader * m_pPixelShader;
	ID3D11VertexShader * m_pShadowMapVS;
	ID3D11PixelShader * m_pShadowMapPS;
	ID3D11VertexShader * m_pDebugTextureVS;
	ID3D11PixelShader * m_pDebugTexturePS;

	//Shader Constant Buffers
	ID3D11Buffer * m_pCBNeverChanges;
	ID3D11Buffer * m_pCBOnResize;
	ID3D11Buffer * m_pCBPerFrame;
	ID3D11Buffer * m_pCBPerObject;
	ID3D11Buffer * m_pCBPerObjShadow;
	ID3D11Buffer * m_pCBPerFrameScreenQuad;

	//Vertex and Index Buffers
	ID3D11Buffer* m_pScreenQuadVB;
	ID3D11Buffer* m_pScreenQuadIB;
	ID3D11Buffer * m_pVertexBuffer;
	ID3D11Buffer * m_pIndexBuffer;
	ID3D11Buffer *m_pInstancedBuffer;
	ID3D11Buffer * m_pGroundVertexBuffer;
	ID3D11Buffer * m_pGroundIndexBuffer;

	//Shader Resources
	ID3D11ShaderResourceView * m_pGroundSRV; 
	ID3D11ShaderResourceView * m_pPillarSRV;
	ID3D11ShaderResourceView * m_pDepthSRV;

	//Sampler States
	ID3D11SamplerState * m_pSampleLinear;
	ID3D11SamplerState * m_pSampleShadowMap;

	//Matrices
	XMMATRIX mLightVPT;
	XMMATRIX mLightView;
	XMMATRIX mLightProj;
	XMMATRIX mLightViewport;
	XMMATRIX mWorld;
	XMMATRIX mView;
	XMMATRIX mProj;

	//Lights
	DirectionalLight mDirLight;
	PointLight mPointLight;

	//Shadow Map
	ShadowMap *m_pShadowMap;
	int  mShadowMapSize;

	//Scene Parameter
	UINT instanceCnt;
	float pillarSize;
	float grndLength;
	float grndWidth;
};

