#include "lbl.h"
#define PI 3.141592
#include <math.h>

void translate(int x, int y, std::vector<struct point> *vertices) {
	std::vector<struct point>::iterator i;
	for (i = (*vertices).begin(); i != (*vertices).end(); ++i) {
		
	}
}

glm::mat4 translationMatrix(float x, float y, float z)
{
	return glm::mat4( 1.0f, 0.0f, 0.0f, x,
					  0.0f, 1.0f, 0.0f, y,
					  0.0f, 0.0f, 1.0f, z,
					  0.0f, 0.0f, 0.0f, 1.0f);
}

glm::mat4 rotationMatrix(float angle, char axis) {
	angle = angle * PI / 180;
	switch (axis) {
	case 'z':
		return glm::mat4(cos(angle), -sin(angle), 0.0f, 0.0f,
						 sin(angle), cos(angle), 0.0f, 0.0f,
						 0.0f, 0.0f, 1.0f, 0.0f,
						 0.0f, 0.0f, 0.0f, 1.0f);
		break;
	case 'y':
		return glm::mat4(cos(angle), 0.0f, -sin(angle), 0.0f,
						 0.0f, 1.0f, 0.0f, 0.0f,
						 sin(angle), 0.0f, cos(angle), 0.0f,
						 0.0f, 0.0f, 0.0f, 1.0f);
		break;
	case 'x':
		return glm::mat4(1.0f, 0.0f, 0.0f, 0.0f,
						 0.0f, cos(angle), -sin(angle), 0.0f,
						 0.0f, sin(angle), cos(angle), 0.0f,
					 	 0.0f, 0.0f, 0.0f, 1.0f);
		break;
	}
	
}

glm::mat4 scalingMatrix(float scalex, float scaley, float scalez)
{
	return glm::mat4(scalex, 0.0f, 0.0f, 0.0f,
					 0.0f, scaley, 0.0f, 0.0f,
					 0.0f, 0.0f, scalez, 0.0f,
					 0.0f, 0.0f, 0.0f, 1.0f);
}

glm::mat4 projectionMatrix(float projectionH)
{
	return glm::mat4(1.0f, 0.0f, 0.0f, 0.0f,
					 0.0f, 1.0f, 0.0f, 0.0f,
					 0.0f, 0.0f, 1.0f, 0.0f,
					 0.0f, 0.0f, 1.0f/projectionH, 0.0f);
}

void project(glm::vec4 *vertices, int size)
{
	for (int i = 0; i < size; ++i) {
		float scale = 1.0f / vertices[i].z * 4.0f;
		vertices[i] = vertices[i] * scalingMatrix(scale, scale, scale);
	}
}

void transform(glm::mat4 matrix, glm::vec4 *vertices, int size) {
	for (int i = 0; i < size; ++i) {
		vertices[i] = matrix * vertices[i];
	}
	return;
}

void findExtremes(std::vector<glm::vec4>& vertices, float& leftest, float& rightest, float &highest, float &lowest, float& closest, float& furthest)
{
	leftest = rightest = vertices.front().x;
	highest = lowest = vertices.front().y;
	closest = furthest = vertices.front().z;
	for (glm::vec4 vertex : vertices) {
		if (vertex.x < leftest)	leftest = vertex.x;
		else if (vertex.x > rightest) rightest = vertex.x;
		if (vertex.y < highest) highest = vertex.y;
		else if (vertex.y > lowest) lowest = vertex.y;
		if (vertex.z < closest) closest = vertex.z;
		else if (vertex.y > furthest) furthest = vertex.z;
	}
}

glm::vec3 triangleCenter(glm::vec4 * vertices)
{
	glm::vec3 result = glm::vec3(0.0f, 0.0f, 0.0f);
	for (int i = 0; i < 3; ++i) {
		result = result + glm::vec3(vertices[i]);
	}
	result = result / 3.0f;
	return glm::vec3();
}

