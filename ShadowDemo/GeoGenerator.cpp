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

	Vertex::VertexPN v[8] = {

		Vertex::VertexPN( -hw, -hh, -hd, -1.0f, -1.0f, -1.0f ),
		Vertex::VertexPN( hw, -hh, -hd, 1.0f, -1.0f, -1.0f ),
		Vertex::VertexPN( hw, hh, -hd, 1.0f, 1.0f, -1.0f ),
		Vertex::VertexPN( -hw, hh, -hd, -1.0f, 1.0f, -1.0f ),

		Vertex::VertexPN( -hw, -hh, hd, -1.0f, -1.0f, 1.0f ),
		Vertex::VertexPN( hw, -hh, hd, 1.0f, -1.0f, 1.0f ),
		Vertex::VertexPN( hw, hh, hd, 1.0f, 1.0f, 1.0f ),
		Vertex::VertexPN( -hw, hh, hd, -1.0f, 1.0f, 1.0f )

	};
	mesh.vertices.assign( v, v + 8);

	DWORD i[36] = 
	{
		0, 1, 2,
		0, 2, 3,
		1, 5, 6,
		1, 6, 2,
		5, 4, 7,
		5, 7, 6,
		4, 0, 3,
		4, 3, 7,
		3, 2, 6,
		3, 6, 7,
		4, 5, 1,
		4, 1, 0
	};

	mesh.indices.assign( i , i + 36);
}