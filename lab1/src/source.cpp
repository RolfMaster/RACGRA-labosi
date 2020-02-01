#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <GL/glut.h>
#include <GL/freeglut.h>
#include <glm/glm.hpp>
#include "lbl.h"
#include <vector>
#include <fstream>
#include <string>
#include "object.h"
#include <filesystem>

int WINDOW_WIDTH = 1280;
int WINDOW_HEIGHT = 720;

void display();
void reshape(int width, int height);
void updatePerspective();
void renderScene();
void mouseFunc(int, int, int, int);
void mouseMoved(int, int);
void keyboardFunc(unsigned char key, int x, int y);
void specialKeyboardFunc(int key, int x, int y);

std::vector<std::string> objFileNames;
std::vector<std::string>::iterator currentOpenFile;

std::vector<object*> objects;
object* activeObject;

object mainCamera;
glm::vec4 cameraTarget;

const int verticesPerSegment = 20;
const int numOfControlPoints = 12;
const int numOfSegments = numOfControlPoints - 3;
glm::vec4 controlPoints[numOfControlPoints];
const int BSplineSize = numOfSegments * verticesPerSegment;
glm::vec4 BSplineVertices[BSplineSize];
glm::vec4 BSplineTangentBase;	
float BSplineTangentPosition = 0.5f;
glm::mat4 BSplineTangentCoordinates;

shadingMode myShadingMode = FLAT;

transformMode myTransformMode = NONE;
glm::vec4 previousCursorLocation = { 0.0f, 0.0f, 0.0f, 0.0f };

char rotationalAxis = 'z';
glm::vec3 lightVector = normalize(glm::vec3(1, 0, 0));

float zoomFactor = 200.0f;
float opticalZoom = 2.0f;

int main(int argc, char **argv) {
	getObjectsFromDir();
	loadObjectFromFile(objFileNames.front());

	controlPoints[0] = glm::vec4(0.0f, 50.0f, 15.0f, 1.0f);
	controlPoints[1] = glm::vec4(0.0f, 10.0f, 5.0f, 1.0f);
	controlPoints[2] = glm::vec4(10.0f, 10.0f, 10.0f, 1.0f);
	controlPoints[3] = glm::vec4(10.0f, 0.0f, 15.0f, 1.0f);
	controlPoints[4] = glm::vec4(0.0f, 0.0f, 20.0f, 1.0f);
	controlPoints[5] = glm::vec4(0.0f, 10.0f, 25.0f, 1.0f);
	controlPoints[6] = glm::vec4(10.0f, 10.0f, 30.0f, 1.0f);
	controlPoints[7] = glm::vec4(10.0f, 0.0f, 35.0f, 1.0f);
	controlPoints[8] = glm::vec4(0.0f, 0.0f, 40.0f, 1.0f);
	controlPoints[9] = glm::vec4(0.0f, 10.0f, 45.0f, 1.0f);
	controlPoints[10] = glm::vec4(10.0f, 10.0f, 50.0f, 1.0f);
	controlPoints[11] = glm::vec4(10.0f, 0.0f, 55.0f, 1.0f);


	transform(scalingMatrix(0.1f, 0.1f, -0.1f), &controlPoints[0], numOfControlPoints);
	BSplinePopulateVertices(&BSplineVertices[0], &controlPoints[0], numOfControlPoints, verticesPerSegment);

	mainCamera.center = glm::vec4(0, 0, 5.0, 1.0);	
	cameraTarget = glm::vec4(0, 0, 0, 0);

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
	glutInitWindowPosition(100, 0);
	glutCreateWindow("sick gfx");
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutMouseFunc(mouseFunc);
	glutPassiveMotionFunc(mouseMoved);
	glutKeyboardFunc(keyboardFunc);
	glutSpecialFunc(specialKeyboardFunc);
	glutMainLoop();
	return 0;
}

void display() {
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	renderScene();
	glutSwapBuffers();
}

void reshape(int width, int height) {
	WINDOW_WIDTH = width; WINDOW_HEIGHT = height;
	glViewport(0, 0, width, height);
	updatePerspective();
}

