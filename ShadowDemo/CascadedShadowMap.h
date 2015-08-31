#pragma once

#include <D3DX11.h>
#include <xnamath.h>
#include <vector>

static const int MAXCASCADECNT = 8;

class Camera;
class AABB;


struct CSMConfig
{
	UINT mCascadesCnt;      //Number of cascades
	float mSceneCoveragePct;   // Percentage of the whole view( in z direction ) to apply CSM
	float mCascadeResolution;   //Shadow map resolution for each cascade
	float mSubFrustumCoveragePct[MAXCASCADECNT];   //Percentage of coverage( in z direction ) for each cascades
}; 

class CascadedShadowMap
{
public:
	CascadedShadowMap(ID3D11Device *, ID3D11DeviceContext * , CSMConfig * , Camera *viewCamera, Camera * lightCamera);
	~CascadedShadowMap();

	//Partition View Frustrum
	void BuildCascadeProj(AABB * aabb);

private:

	void ComputeSubFrustumCorners(float beg, float end, const XMMATRIX & proj, std::vector<XMVECTOR> & corners);

	//Configuration of CSM
	CSMConfig * m_pConfig;

	//Scene view camera and light view camera
	Camera * m_pViewCamera;
	Camera * m_pLightCamera;
	
	//AABB of the scene 
	AABB * m_pSceneAABB;

	//Light camera projection matrices for each cascade
	XMMATRIX mCascadesOrthoProj[MAXCASCADECNT];

	//Scene view space thresholds for frustrum partition
	float mFrustumPartitionDepth[MAXCASCADECNT];

	ID3D11Device *m_pDevice;
	ID3D11DeviceContext * m_pContext;
};

