#pragma once
#include <glm/glm.hpp>
#include <vector>
#include "lbl.h"

class object
{
public:
	object();
	~object();
	
	int numOfVertices = 0;
	int numOfTriangles = 0;	

	glm::vec4 center = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
	glm::vec4 side = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
	glm::vec4 up = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);

	void drawCenter();

	glm::vec4 *vertices = nullptr;
	struct triangle *triangles = nullptr;
	glm::vec3 *surfaceNormals = nullptr;
	glm::vec3 *vertexNormals = nullptr;

	void setVertices(std::vector<glm::vec4> *vertices);
	void setTriangles(std::vector<struct triangle> *triangles);
	void calculateNormals();
	
	void transform(glm::mat4 transMat);

private:
	void calculateSurfaceNormals();
	void calculateVertexNormals();
};