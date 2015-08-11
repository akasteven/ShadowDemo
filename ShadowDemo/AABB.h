#pragma once

#include <D3DX11.h>
#include <xnamath.h>
#include "MathUtility.h"
#include "Vertex.h"

class AABB
{
public:

	AABB() :Center(0.0f, 0.0f, 0.0f), Extents(0.0f, 0.0f, 0.0f), Mins(0.0f, 0.0f, 0.0f), Maxs(0.0f, 0.0f, 0.0f){}
	AABB(const XMFLOAT3 &mins, const XMFLOAT3 &maxs) :Mins(mins), Maxs(maxs){ ComputeCenterExt(); }
	~AABB();

	void Clear(){ Mins = XMFLOAT3(0.0f, 0.0f, 0.0f); Maxs = XMFLOAT3(0.0f, 0.0f, 0.0f); }
	void BuildFromVertices(Vertex::VertexBase * vertex, int numVer);
	void AddVertex(const Vertex::VertexBase & vertex);
	void ComputeCenterExt();
	void ComputeMinMax();
	float GetRadius(){ return sqrtf( Extents.x * Extents.x + Extents.y * Extents.y + Extents.z * Extents.z ); }

public:

	XMFLOAT3 Center;
	XMFLOAT3 Extents;
	XMFLOAT3 Mins;
	XMFLOAT3 Maxs;
};

