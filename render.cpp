#include "render.h"
#include "glm.h"

//// Model loading variables
GLMmodel* pmodel = NULL;

const int MAXGRID = 100;

// Wave stuff
float force[MAXGRID][MAXGRID],
veloc[MAXGRID][MAXGRID],
posit[MAXGRID][MAXGRID],
vertNorms[MAXGRID][MAXGRID][3],
faceNorms[2][MAXGRID][MAXGRID][3],
faceNormSegs[2][2][MAXGRID][MAXGRID][3];	
float dt = 0.003;

#define SQRTOFTWOINV 1.0 / 1.414213562

// Default constructor
// used for initializing any user defined variables
Render::Render()
{
	// specify a default location for the camera
	eye[0] = 0.0f;
	eye[1] = 0.0f;
	eye[2] = 0.0f;

	// specify default values to the rotational values in the transformation matrix
	rot[0] = 0.0f;
	rot[1] = 0.0f;
	rot[2] = 0.0f;

	LEFT = 0;
	MIDDLE = 1;
	RIGHT = 2;
	mButton = -1;

}

Render::~Render()
{


}

void Render::init(void)
{
	glClearColor (0.0, 0.0, 0.0, 0.0);
	glShadeModel (GL_SMOOTH);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glClearDepth(1.0);

	glEnable (GL_COLOR_MATERIAL);
	glColorMaterial (GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
 
	// define the parameters for the ambient light
//	glEnable(GL_LIGHT0);
	GLfloat light0_ambient[] = { 0.4, 0.4, 0.4, 1 };
//	GLfloat light0_diffuse[] = { 0.5, 0.5, 0.5, 1 };
//	GLfloat light0_specular[] = { 0.6, 0.6, 0.6, 1 };
	glLightfv(GL_LIGHT0, GL_AMBIENT, light0_ambient);
//	glLightfv(GL_LIGHT0, GL_DIFFUSE, light0_diffuse);
//	glLightfv(GL_LIGHT0, GL_SPECULAR, light0_specular);

	// Create white sun
	glEnable(GL_LIGHT1);
	GLfloat light1_ambient[] = { 0.0, 0.0, 0.0, 1 };
	GLfloat light1_diffuse[] = { 1.0, 1.0, 1.0, 1 };
	GLfloat light1_specular[] = { 0.4, 0.4, 0.4, 1 };
	glLightfv(GL_LIGHT1, GL_AMBIENT, light1_ambient);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, light1_diffuse);
	glLightfv(GL_LIGHT1, GL_SPECULAR, light1_specular);

	// Light used for reflection
	glEnable(GL_LIGHT3);
	GLfloat light3_ambient[] = { 0.3, 0.3, 0.3, 1 };
	GLfloat light3_diffuse[] = { 1.0, 1.0, 1.0, 1 };
	GLfloat light3_specular[] = { 1.0, 1.0, 1.0, 1 };
	GLfloat light3_position[] = { 100.0, 100.0, 0.0, 1 };
	glLightfv(GL_LIGHT3, GL_AMBIENT, light3_ambient);
	glLightfv(GL_LIGHT3, GL_DIFFUSE, light3_diffuse);
	glLightfv(GL_LIGHT3, GL_SPECULAR, light3_specular);
	glLightfv(GL_LIGHT3, GL_POSITION, light3_position);

	// Create the fog effect
	glEnable(GL_FOG);
	GLfloat fogColor[4] = {0.5, 0.5, 0.5, 1.0};
	static GLint fogMode;
	fogMode = GL_LINEAR;
	glFogi(GL_FOG_MODE, fogMode);
	glFogfv(GL_FOG_COLOR, fogColor);
	glFogf(GL_FOG_DENSITY, 0.35);
	glHint(GL_FOG_HINT, GL_DONT_CARE);
	glFogf(GL_FOG_START, 1.0);
	glFogf(GL_FOG_END, 800.0);	

	// Set up the sky texture
	glGenTextures(1, &texture);
	glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );	
	
	// Read an obj file and load it, but not displayed yet
    if (!pmodel) {
        pmodel = glmReadOBJ("f-16.obj");
        if (!pmodel) exit(0);
        glmUnitize(pmodel);
        glmFacetNormals(pmodel);
        glmVertexNormals(pmodel, 90.0);
    }
	
	// Set up waves grid
	numRows = 100;
	numCols = 100;
	
	for (int i=1; i<MAXGRID-1; i++) {
		for (int j=1; j<MAXGRID-1; j++) {
			posit[i][j]= 
			(sin(M_PI*2 * ((float)i/(float)MAXGRID)) +
			 sin(M_PI*2 * ((float)j/(float)MAXGRID)))* MAXGRID/6.0;	
		}
	}
		
	// Start the waves
	go();
}

