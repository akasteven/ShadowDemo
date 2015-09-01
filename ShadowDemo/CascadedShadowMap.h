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
	
	//Used for ComputeTightNearFar function
	struct Triangle
	{
		XMVECTOR pt[3];
		BOOL culled;
	};

	//Compute tight near and far planes for each orthographic projection by insecting subfrustum AABB with scene AABB in light camera space
	void ComputeTightNearFar(float& fNearPlane, float& fFarPlane, AABB * subFrustumAABB, std::vector<XMVECTOR> &sceneAABBCorners);

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