void updatePerspective() {
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0, (float)WINDOW_WIDTH/WINDOW_HEIGHT, 0.5, 20.0);
	gluLookAt(mainCamera.center.x, mainCamera.center.y, mainCamera.center.z,
		0, 0, 0,
		0.0, 1.0, 0.0);
	glMatrixMode(GL_MODELVIEW);
}

void renderScene() {
	glPointSize(1.0f);
	glColor3f(0.0f, 1.0f, 0.0f);
	
	std::vector<glm::vec3> vertexNormals;


	int currentSegment = (int)(BSplineTangentPosition*numOfSegments);
	int currentPosition = BSplineTangentPosition * (verticesPerSegment * numOfSegments);
	float currentT = (currentPosition - (currentSegment*verticesPerSegment)) / (float)verticesPerSegment;
	BSplineTangentBase = BSplineVertices[currentPosition];
	glm::vec4 tangentTip = BSplineTangentBase + BSplineDoubleTangent(&controlPoints[currentSegment], currentT);

	BSplineTangentCoordinates = BSplineCoordinates(&controlPoints[currentSegment], currentT);
	
	
	for (int i = 0; i < activeObject->numOfTriangles; ++i) {
		
		triangle tri = activeObject->triangles[i];
		glm::vec4 v1 = activeObject->vertices[tri.p1];
		glm::vec4 v2 = activeObject->vertices[tri.p2];
		glm::vec4 v3 = activeObject->vertices[tri.p3];

		glm::mat4 translMat = translationMatrix(BSplineTangentBase.x, BSplineTangentBase.y, BSplineTangentBase.z);
		glm::mat4 transfMat = BSplineTangentCoordinates * translMat;
		v1 = v1 * transfMat;
		v2 = v2 * transfMat;
		v3 = v3 * transfMat;

		glm::vec3 normal = activeObject->surfaceNormals[i];
		
		glm::vec3 ray = glm::vec3(mainCamera.center) - glm::vec3(v1);
		if (dot(ray, normal) < 0)
			continue;

		switch (myShadingMode) {
		case(FLAT):
		{
			float normality = glm::dot(lightVector, activeObject->surfaceNormals[i]);
			glColor3f(normality, 0, 0.2);
			glBegin(GL_TRIANGLES);
			glVertex3f(v1.x, v1.y, v1.z);
			glVertex3f(v2.x, v2.y, v2.z);
			glVertex3f(v3.x, v3.y, v3.z);
			glEnd();
			break;
		}
		case(GOURAUD):
			float normality1 = glm::dot(lightVector, activeObject->vertexNormals[tri.p1]);
			float normality2 = glm::dot(lightVector, activeObject->vertexNormals[tri.p2]);
			float normality3 = glm::dot(lightVector, activeObject->vertexNormals[tri.p3]);

			glBegin(GL_TRIANGLES);
			glColor3f(normality1, 0, 0.2); glVertex3f(v1.x, v1.y, v1.z);
			glColor3f(normality2, 0, 0.2); glVertex3f(v2.x, v2.y, v2.z);
			glColor3f(normality3, 0, 0.2); glVertex3f(v3.x, v3.y, v3.z);
			glEnd();
			break;
		}
	}
	
	//drawing BSpline
	glColor3f(1.0f, 1.0f, 1.0f);
	glBegin(GL_LINE_STRIP);
	for (int i = 0; i < BSplineSize; ++i) {
		glVertex3f(BSplineVertices[i].x, BSplineVertices[i].y, BSplineVertices[i].z);
	}
	glEnd();


	glColor3f(0.0f, 1.0f, 1.0f);
	glBegin(GL_LINE_STRIP);
	glVertex3f(BSplineTangentBase.x, BSplineTangentBase.y, BSplineTangentBase.z);
	glVertex3f(tangentTip.x, tangentTip.y, tangentTip.z);
	glEnd();


	//activeObject->drawCenter();
	/*
	glPointSize(5.0f);
	glColor3f(0, 0, 1);
	glBegin(GL_POINTS);
	glVertex3f(cameraTarget.x, cameraTarget.y, cameraTarget.z);
	glEnd();
	*/
}