void Render::reshape(int w, int h)
{
	glViewport (0, 0, (GLsizei) w, (GLsizei) h);
	Wx = w;
	Wy = h;
}

void Render::mouseClick(int button, int state, int x, int y)
{
	if (state == GLUT_DOWN)
	{
		mOldX = x;
		mOldY = y;
		switch (button)
		{
			case GLUT_LEFT_BUTTON:
				mButton = LEFT;
				break;
			case GLUT_MIDDLE_BUTTON:
				mButton = MIDDLE;
				break;
			case GLUT_RIGHT_BUTTON:
				mButton = RIGHT;
				break;
		}
	}
	else if (state == GLUT_UP)
	{
		mButton = -1;
	}
}

void Render::mouseMove(int x, int y)
{
	// Allow rotation about X and Y axis
	if (mButton == LEFT)
	{
		rot[0] -= ((mOldY - y) * 180.0f) / 1000.0f;
		rot[1] -= ((mOldX - x) * 180.0f) / 1000.0f;
		clamp(rot[0], rot[1], rot[2]);
	}
	
	// Allow zooming
	else if (mButton == MIDDLE)
	{
		eye[2] -= ((mOldY - y) * 180.0f) / 100.0f;
		
		if(eye[2] >= 1000)
		{
			eye[2] = 1000;
		}
		
		clamp(rot[0], rot[1], rot[2]);
	}
	
	// Allow translation of the model in X-Z plane
	else if (mButton == RIGHT)
	{
		eye[0] += ((mOldX - x) * 180.0f) / 100.0f;
		eye[1] -= ((mOldY - y) * 180.0f) / 100.0f;
		clamp(rot[0], rot[1], rot[2]);
	}
	
	mOldX = x;
	mOldY = y;
	
	// i changed values in the modelview matrix. so
	// update my display and the objects on the screens
	glutPostRedisplay();
	
}

void Render::keyPos(unsigned char key, int x, int y)
{
	switch (key)
	{
		case 27:
			exit (0);

		default:
			break;
	}
	glutPostRedisplay();
}

#ifdef __APPLE__
#pragma mark --
#pragma mark Flying Stuff

void Render::flyPlane(float x, float y, float z)
{
	xMot += x;
	yMot += y;
	zMot += z;
}

void Render::drawObj(void)
{
	glDisable(GL_COLOR_MATERIAL);
	
	glPushMatrix();
		glTranslatef(0.0, 0.0, -400);
		glRotatef(xMot, 0.0, -1.0, 0.0);
		glTranslatef(200.0, 0.0, 0.0);
		glRotatef(40, 0.0, 0.0, 1.0);
		glmDraw(pmodel, GLM_SMOOTH | GLM_MATERIAL);
	glPopMatrix();
	
	glEnable(GL_COLOR_MATERIAL);
}

#pragma mark --
#endif

