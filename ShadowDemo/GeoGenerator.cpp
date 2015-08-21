#include "GeoGenerator.h"


GeoGenerator::GeoGenerator()
{
}


GeoGenerator::~GeoGenerator()
{
}


void GeoGenerator::GenCuboid(float w, float h, float d, Mesh & mesh)
{
	float hw = w / 2.0f;
	float hh = h / 2.0f;
	float hd = d / 2.0f;

	Vertex::VertexPNT v[24] = {

		//front
		Vertex::VertexPNT( -hw, -hh, -hd, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f ),
		Vertex::VertexPNT( hw, -hh, -hd, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f),
		Vertex::VertexPNT( hw, hh, -hd, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f ),
		Vertex::VertexPNT( -hw, hh, -hd, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f),

		//back
		Vertex::VertexPNT(-hw, -hh, hd, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f),
		Vertex::VertexPNT( hw, -hh, hd, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f),
		Vertex::VertexPNT( hw, hh, hd, 0.0f, 0.0f, 1.0f, 0.0f , 0.0f),
		Vertex::VertexPNT( -hw, hh, hd, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f),

		//top
		Vertex::VertexPNT(-hw, hh, -hd, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f),
		Vertex::VertexPNT(hw, hh, -hd, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f),
		Vertex::VertexPNT(hw, hh, hd, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f),
		Vertex::VertexPNT(-hw, hh, hd, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f),

		//bottom
		Vertex::VertexPNT(-hw, hh, -hd, 0.0f, -1.0f, 0.0f, 1.0f, 1.0f),
		Vertex::VertexPNT(hw, hh, -hd, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f),
		Vertex::VertexPNT(hw, hh, hd, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f),
		Vertex::VertexPNT(-hw, hh, hd, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f),

		//left
		Vertex::VertexPNT(-hw, -hh, hd, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f),
		Vertex::VertexPNT(-hw, -hh, -hd, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f),
		Vertex::VertexPNT(-hw, hh, -hd, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f),
		Vertex::VertexPNT(-hw, hh, hd, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f),

		//right
		Vertex::VertexPNT(hw, -hh, hd, 1.0f, -1.0f, 0.0f, 1.0f, 1.0f),
		Vertex::VertexPNT(hw, -hh, -hd, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f),
		Vertex::VertexPNT(hw, hh, -hd, 1.0f, -1.0f, 0.0f, 0.0f, 0.0f),
		Vertex::VertexPNT(hw, hh, hd, 1.0f, -1.0f, 0.0f, 1.0f, 0.0f),
	};
	mesh.vertices.assign( v, v + 24);

	DWORD i[36] = 
	{
		0, 1, 2,
		0, 2, 3,
		5, 4, 7,
		5, 7, 6,
		8, 9, 10,
		8, 10, 11,
		13, 12, 15,
		13, 15, 14,
		16, 17, 18,
		16, 18, 19,
		21, 20, 23,
		21, 23, 22
	};

	mesh.indices.assign( i , i + 36);
}