void mouseFunc(int button, int state, int x, int y) {
}

void mouseMoved(int x, int y) {

	glm::vec4* vertices = activeObject->vertices;

	switch (myTransformMode) {
		case ROTATION:
		{
			glm::vec4 pivot = activeObject->center;
			//float angle = atan2f((float)x - vertices[0].x, (float)y - vertices[0].y);
			//float prevAngle = atan2f((float)previousCursorLocation.x - pivot.x, (float)previousCursorLocation.y - pivot.y);
			glm::mat4 transMat = translationMatrix(-pivot.x, -pivot.y, -pivot.z);
			glm::mat4 rotMat = rotationMatrix(previousCursorLocation.x - x, rotationalAxis);
			//glm::vec4 newPos = glm::vec4((float)x, (float)y, 0.0f, 1.0f);
			transform(transMat*rotMat*inverse(transMat), vertices, activeObject->numOfVertices);
			break;
		}
		case TRANSLATION:
		{
			glm::vec4 pivot = activeObject->center;
			glm::mat4 transMat = translationMatrix((x - previousCursorLocation.x) / zoomFactor, (y - previousCursorLocation.y) / zoomFactor, 0.0f);
			transform(transMat, vertices, activeObject->numOfVertices);
			activeObject->center = (activeObject->center)*transMat;
			break;
		}
		case SCALING:
		{
			glm::vec4 newPos = glm::vec4((float)x, (float)y, 0.0f, 1.0f);
			glm::vec4 pivot = activeObject->center;
			float scaleFactor = length(glm::vec2(newPos) - glm::vec2(pivot)) / length(glm::vec2(previousCursorLocation) - glm::vec2(pivot));
			glm::mat4 scaleMat = scalingMatrix(scaleFactor, scaleFactor, scaleFactor);
			glm::mat4 transMat = translationMatrix(-pivot.x, -pivot.y, 0.0f);
			transform(transMat*scaleMat*inverse(transMat), vertices, activeObject->numOfVertices);
			transform(transMat*scaleMat*inverse(transMat), &activeObject->center, 1);
			break;
		}
		case BSPLINE:
		{
			BSplineTangentPosition += (x - previousCursorLocation.x)/600.0f;
			if (BSplineTangentPosition >= 1.0f) BSplineTangentPosition = 0.999f;
			else if (BSplineTangentPosition < 0.0f) BSplineTangentPosition = 0.0f;
			break;
		}
		default:
			break;
	}
	if (myTransformMode != NONE)
		display();

	
	previousCursorLocation  = glm::vec4(x, y, 0.0f, 1.0f);
}