// Calculates a normal vector, given three points
vec3_t Render::calcNormal(vec3_t p1, vec3_t p2, vec3_t p3)
{
	vec3_t myNorm, vec1, vec2;
	
	// Defines vectors created by the points
	vec1 = p1 - p2;
	vec2 = p3 - p2;
	
	// Find cross product to find normal at p2
	myNorm.Cross(vec1, vec2);
	myNorm.Normalize();
	
	return myNorm;
}

void Render::drawWater(void)
{	
    glColor3f(0.0f, 0.0f, 1.0f);
    for (int i = 0; i < MAXGRID - 1; ++i)
    {
        glBegin(GL_TRIANGLE_STRIP);
        for (int j = 0; j < MAXGRID; ++j)
        {
			glColor3f(0.0, 0.0, 0.2);
			glNormal3fv( vertNorms[i][j] );
            glVertex3f( j*20, posit[i][j], i*20);
			glNormal3fv( vertNorms[i+1][j] );
            glVertex3f( j*20, posit[i+1][j], (i+1)*20);
        }
        glEnd();
    }
}

void Render::getforce(void)
{
    float d;
	
    for(int i=0;i<MAXGRID;i++) 
        for(int j=0;j<MAXGRID;j++) 
        {
            force[i][j]=0.0;
        }
	
    for(int i=2;i<MAXGRID-2;i++)
        for(int j=2;j<MAXGRID-2;j++) 
        {
            d=posit[i][j]-posit[i][j-1];
            force[i][j] -= d;
            force[i][j-1] += d;
			
            d=posit[i][j]-posit[i-1][j];
            force[i][j] -= d;
            force[i-1][j] += d;
			
            d= (posit[i][j]-posit[i][j+1]); 
            force[i][j] -= d ;
            force[i][j+1] += d;
			
            d= (posit[i][j]-posit[i+1][j]); 
            force[i][j] -= d ;
            force[i+1][j] += d;
			
            d= (posit[i][j]-posit[i+1][j+1])*SQRTOFTWOINV; 
            force[i][j] -= d ;
            force[i+1][j+1] += d;
			
            d= (posit[i][j]-posit[i-1][j-1])*SQRTOFTWOINV; 
            force[i][j] -= d ;
            force[i-1][j-1] += d;
			
            d= (posit[i][j]-posit[i+1][j-1])*SQRTOFTWOINV; 
            force[i][j] -= d ;
            force[i+1][j-1] += d;
			
            d= (posit[i][j]-posit[i-1][j+1])*SQRTOFTWOINV; 
            force[i][j] -= d ;
            force[i- 1][j+1] += d;
        }
}

void Render::getvelocity(void)
{
    for(int i=0;i<MAXGRID;i++)
        for(int j=0;j<MAXGRID;j++)
            veloc[i][j]+=force[i][j] * dt;
}

void Render::getposition(void)
{
    for(int i=0;i<MAXGRID;i++)
        for(int j=0;j<MAXGRID;j++)
            posit[i][j]+=veloc[i][j];
}


void Render::copy(float vec0[3], float vec1[3])
{
    vec0[0] = vec1[0];
    vec0[1] = vec1[1];
    vec0[2] = vec1[2];
}

void Render::sub(float vec0[3], float vec1[3], float vec2[3])
{
    vec0[0] = vec1[0] - vec2[0];
    vec0[1] = vec1[1] - vec2[1];
    vec0[2] = vec1[2] - vec2[2];
}

void Render::add(float vec0[3], float vec1[3], float vec2[3])
{
    vec0[0] = vec1[0] + vec2[0];
    vec0[1] = vec1[1] + vec2[1];
    vec0[2] = vec1[2] + vec2[2];
}

void Render::scalDiv(float vec[3], float c)
{
    vec[0] /= c; vec[1] /= c; vec[2] /= c;
}

void Render::cross(float vec0[3], float vec1[3], float vec2[3])
{
    vec0[0] = vec1[1] * vec2[2] - vec1[2] * vec2[1];
    vec0[1] = vec1[2] * vec2[0] - vec1[0] * vec2[2];
    vec0[2] = vec1[0] * vec2[1] - vec1[1] * vec2[0];
}

