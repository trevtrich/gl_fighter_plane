#ifndef _RENDER_H
#define _RENDER_H

#include <stdlib.h>
#include <math.h>
#include <iostream>
#include <GLUT/GLUT.h>

#include "vector.h"

class Render
{
public:
	
	//constructor definition
	Render();
	//destructor definition
	~Render();

	// These functions are defined public so they can be called by the main function in main.cpp
	void display(void);
	void init(void);
	void reshape(int w, int h);
	void mouseClick(int button, int state, int x, int y);
	void mouseMove(int x, int y);
	void keyPos(unsigned char key, int x, int y);

	// Create the ability to fly the plane
	void flyPlane(float x, float y, float z);
	
	// Allow the calculation of normal
	vec3_t calcNormal(vec3_t p1, vec3_t p2, vec3_t p3);
	
	// Make some waves
//	void calcWaves(int num);
//	void drawWaves(float wh);
	void drawWater(void);
	void getFaceNorms(void);
	void getVertNorms(void);
	void getFaceNormSegs(void);
	void drawFaceNormals(void);
	void getforce(void);
	void getvelocity(void);
	void getposition(void);
	void copy(float*, float*);
	void sub(float*, float*, float*);
	void add(float*, float*, float*);
	void scalDiv(float*, float);
	void cross(float*, float*, float*);
	void norm(float*);
	void set(float*, float, float, float);
	void go(void);
	void wave(void);
	

private:
	// These functions are private and cannot be called anywhere outside of render class
	void draw(void);
	void drawObj(void);
	void clamp(float v0, float v1, float v2);
	void clamp(float ang);

	// Variables used in the member functions of render class
	float eye[3];
	float rot[3];
	int Wx, Wy;
	int LEFT, MIDDLE, RIGHT;
	int mButton;
	int mOldX, mOldY;

	// Keep track of the plane's speed
	float xMot, yMot, zMot;
	
	// Save our sky texture
	GLuint texture;

	// Keep track of the height of waves
	int numRows, numCols;
	float height[10000];
	
	bool waving, editing;
};

#endif
