#include "CascadedShadowMap.h"
#include "Camera.h"
#include "AABB.h"
#include "Frustum.h"

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
	//Transform the corners into light camera space
	for (int i = 0; i < sceneAABBCorners.size() ; i++)
		sceneAABBCorners[i] = XMVector4Transform(sceneAABBCorners[i], m_pLightCamera->GetViewMatrix());

	float viewDepthRange = m_pViewCamera->GetFarZ() - m_pViewCamera->GetNearZ();

	XMVECTOR det;
	XMMATRIX matInverseViewCamera = XMMatrixInverse(&det, m_pViewCamera->GetViewMatrix());

	//Calculate the subfrustrum 
	float subFrustrumBegin = 0.0f;
	for (int i = 0; i < m_pConfig->mCascadesCnt; ++i)
	{
		float subFrustrumEnd = m_pConfig->mSubFrustumCoveragePct[i];
		subFrustrumBegin /= m_pConfig->mSceneCoveragePct;
		subFrustrumEnd /= m_pConfig->mSceneCoveragePct;

		subFrustrumBegin *= viewDepthRange;
		subFrustrumEnd *= viewDepthRange;

		//Compute corners for current sub-frustum
		std::vector<XMVECTOR> subFrustumCorners;
		Frustum::ComputeFrustumCorners(subFrustrumBegin, subFrustrumEnd, m_pViewCamera->GetProjMatrix(), subFrustumCorners);
		
		//Transform corners to light camera space
		for (int i = 0; i < subFrustumCorners.size(); i++)
		{
			//Transform from view camera space to world space
			subFrustumCorners[i] = XMVector4Transform(subFrustumCorners[i], matInverseViewCamera * m_pLightCamera->GetViewMatrix());
		}

		AABB * subFrustumAABB = new AABB();
		subFrustumAABB->BuildFromVertices();
	}



}