void Render::norm(float vec[3])
{
    float c = sqrt(vec[0] * vec[0] + vec[1] * vec[1] + vec[2] * vec[2]);
    scalDiv(vec, c); 
}

void Render::set(float vec[3], float x, float y, float z)
{
    vec[0] = x;
    vec[1] = y;
    vec[2] = z;
}


/* face normals - for flat shading */
void Render::getFaceNorms(void)
{
    float vec0[3], vec1[3], vec2[3], norm0[3], norm1[3];
    float geom0[3], geom1[3], geom2[3], geom3[3];
    for (int i = 0; i < MAXGRID-1; ++i)
    {
        for (int j = 0; j < MAXGRID-1; ++j)
        {
            /* get vectors from geometry points */
            geom0[0] = i; geom0[1] = j; geom0[2] = posit[i][j];
            geom1[0] = i; geom1[1] = j+1; geom1[2] = posit[i][j+1];
            geom2[0] = i+1; geom2[1] = j; geom2[2] = posit[i+1][j];
            geom3[0] = i+1; geom3[1] = j+1; geom3[2] = posit[i+1][j+1];
			
            sub( vec0, geom1, geom0 );
            sub( vec1, geom1, geom2 );
            sub( vec2, geom1, geom3 );
			
            /* get triangle face normals from vectors & normalize them */
            cross( norm0, vec0, vec1 );
            norm( norm0 );
			
            cross( norm1, vec1, vec2 ); 
            norm( norm1 );
			
            copy( faceNorms[0][i][j], norm0 );
            copy( faceNorms[1][i][j], norm1 );
        }
    }
}

/* vertex normals - average of face normals for smooth shading */
void Render::getVertNorms(void)
{
    float avg[3];
    for (int i = 0; i < MAXGRID; ++i)
    {
        for (int j = 0; j < MAXGRID; ++j)
        {
            /* For each vertex, average normals from all faces sharing */
            /* vertex.  Check each quadrant in turn */
            set(avg, 0.0, 0.0, 0.0);
			
            /* Right & above */
            if (j < MAXGRID-1 && i < MAXGRID-1)
            {
                add( avg, avg, faceNorms[0][i][j] );
            }
            /* Right & below */
            if (j < MAXGRID-1 && i > 0)
            {
                add( avg, avg, faceNorms[0][i-1][j] );
                add( avg, avg, faceNorms[1][i-1][j] );
            }
            /* Left & above */
            if (j > 0 && i < MAXGRID-1)
            {
                add( avg, avg, faceNorms[0][i][j-1] );
                add( avg, avg, faceNorms[1][i][j-1] );
            }
            /* Left & below */
            if (j > 0 && i > 0)
            {
                add( avg, avg, faceNorms[1][i-1][j-1] );
            }
			
            /* Normalize */
            norm( avg );
            copy( vertNorms[i][j], avg );
        }
    }
}

void Render::getFaceNormSegs(void)
{
    float center0[3], center1[3], normSeg0[3], normSeg1[3];
    float geom0[3], geom1[3], geom2[3], geom3[3];
    for (int i = 0; i < MAXGRID - 1; ++i)
    {
        for (int j = 0; j < MAXGRID - 1; ++j)
        {
            geom0[0] = i; geom0[1] = j; geom0[2] = posit[i][j];
            geom1[0] = i; geom1[1] = j+1; geom1[2] = posit[i][j+1];
            geom2[0] = i+1; geom2[1] = j; geom2[2] = posit[i+1][j];
            geom3[0] = i+1; geom3[1] = j+1; geom3[2] = posit[i+1][j+1];
			
            /* find center of triangle face by averaging three vertices */
            add( center0, geom2, geom0 );
            add( center0, center0, geom1 );
            scalDiv( center0, 3.0 );
			
            add( center1, geom2, geom1 );
            add( center1, center1, geom3 );
            scalDiv( center1, 3.0 );
			
            /* translate normal to center of triangle face to get normal segment */
            add( normSeg0, center0, faceNorms[0][i][j] );
            add( normSeg1, center1, faceNorms[1][i][j] );
			
            copy( faceNormSegs[0][0][i][j], center0 );
            copy( faceNormSegs[1][0][i][j], center1 );
			
            copy( faceNormSegs[0][1][i][j], normSeg0 );
            copy( faceNormSegs[1][1][i][j], normSeg1 );
        }
    }
}

