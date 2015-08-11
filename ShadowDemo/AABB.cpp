#include "AABB.h"


AABB::~AABB()
{
}

void AABB::BuildFromVertices(Vertex::VertexBase * vertex, int numVer)
{
	Clear();
	if (numVer <= 0)
		return;

	for (int i = 0; i < numVer; i++)
	{
		Maxs = MathHelper::VectorMax(Maxs, vertex[i].Pos);
		Mins = MathHelper::VectorMin(Maxs, vertex[i].Pos);
	}
}

void AABB::AddVertex(const Vertex::VertexBase & vertex)
{
	if (vertex.Pos.x >Maxs.x)	Maxs.x = vertex.Pos.x;
	if (vertex.Pos.x <Mins.x)   Mins.x = vertex.Pos.x;

	if (vertex.Pos.y >Maxs.y)  Maxs.y = vertex.Pos.y;
	if (vertex.Pos.y <Mins.y)  Mins.y = vertex.Pos.y;

	if (vertex.Pos.z >Maxs.z)   Maxs.z = vertex.Pos.z;
	if (vertex.Pos.z <Mins.z)  Mins.z = vertex.Pos.z;
}

void AABB::ComputeCenterExt()
{
	Center = XMFLOAT3( ( Maxs.x + Mins.x ) * 0.5f, ( Maxs.y + Mins.y ) * 0.5f, ( Maxs.z + Mins.z ) * 0.5f) ;
	Extents = XMFLOAT3(Maxs.x - Center.x, Maxs.y - Center.y, Maxs.z - Center.z);
}

void AABB::ComputeMinMax()
{
	Mins = XMFLOAT3(Center.x - Extents.x, Center.y - Extents.y, Center.z - Extents.z);
	Maxs = XMFLOAT3(Center.x + Extents.x, Center.y + Extents.y, Center.z + Extents.z);
}