void Bezier(glm::vec4 * vertices, glm::vec4 * controlPoints, int n)
{
	for (int i = 0; i <= n; ++i) {
		float staticPart = (float)factorial(n) / (factorial(i) * factorial(n - i));
		
		for (int t = 0; t < 100; ++t) {
			float temp = std::powf(t / 100.0f, (float)i) * std::powf(1.0f - (t / 100.0f), (float)n - i);
			vertices[t] = vertices[t] + controlPoints[i] * (temp * staticPart);
		}
	}
}

void BSplinePopulateVertices(glm::vec4 * vertices, glm::vec4 * controlPoints, const int numOfControlPoints, const int numOfVerticesPerSegment)
{
	glm::mat4 BMatrix = (1.0f/6.0f) * glm::mat4(-1.0f, 3.0f, -3.0f, 1.0f,
		3.0f, -6.0f, 0.0f, 4.0f,
		-3.0f, 3.0f, 3.0f, 1.0f,
		1.0f, 0.0f, 0.0f, 0.0f);
	glm::vec4 TVector;
	glm::mat4 RMatrix;
	float t;

	for (int j = 0; j < numOfControlPoints - 3; j++) {
		RMatrix = transpose(glm::mat4(controlPoints[j], controlPoints[j+1], controlPoints[j+2], controlPoints[j+3]));
		for (int i = 0; i < numOfVerticesPerSegment; i++) {
			t = (float)i / (float)numOfVerticesPerSegment;
			TVector = glm::vec4(std::powf(t, 3), std::powf(t, 2), t, 1.0f);

			vertices[j*numOfVerticesPerSegment + i] = TVector * BMatrix * RMatrix;
		}
	}
}

glm::vec4 BSplineTangent(glm::vec4 controlPoints[4], float t)
{
	glm::vec3 TVector = glm::vec3(std::pow(t, 2), t, 1.0f);
	glm::mat4x3 BMatrix = 1.0f/2.0f * glm::mat4x3(-1.0f, 2.0f, -1.0f,
		3.0f, -4.0f, 0.0f,
		-3.0f, 2.0f, 1.0f,
		1.0f, 0.0f, 0.0f);
	glm::mat4 RMatrix = transpose(glm::mat4(controlPoints[0], controlPoints[1], controlPoints[2], controlPoints[3]));
	return TVector * BMatrix * RMatrix;
}

glm::vec4 BSplineDoubleTangent(glm::vec4 controlPoints[4], float t)
{
	glm::vec4 TVector = glm::vec4(6.0f * t, 2.0, 0.0, 0.0f);
	glm::mat4 BMatrix = (1.0f / 6.0f) * glm::mat4(-1.0f, 3.0f, -3.0f, 1.0f,
		3.0f, -6.0f, 0.0f, 4.0f,
		-3.0f, 3.0f, 3.0f, 1.0f,
		1.0f, 0.0f, 0.0f, 0.0f);
	glm::mat4 RMatrix = transpose(glm::mat4(controlPoints[0], controlPoints[1], controlPoints[2], controlPoints[3]));
	
	return TVector * BMatrix * RMatrix;
}

glm::mat4 BSplineCoordinates(glm::vec4 controlPoints[4], float t)
{
	if (t == 0.0f) t = 0.00001f;
	glm::vec4 Xaxis = BSplineTangent(controlPoints, t);
	glm::vec4 Yaxis = glm::vec4(glm::cross(glm::vec3(BSplineTangent(controlPoints,t)),glm::vec3(BSplineDoubleTangent(controlPoints, t))), 1.0f);
	glm::vec4 Zaxis = glm::vec4(glm::cross(glm::vec3(Xaxis), glm::vec3(Yaxis)), 1.0f);
	
	return glm::transpose(glm::mat4(glm::normalize(Xaxis), glm::normalize(Yaxis), glm::normalize(Zaxis), glm::normalize(glm::vec4(0.0f, 0.0f, 0.0f, 1.0f))));
}



float factorial(float num)
{
	float ret = 1;
	for (int i = 1; i <= num; ++i)
		ret *= i;
	return ret;
	return 0.0f;
}