void Render::drawFaceNormals(void)
{
    glColor3f(1.0,1.0,1.0);
    for (int i = 0; i < MAXGRID - 1; ++i)
    {
        for (int j = 0; j < MAXGRID - 1; ++j)
        {
            glBegin(GL_LINES);
            glVertex3fv(faceNormSegs[0][0][i][j]);
            glVertex3fv(faceNormSegs[0][1][i][j]);
            glEnd();
			
            glBegin(GL_LINES);
            glVertex3fv(faceNormSegs[1][0][i][j]);
            glVertex3fv(faceNormSegs[1][1][i][j]);
            glEnd();
        }
    }
}

void Render::wave(void)
{
    if (waving)
    {
        getforce();
        getvelocity();
        getposition();
        glutPostRedisplay();
    }
}

void Render::go(void)
{
    waving = true;
    editing = false;
}

void Render::display(void)
{
	// must clear color and depth buffer for lighting purposes
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Specifies which matrix stack is the target for subsequent matrix operations
	// In this example, the projection matrix is set to perspective projection matrix stack
	glMatrixMode(GL_PROJECTION);
	// all matrix values from previous frames set to identity
	glLoadIdentity();
	// perspective projection loaded with new values for Wx and Wy updated
	gluPerspective(60, (GLfloat) Wx/(GLfloat) Wy, 1, 1000000);


	// Applies subsequent matrix operations to the modelview matrix stack.
	glMatrixMode(GL_MODELVIEW);

	// Clears all the previously loaded values in the modelview matrix
	glLoadIdentity();
	
	glPushMatrix();
		glTranslatef(-eye[0], -eye[1], -eye[2]);
		glRotatef(-rot[0], 1.0f, 0.0f, 0.0f);
		glRotatef(-rot[1], 0.0f, 1.0f, 0.0f);
		glRotatef(rot[2], 0.0f, 0.0f, 1.0f);
	
		// Create the world
		glBindTexture(GL_TEXTURE_2D, texture);
		glColor3f(0.4, 0.8, 1.0);
		glutSolidSphere(700, 20, 20);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		// Draw the water
		getFaceNorms();
		getVertNorms();
		glPushMatrix();
			glTranslatef(-1000.0, -60.0, -1000.0);
			drawWater();
		glPopMatrix();
	
		drawObj();
		
		glPushMatrix();
			// Rotate the white sun
			glRotatef(xMot/2.0, 1.0, 0.0, 0.0);
			glTranslatef(0.0, 0.0, -800.0);
			GLfloat	light1_position[] = { 0.0, 0.0, 0.0, 1.0 };
			glLightfv(GL_LIGHT1, GL_POSITION, light1_position);
		glPopMatrix();
	glPopMatrix();
	
	glutSwapBuffers();
}

// this is for clamping the numbers between 0 & 360. used for rotation values in the mouse move function
void Render::clamp(float ang)
{
	if(ang > 360.0f)
		ang -= 360.0f;

}
// this is an overloaded function for clamp - clamp if there are three numbers
void Render::clamp(float v0, float v1, float v2)
{
	if (v0 > 360 || v0 < -360)
		v0 = 0;
	if (v1 > 360 || v1 < -360)
		v1 = 0;
	if (v2 > 360 || v2 < -360)
		v2 = 0;
}

