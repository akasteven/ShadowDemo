#include "CascadedShadowMap.h"
#include "Camera.h"
#include "AABB.h"

CascadedShadowMap::CascadedShadowMap(ID3D11Device *device, ID3D11DeviceContext * context, CSMConfig * config, Camera * viewCamera, Camera * lightCamera)
: m_pDevice(device),
m_pContext(context),
m_pConfig(config),
m_pLightCamera(lightCamera),
m_pViewCamera (viewCamera)
{

}


CascadedShadowMap::~CascadedShadowMap()
{
}



void CascadedShadowMap::BuildCascadeProj( AABB * aabb )
{
	m_pSceneAABB = aabb;

	//Get the eight corners of the scene AABB
	std::vector<XMVECTOR> sceneAABBCorners;
	aabb->GetCorners(sceneAABBCorners);
	//Transform the corners into light space
	for (int i = 0; i < 8; i++)
		sceneAABBCorners[i] = XMVector4Transform(sceneAABBCorners[i], m_pLightCamera->GetViewMatrix());

	float viewDepthRange = m_pViewCamera->GetFarZ() - m_pViewCamera->GetNearZ();

	//Calculate the subfrustrum 
	float subFrustrumBegin = 0.0f;
	for (int i = 0; i < m_pConfig->mCascadesCnt; ++i)
	{
		float subFrustrumEnd = m_pConfig->mSubFrustumCoveragePct[i];
		subFrustrumBegin /= m_pConfig->mSceneCoveragePct;
		subFrustrumEnd /= m_pConfig->mSceneCoveragePct;

		subFrustrumBegin *= viewDepthRange;
		subFrustrumEnd *= viewDepthRange;



	}



}