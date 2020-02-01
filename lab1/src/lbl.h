#pragma once
#include <vector>
#include <glm/glm.hpp>

struct triangle {
	int p1, p2, p3;
};

struct point {
	int x, y;
};

enum transformMode {
	ROTATION,
	TRANSLATION,
	SCALING,
	BEZIER,
	BSPLINE,
	NONE
};

enum shadingMode {
	FLAT,
	GOURAUD
};

glm::mat4 translationMatrix(float x, float y, float z);
glm::mat4 rotationMatrix(float angle, char axis);
glm::mat4 scalingMatrix(float scalex, float scaley, float scalez);
glm::mat4 projectionMatrix(float projectionH);

void project(glm::vec4*, int size);
void transform(glm::mat4, glm::vec4*, int size);

void findExtremes(std::vector<glm::vec4> &vertices, float& leftest, float& rightest, float& highest, float& lowest, float& closest, float& furthest);

glm::vec3 triangleCenter(glm::vec4* vertices);

void Bezier(glm::vec4 *vertices, glm::vec4 *controlPoints, int numOfPoints);

void BSplinePopulateVertices(glm::vec4 * vertices, glm::vec4 * controlPoints, const int numOfControlPoints, const int numOfVerticesPerSegment);

glm::vec4 BSplineTangent(glm::vec4 controlPoints[4], float t);
glm::vec4 BSplineDoubleTangent(glm::vec4 controlPoints[4], float t);
glm::mat4 BSplineCoordinates(glm::vec4 controlPoints[4], float t);

float factorial(float num);

void loadObjectFromFile(std::string name);
void getObjectsFromDir();