void keyboardFunc(unsigned char key, int x, int y) {
	glm::vec4 bla;
	std::cout << key << std::endl;
	key = tolower(key);
	switch(key) {
		case('w'):
			mainCamera.center.z -= 0.1f;
			cameraTarget.z -= 0.1f;
			break;
		case('s'):
			mainCamera.center.z += 0.1f;
			cameraTarget.z += 0.1f;
			break;
		case('d'):
			mainCamera.center.x += 0.1f;
			cameraTarget.x += 0.1f;
			break;
		case('a'):
			mainCamera.center.x -= 0.1f;
			cameraTarget.x -= 0.1f;
			break;
		case('e'):
			mainCamera.center.y += 0.1f;
			cameraTarget.y += 0.1f;
			break;
		case('q'):
			mainCamera.center.y -= 0.1f;
			cameraTarget.y -= 0.1f;
			break;
		case('r'): 
			if (myTransformMode == ROTATION) 
				myTransformMode = NONE;
			else 
				myTransformMode = ROTATION;
			break;
		case('t'):
			if (myTransformMode == TRANSLATION)
				myTransformMode = NONE;
			else
				myTransformMode = TRANSLATION;
			break;
		case('c'):
			if (myTransformMode == SCALING)
				myTransformMode = NONE;
			else
				myTransformMode = SCALING;
			break;
		case(27):
			glutLeaveMainLoop();
			break;
		case('x'):
		case('y'):
		case('z'):
			rotationalAxis = key;
			break;
		case('j'):
			bla = glm::vec4(lightVector, 1.0f);
			transform(rotationMatrix(10, 'y'), &bla, 1);
			lightVector = glm::vec3(bla);
			break;
		case('l'):
			bla = glm::vec4(lightVector, 1.0f);
			transform(rotationMatrix(-10, 'y'), &bla, 1);
			lightVector = glm::vec3(bla);
			break;
		case('k'):
			bla = glm::vec4(lightVector, 1.0f);
			transform(rotationMatrix(10, 'x'), &bla, 1);
			lightVector = glm::vec3(bla);
			break;
		case('i'):
			bla = glm::vec4(lightVector, 1.0f);
			transform(rotationMatrix(-10, 'x'), &bla, 1);
			lightVector = glm::vec3(bla);
			break;
		case('p'):
			if (myShadingMode == FLAT) myShadingMode = GOURAUD;
			else myShadingMode = FLAT;
			break;
		case('b'):
			if (myTransformMode == BSPLINE)
				myTransformMode = NONE;
			else
				myTransformMode = BSPLINE;
			break;
	}
	updatePerspective();
	glutPostRedisplay();
}

void loadObjectFromFile(std::string name) {
	delete activeObject;

	std::vector<glm::vec4> verticesVec;
	std::vector<triangle> triangles;

	std::string line;
	std::ifstream ifs("../objekti/" + name + ".obj");
	if (ifs.is_open()) {
		while (getline(ifs, line)) {
			if (line[0] == 'v') {
				char myString[50];
				strcpy(myString, line.c_str());

				char * pch;
				pch = strtok(myString, " ");
				

				glm::vec4 vertex = glm::vec4(glm::vec3(atof(strtok(NULL, " ")), atof(strtok(NULL, " ")), atof(strtok(NULL, " "))), 1.0f);
				verticesVec.push_back(vertex);
			}
			else if (line[0] == 'f') {
				char myString[50];
				strcpy(myString, line.c_str());

				char * pch;
				pch = strtok(myString, " ");

				triangle t = {atoi(strtok(NULL, " ")) - 1, atoi(strtok(NULL, " ")) - 1, atoi(strtok(NULL, " ")) - 1};
				triangles.push_back(t);
			}
		}
		
		ifs.close();
	}		

	object *mesh = new object();
	mesh->setVertices(&verticesVec);
	mesh->setTriangles(&triangles);
	mesh->numOfVertices = verticesVec.size();
	mesh->numOfTriangles = triangles.size();

	mesh->calculateNormals();
	//mesh->center = middle*transMat;
	//transform(transMat, mesh->vertices, mesh->numOfVertices);

	activeObject = mesh;
}

void getObjectsFromDir()
{
	std::string path = "../objekti";
	for (const auto & entry : std::filesystem::directory_iterator(path)) {
		std::string temp = (entry.path().string());
		objFileNames.push_back(temp.substr(11, temp.length()-15));
	}
	currentOpenFile = objFileNames.begin();
}

void specialKeyboardFunc(int key, int x, int y)
{
	switch (key) {
	case(GLUT_KEY_LEFT):
		if (currentOpenFile != objFileNames.begin()) {
			currentOpenFile--;
			//loadObjectFromFile(*currentOpenFile);
		}
		else
			std::cout << "beginning" << std::endl;

		std::cout << *currentOpenFile << std::endl;
		break;
	case(GLUT_KEY_RIGHT):
		currentOpenFile++;
		if (currentOpenFile != objFileNames.end()) {
			//loadObjectFromFile(*currentOpenFile);
		}
		else {
			currentOpenFile--;
			std::cout << "end" << std::endl;
		}
		std::cout << *currentOpenFile << std::endl;
		break;
	case(GLUT_KEY_F9):
		loadObjectFromFile(*currentOpenFile);
		display();
		break;
	}
	
}