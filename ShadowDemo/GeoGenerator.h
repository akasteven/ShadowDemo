#pragma once

#include "Vertex.h"
#include "D3DUtil.h"
#include <vector>


class GeoGenerator
{
public:
	GeoGenerator();
	~GeoGenerator();

	struct Mesh
	{
		std::vector<Vertex::VertexPN> vertices;
		std::vector<DWORD> indices;
	};

	static void GenCuboid( float w, float h, float d, Mesh & mesh);

};
