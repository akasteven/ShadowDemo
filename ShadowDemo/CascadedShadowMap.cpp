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



void CascadedShadowMap::ComputeTightNearFar(float& fNearPlane,
	float& fFarPlane,
	FXMVECTOR vLightCameraOrthographicMin,
	FXMVECTOR vLightCameraOrthographicMax,
	XMVECTOR* pvPointsInCameraView)
{

	// Initialize the near and far planes
	fNearPlane = FLT_MAX;
	fFarPlane = -FLT_MAX;

	Triangle triangleList[16];
	INT iTriangleCnt = 1;

	triangleList[0].pt[0] = pvPointsInCameraView[0];
	triangleList[0].pt[1] = pvPointsInCameraView[1];
	triangleList[0].pt[2] = pvPointsInCameraView[2];
	triangleList[0].culled = false;

	// These are the indices used to tesselate an AABB into a list of triangles.
	static const INT iAABBTriIndexes[] =
	{
		0, 1, 2, 1, 2, 3,
		4, 5, 6, 5, 6, 7,
		0, 2, 4, 2, 4, 6,
		1, 3, 5, 3, 5, 7,
		0, 1, 4, 1, 4, 5,
		2, 3, 6, 3, 6, 7
	};

	INT iPointPassesCollision[3];

	// At a high level: 
	// 1. Iterate over all 12 triangles of the AABB.  
	// 2. Clip the triangles against each plane. Create new triangles as needed.
	// 3. Find the min and max z values as the near and far plane.

	//This is easier because the triangles are in camera spacing making the collisions tests simple comparisions.

	float fLightCameraOrthographicMinX = XMVectorGetX(vLightCameraOrthographicMin);
	float fLightCameraOrthographicMaxX = XMVectorGetX(vLightCameraOrthographicMax);
	float fLightCameraOrthographicMinY = XMVectorGetY(vLightCameraOrthographicMin);
	float fLightCameraOrthographicMaxY = XMVectorGetY(vLightCameraOrthographicMax);

	for (INT AABBTriIter = 0; AABBTriIter < 12; ++AABBTriIter)
	{

		triangleList[0].pt[0] = pvPointsInCameraView[iAABBTriIndexes[AABBTriIter * 3 + 0]];
		triangleList[0].pt[1] = pvPointsInCameraView[iAABBTriIndexes[AABBTriIter * 3 + 1]];
		triangleList[0].pt[2] = pvPointsInCameraView[iAABBTriIndexes[AABBTriIter * 3 + 2]];
		iTriangleCnt = 1;
		triangleList[0].culled = FALSE;

		// Clip each invidual triangle against the 4 frustums.  When ever a triangle is clipped into new triangles, 
		//add them to the list.
		for (INT frustumPlaneIter = 0; frustumPlaneIter < 4; ++frustumPlaneIter)
		{

			FLOAT fEdge;
			INT iComponent;

			if (frustumPlaneIter == 0)
			{
				fEdge = fLightCameraOrthographicMinX; // todo make float temp
				iComponent = 0;
			}
			else if (frustumPlaneIter == 1)
			{
				fEdge = fLightCameraOrthographicMaxX;
				iComponent = 0;
			}
			else if (frustumPlaneIter == 2)
			{
				fEdge = fLightCameraOrthographicMinY;
				iComponent = 1;
			}
			else
			{
				fEdge = fLightCameraOrthographicMaxY;
				iComponent = 1;
			}

			for (INT triIter = 0; triIter < iTriangleCnt; ++triIter)
			{
				// We don't delete triangles, so we skip those that have been culled.
				if (!triangleList[triIter].culled)
				{
					INT iInsideVertCount = 0;
					XMVECTOR tempOrder;
					// Test against the correct frustum plane.
					// This could be written more compactly, but it would be harder to understand.

					if (frustumPlaneIter == 0)
					{
						for (INT triPtIter = 0; triPtIter < 3; ++triPtIter)
						{
							if (XMVectorGetX(triangleList[triIter].pt[triPtIter]) >
								XMVectorGetX(vLightCameraOrthographicMin))
							{
								iPointPassesCollision[triPtIter] = 1;
							}
							else
							{
								iPointPassesCollision[triPtIter] = 0;
							}
							iInsideVertCount += iPointPassesCollision[triPtIter];
						}
					}
					else if (frustumPlaneIter == 1)
					{
						for (INT triPtIter = 0; triPtIter < 3; ++triPtIter)
						{
							if (XMVectorGetX(triangleList[triIter].pt[triPtIter]) <
								XMVectorGetX(vLightCameraOrthographicMax))
							{
								iPointPassesCollision[triPtIter] = 1;
							}
							else
							{
								iPointPassesCollision[triPtIter] = 0;
							}
							iInsideVertCount += iPointPassesCollision[triPtIter];
						}
					}
					else if (frustumPlaneIter == 2)
					{
						for (INT triPtIter = 0; triPtIter < 3; ++triPtIter)
						{
							if (XMVectorGetY(triangleList[triIter].pt[triPtIter]) >
								XMVectorGetY(vLightCameraOrthographicMin))
							{
								iPointPassesCollision[triPtIter] = 1;
							}
							else
							{
								iPointPassesCollision[triPtIter] = 0;
							}
							iInsideVertCount += iPointPassesCollision[triPtIter];
						}
					}
					else
					{
						for (INT triPtIter = 0; triPtIter < 3; ++triPtIter)
						{
							if (XMVectorGetY(triangleList[triIter].pt[triPtIter]) <
								XMVectorGetY(vLightCameraOrthographicMax))
							{
								iPointPassesCollision[triPtIter] = 1;
							}
							else
							{
								iPointPassesCollision[triPtIter] = 0;
							}
							iInsideVertCount += iPointPassesCollision[triPtIter];
						}
					}

					// Move the points that pass the frustum test to the begining of the array.
					if (iPointPassesCollision[1] && !iPointPassesCollision[0])
					{
						tempOrder = triangleList[triIter].pt[0];
						triangleList[triIter].pt[0] = triangleList[triIter].pt[1];
						triangleList[triIter].pt[1] = tempOrder;
						iPointPassesCollision[0] = TRUE;
						iPointPassesCollision[1] = FALSE;
					}
					if (iPointPassesCollision[2] && !iPointPassesCollision[1])
					{
						tempOrder = triangleList[triIter].pt[1];
						triangleList[triIter].pt[1] = triangleList[triIter].pt[2];
						triangleList[triIter].pt[2] = tempOrder;
						iPointPassesCollision[1] = TRUE;
						iPointPassesCollision[2] = FALSE;
					}
					if (iPointPassesCollision[1] && !iPointPassesCollision[0])
					{
						tempOrder = triangleList[triIter].pt[0];
						triangleList[triIter].pt[0] = triangleList[triIter].pt[1];
						triangleList[triIter].pt[1] = tempOrder;
						iPointPassesCollision[0] = TRUE;
						iPointPassesCollision[1] = FALSE;
					}

					if (iInsideVertCount == 0)
					{ // All points failed. We're done,  
						triangleList[triIter].culled = true;
					}
					else if (iInsideVertCount == 1)
					{// One point passed. Clip the triangle against the Frustum plane
						triangleList[triIter].culled = false;

						// 
						XMVECTOR vVert0ToVert1 = triangleList[triIter].pt[1] - triangleList[triIter].pt[0];
						XMVECTOR vVert0ToVert2 = triangleList[triIter].pt[2] - triangleList[triIter].pt[0];

						// Find the collision ratio.
						FLOAT fHitPointTimeRatio = fEdge - XMVectorGetByIndex(triangleList[triIter].pt[0], iComponent);
						// Calculate the distance along the vector as ratio of the hit ratio to the component.
						FLOAT fDistanceAlongVector01 = fHitPointTimeRatio / XMVectorGetByIndex(vVert0ToVert1, iComponent);
						FLOAT fDistanceAlongVector02 = fHitPointTimeRatio / XMVectorGetByIndex(vVert0ToVert2, iComponent);
						// Add the point plus a percentage of the vector.
						vVert0ToVert1 *= fDistanceAlongVector01;
						vVert0ToVert1 += triangleList[triIter].pt[0];
						vVert0ToVert2 *= fDistanceAlongVector02;
						vVert0ToVert2 += triangleList[triIter].pt[0];

						triangleList[triIter].pt[1] = vVert0ToVert2;
						triangleList[triIter].pt[2] = vVert0ToVert1;

					}
					else if (iInsideVertCount == 2)
					{ // 2 in  // tesselate into 2 triangles


						// Copy the triangle\(if it exists) after the current triangle out of
						// the way so we can override it with the new triangle we're inserting.
						triangleList[iTriangleCnt] = triangleList[triIter + 1];

						triangleList[triIter].culled = false;
						triangleList[triIter + 1].culled = false;

						// Get the vector from the outside point into the 2 inside points.
						XMVECTOR vVert2ToVert0 = triangleList[triIter].pt[0] - triangleList[triIter].pt[2];
						XMVECTOR vVert2ToVert1 = triangleList[triIter].pt[1] - triangleList[triIter].pt[2];

						// Get the hit point ratio.
						FLOAT fHitPointTime_2_0 = fEdge - XMVectorGetByIndex(triangleList[triIter].pt[2], iComponent);
						FLOAT fDistanceAlongVector_2_0 = fHitPointTime_2_0 / XMVectorGetByIndex(vVert2ToVert0, iComponent);
						// Calcaulte the new vert by adding the percentage of the vector plus point 2.
						vVert2ToVert0 *= fDistanceAlongVector_2_0;
						vVert2ToVert0 += triangleList[triIter].pt[2];

						// Add a new triangle.
						triangleList[triIter + 1].pt[0] = triangleList[triIter].pt[0];
						triangleList[triIter + 1].pt[1] = triangleList[triIter].pt[1];
						triangleList[triIter + 1].pt[2] = vVert2ToVert0;

						//Get the hit point ratio.
						FLOAT fHitPointTime_2_1 = fEdge - XMVectorGetByIndex(triangleList[triIter].pt[2], iComponent);
						FLOAT fDistanceAlongVector_2_1 = fHitPointTime_2_1 / XMVectorGetByIndex(vVert2ToVert1, iComponent);
						vVert2ToVert1 *= fDistanceAlongVector_2_1;
						vVert2ToVert1 += triangleList[triIter].pt[2];
						triangleList[triIter].pt[0] = triangleList[triIter + 1].pt[1];
						triangleList[triIter].pt[1] = triangleList[triIter + 1].pt[2];
						triangleList[triIter].pt[2] = vVert2ToVert1;
						// Cncrement triangle count and skip the triangle we just inserted.
						++iTriangleCnt;
						++triIter;


					}
					else
					{ // all in
						triangleList[triIter].culled = false;

					}
				}// end if !culled loop            
			}
		}
		for (INT index = 0; index < iTriangleCnt; ++index)
		{
			if (!triangleList[index].culled)
			{
				// Set the near and far plan and the min and max z values respectivly.
				for (int vertind = 0; vertind < 3; ++vertind)
				{
					float fTriangleCoordZ = XMVectorGetZ(triangleList[index].pt[vertind]);
					if (fNearPlane > fTriangleCoordZ)
					{
						fNearPlane = fTriangleCoordZ;
					}
					if (fFarPlane  <fTriangleCoordZ)
					{
						fFarPlane = fTriangleCoordZ;
					}
				}
			}
		}
	}

}

void CascadedShadowMap::BuildCascadeProj( AABB * aabb )
{
	m_pSceneAABB = aabb;

	//Get the eight corners of the scene AABB
	std::vector<XMVECTOR> sceneAABBCornersLightSpace;
	aabb->GetCorners(sceneAABBCornersLightSpace);
	//Transform the corners into light camera space
	for (int i = 0; i < sceneAABBCornersLightSpace.size(); i++)
		sceneAABBCornersLightSpace[i] = XMVector4Transform(sceneAABBCornersLightSpace[i], m_pLightCamera->GetViewMatrix());

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
			subFrustumCorners[i] = XMVector4Transform(subFrustumCorners[i], matInverseViewCamera * m_pLightCamera->GetViewMatrix());

		//Generate AABB for sub-frustum in light camera space
		AABB * subFrustumAABB = new AABB();
		subFrustumAABB->BuildFromVertices(subFrustumCorners);

		//Compute a tight depth bound for the current cascade
		float nearZ, farZ;
		ComputeTightNearFar(nearZ, farZ, subFrustumAABB, sceneAABBCornersLightSpace);

		subFrustrumBegin = m_pConfig->mSubFrustumCoveragePct[i];
	}



}