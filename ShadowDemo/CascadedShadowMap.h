#pragma once

#include <D3DX11.h>
#include <xnamath.h>
#include <vector>


static const int MAXCASCADECNT = 8;


class Camera;

struct CSMConfig
{
	UINT mCascadesCnt;
	float mCascadesSize;
	float mPartitionPercent[MAXCASCADECNT];
};

class CascadedShadowMap
{
public:
	CascadedShadowMap();
	~CascadedShadowMap();

	//Partition View Frustrum
	void BuildCascadeProj();

private:

	//Configuration of CSM
	CSMConfig * mConfig;

	//Scene view camera and light view camera
	Camera * m_pViewCamera;
	Camera * m_pLightCamera;

	//Light camera projection matrices for each cascade
	XMMATRIX mCascadesOrthoProj[MAXCASCADECNT];

	//Scene view space thresholds for frustrum partition
	float mFrustumPartitionDepth[MAXCASCADECNT];
};

