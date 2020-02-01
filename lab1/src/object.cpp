#include "object.h"
#include <GL/glut.h>


object::object()
{
}



object::~object()
{
	delete this->vertices;
	delete this->triangles;
	delete this->surfaceNormals;
	delete this->vertexNormals;
}

void object::setVertices(std::vector<glm::vec4> *verticesVec)
{
	this->vertices = new glm::vec4[(*verticesVec).size()];
	this->numOfVertices = (*verticesVec).size();

	for (int i = 0; i < numOfVertices; ++i) {
		this->vertices[i] = (*verticesVec)[i];
	}
}

void object::setTriangles(std::vector<struct triangle>* trianglesVec)
{
	this->triangles = new triangle[(*trianglesVec).size()];
	this->numOfTriangles = (*trianglesVec).size();

	for (int i = 0; i < numOfTriangles; ++i) {
		this->triangles[i] = (*trianglesVec)[i];
	}
}

void object::calculateSurfaceNormals()
{
	this->surfaceNormals = new glm::vec3[this->numOfTriangles];
	for (int i = 0; i < this->numOfTriangles; ++i) {
		struct triangle tri = this->triangles[i];
		glm::vec4 v1, v2, v3;
		v1 = this->vertices[tri.p1];
		v2 = this->vertices[tri.p2];
		v3 = this->vertices[tri.p3];

		this->surfaceNormals[i] = normalize(glm::cross(glm::vec3(v2 - v1), glm::vec3(v2 - v3)));
	}
}

void object::calculateVertexNormals()
{
	this->vertexNormals = new glm::vec3[this->numOfVertices];
	for (int i = 0; i < this->numOfTriangles; ++i) {
		struct triangle tri = this->triangles[i];
		this->vertexNormals[tri.p1] += this->surfaceNormals[i];
		this->vertexNormals[tri.p2] += this->surfaceNormals[i];
		this->vertexNormals[tri.p3] += this->surfaceNormals[i];
	}
	for (int i = 0; i < this->numOfVertices; ++i) {
		this->vertexNormals[i] = normalize(this->vertexNormals[i]);
	}
}

void object::calculateNormals()
{
	calculateSurfaceNormals();
	calculateVertexNormals();
}

void object::transform(glm::mat4 transMat)
{
	for (int i = 0; i < numOfVertices; ++i) {
		vertices[i] = vertices[i] * transMat;
	}
	center = center * transMat;
	side = side * transMat;
	up = up * transMat;
}

void object::drawCenter()
{
	glPointSize(3.0f);
	glBegin(GL_POINTS);
	glColor3f(1.0f, 0.0f, 0.0f);
	glVertex2i(this->center.x, this->center.y);
	glEnd();

}
