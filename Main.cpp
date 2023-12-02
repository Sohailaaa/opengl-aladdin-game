//=======================================================================
// Includes
//=======================================================================

#include "TextureBuilder.h"
#include "Model_3DS.h"
#include "GLTexture.h"
#include <glut.h>
#include <iostream>
#include <math.h>
#include <vector>
#include <chrono>
#include <random>



//=======================================================================
// Constants
//=======================================================================
float q = 0;
#define DEG2RAD(a) (a * 0.0174532925)
const float gravity = -0.3;
const float initJumpVel = 5;

int WIDTH = 1280;
int HEIGHT = 720;

char title[] = "Aladdin Game";

// 3D Projection Options
GLdouble fovy = 45.0;
GLdouble aspectRatio = (GLdouble)WIDTH / (GLdouble)HEIGHT;
GLdouble zNear = 0.1;
GLdouble zFar = 100000;

int MAX_NUMBER_OF_ENEMIES = 4;
float MIN_ENEMY_CLOSENESS = 5; // No Two Enemies will be closer than this value;



//=======================================================================
// Classes
//=======================================================================

class Vector {
public:
	float x, y, z;

	Vector(float _x = 0.0f, float _y = 0.0f, float _z = 0.0f) {
		x = _x;
		y = _y;
		z = _z;
	}

	Vector operator+(Vector& v) {
		return Vector(x + v.x, y + v.y, z + v.z);
	}

	Vector operator-(Vector& v) {
		return Vector(x - v.x, y - v.y, z - v.z);
	}
	Vector operator*(float n) {
		return Vector(x * n, y * n, z * n);
	}

	Vector operator/(float n) {
		return Vector(x / n, y / n, z / n);
	}

	Vector unit() {
		return *this / sqrt(x * x + y * y + z * z);
	}

	Vector cross(Vector v) {
		return Vector(y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x);
	}
};
class Direction {
public:
	float x,  z;

	Direction() {
		
	}

};

class GameObject {


public:
	Vector position;
	float rotation;
	float scale;
	float collisionRadius;
	Model_3DS gameObjectModel;
	bool displayed;
	bool needsRotation;
	Direction direction;
	bool isCave = false;
	GameObject() {
	}


	GameObject(Vector position, float rotation, float scale, float collisionRadius, char* pathToModel, bool needsRotation
	) {
		this->position = position;
		this->rotation = rotation;
		this->scale = scale;
		this->displayed = true;
		this->needsRotation = needsRotation;

		this->collisionRadius = collisionRadius;
		gameObjectModel.Load(pathToModel);
	}

	void setPosition(Vector position) {
		this->position = position;
	}

	void setRotation(float rotation) {
		this->rotation = rotation;
	}
	void setDisapear() {
		this->displayed = false;
	}


	void setScale(float scale) {
		this->scale = scale;
	}

	void setCollisionRadius(float collisionRadius) {
		this->collisionRadius = collisionRadius;
	}

	void draw() {
		glPushMatrix();
		glTranslatef(position.x, position.y, position.z);
		glRotatef(rotation, 0, 1, 0);
		glScalef(scale, scale, scale);
		if (needsRotation) {
			glPushMatrix();
			glRotatef(90, 1, 0, 0);

		}


		if (isCave) {
			glPushMatrix();
			glTranslatef(1375, 1455, 0);
			glRotatef(135, 0, 0, 1);


		}
		gameObjectModel.Draw();
		if (needsRotation) {
			glPopMatrix();
		}
		glPopMatrix();
	}

};

class Camera {
public:
	Vector eye, center, up;


	Camera(float eyeX = 3.6f, float eyeY = 0.0f, float eyeZ = 2.0f, float centerX = 0.0f, float centerY = 0.0f, float centerZ = 2.0f, float upX = 0.0f, float upY = 1.0f, float upZ = 0.0f) {
		eye = Vector(eyeX, eyeY, eyeZ);
		center = Vector(centerX, centerY, centerZ);
		up = Vector(upX, upY, upZ);
	}

	void moveX(float d) {
		Vector right = up.cross(center - eye).unit();
		eye = eye + right * d;
		center = center + right * d;
	}

	void moveY(float d) {
		eye = eye + up.unit() * d;
		center = center + up.unit() * d;
	}

	void moveZ(float d) {
		Vector view = (center - eye).unit();
		eye = eye + view * d;
		center = center + view * d;
	}

	void rotateX(float a) {
		Vector view = (center - eye).unit();
		Vector right = up.cross(view).unit();
		view = view * cos(DEG2RAD(a)) + up * sin(DEG2RAD(a));
		up = view.cross(right);
		center = eye + view;
	}

	void rotateY(float a) {
		Vector view = (center - eye).unit();
		Vector right = up.cross(view).unit();
		view = view * cos(DEG2RAD(a)) + right * sin(DEG2RAD(a));
		right = view.cross(up);
		center = eye + view;
	}

	void look() {
		gluLookAt(
			eye.x, eye.y, eye.z,
			center.x, center.y, center.z,
			up.x, up.y, up.z
		);
	}
};

int score = 0;

void drawScore() {
	glPushMatrix();


	// Position the score text at the top-left corner of the window
	glRasterPos3f(0.0, 0.92, 0.0);

	char scoreText[50];
	snprintf(scoreText, sizeof(scoreText), "Score: %d", score);

	for (int i = 0; scoreText[i] != '\0'; i++) {
		glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, scoreText[i]);
	}
	glPopMatrix();
}
//=======================================================================
// Variables
//=======================================================================

GLuint tex;
int cameraZoom = 0;

float cameraDistanceFromPlayer = 17.0f;
float cameraHeightAbovePlayer = 10.0f;

// Game Objects
GameObject aladdin;
GameObject cave;

GameObject snake;
GameObject bottle;

// Textures
GLTexture tex_ground;

// State
bool firstPersonModeOn = false;
int movementState = 0;

// Obstacles

std::vector<GameObject> enemySnakes;
std::vector<GameObject> water;


std::vector<GameObject> rocks;

float playerVerticalVelocity = 0.0;
bool endOne = false;
double speX = 0;
double speZ = 0;
bool flagFront = true;
bool flagBack = false;
bool flagRight = false;
bool flagLeft = false;

Camera camera;

//=======================================================================
// Misc. Functions
//=======================================================================
void jump() {
	playerVerticalVelocity = initJumpVel;
}
double compareDistances(Vector a, Vector b) {
	return sqrtf(
		std::fabs(a.x - b.x) * std::fabs(a.x - b.x) +
		std::fabs(a.y - b.y) * std::fabs(a.y - b.y) +
		std::fabs(a.z - b.z) * std::fabs(a.z - b.z));
}

int getRandomInt(int min, int max) {
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<int> distribution(min, max);
	return distribution(gen);
}

//=======================================================================
// Set Up Camera Function
//=======================================================================

void setupCamera() {
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(fovy, aspectRatio, zNear, zFar);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	camera.look();
}

//=======================================================================
// Lighting Configuration Function
//=======================================================================
void InitLightSource()
{
	// Enable Lighting for this OpenGL Program
	glEnable(GL_LIGHTING);

	// Enable Light Source number 0
	// OpengL has 8 light sources
	glEnable(GL_LIGHT0);

	// Define Light source 0 ambient light
	GLfloat ambient[] = { 0.1f, 0.1f, 0.1, 1.0f };
	glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);

	// Define Light source 0 diffuse light
	GLfloat diffuse[] = { 0.5f, 0.5f, 0.5f, 1.0f };
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);

	// Define Light source 0 Specular light
	GLfloat specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glLightfv(GL_LIGHT0, GL_SPECULAR, specular);

	// Finally, define light source 0 position in World Space
	GLfloat light_position[] = { 0.0f, 10.0f, 0.0f, 1.0f };
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);
}

//=======================================================================
// Material Configuration Function
//======================================================================
void InitMaterial()
{
	// Enable Material Tracking
	glEnable(GL_COLOR_MATERIAL);

	// Sich will be assigneet Material Properties whd by glColor
	glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

	// Set Material's Specular Color
	// Will be applied to all objects
	GLfloat specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glMaterialfv(GL_FRONT, GL_SPECULAR, specular);

	// Set Material's Shine value (0->128)
	GLfloat shininess[] = { 96.0f };
	glMaterialfv(GL_FRONT, GL_SHININESS, shininess);
}

//=======================================================================
// OpengGL Configuration Function
//=======================================================================
void myInit(void)
{
	glClearColor(0.0, 0.0, 0.0, 0.0);

	srand(time(0));

	setupCamera();
	//*******************************************************************************************//
	// EYE (ex, ey, ez): defines the location of the camera.									 //
	// AT (ax, ay, az):	 denotes the direction where the camera is aiming at.					 //
	// UP (ux, uy, uz):  denotes the upward orientation of the camera.							 //
	//*******************************************************************************************//

	InitLightSource();
	InitMaterial();

	// {PositionX, PositionY, PositionZ 
	// 
	// 
	// }, Rotation, Scale, Collision Radius, "Path to model file"
	aladdin = GameObject({ 0,0,0 }, 0, 0.04, 0.5, "models/aladdin/aladdin.3ds", true);
	cave = GameObject({ 20,0,20 }, 0, 0.02, 0.5, "models/cave/cave.3ds", true);
	cave.isCave = true;
	//snake = GameObject({ 7,0,0.9 }, 0, 0.03, 0.5, "models/snake/snake.3ds");
	//bottle = GameObject({ -7,0,0.9 }, 0, 0.08, 0.5, "models/bottle/bottle.3ds");

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_NORMALIZE);
}

//=======================================================================
// Render Ground Function
//=======================================================================
void RenderGround()
{
	glDisable(GL_LIGHTING);	// Disable lighting 

	glColor3f(0.2, 0.2, 0.2);	// Dim the ground texture a bit

	glEnable(GL_TEXTURE_2D);	// Enable 2D texturing

	glBindTexture(GL_TEXTURE_2D, tex_ground.texture[0]);	// Bind the ground texture

	glPushMatrix();
	glScalef(3.0, 3.0, 3.0);	// Scale the ground quad	
	glBegin(GL_QUADS);
	glNormal3f(0, 1, 0);	// Set quad normal direction.
	glTexCoord2f(0, 0);		// Set tex coordinates ( Using (0,0) -> (5,5) with texture wrapping set to GL_REPEAT to simulate the ground repeated grass texture).
	glVertex3f(-20, 0, -20);
	glTexCoord2f(5, 0);
	glVertex3f(20, 0, -20);
	glTexCoord2f(5, 5);
	glVertex3f(20, 0, 20);
	glTexCoord2f(0, 5);
	glVertex3f(-20, 0, 20);
	glEnd();
	glPopMatrix();

	glEnable(GL_LIGHTING);	// Enable lighting again for other entites coming throung the pipeline.

	glColor3f(1, 1, 1);	// Set material back to white instead of grey used for the ground texture.
}

//=======================================================================
// Display Function
//=======================================================================
void myDisplay(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	GLfloat lightIntensity[] = { 0.7, 0.7, 0.7, 1.0f };
	GLfloat lightPosition[] = { 0.0f, 100.0f, 0.0f, 0.0f };
	glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
	glLightfv(GL_LIGHT0, GL_AMBIENT, lightIntensity);

	// Draw Ground
	RenderGround();
	glPushMatrix();


	// Drawing the Game Objects
	//aladdin.draw();
	//glPopMatrix();
	if (flagFront) {
		//PLAYER
		glPushMatrix();
		glTranslated(speX + 1.1, 0.0, speZ + 0.3);
		aladdin.draw();
		glPopMatrix();
	}
	if (flagBack) {
		//PLAYER
		glPushMatrix();
		glTranslated(speX + 1.1, 0.0, speZ + 0.3);
		glRotated(180, 0, 1, 0);
		aladdin.draw();
		glPopMatrix();
	}
	if (flagRight) {
		//PLAYER
		glPushMatrix();
		glTranslated(speX + 1.1, 0.0, speZ + 0.3);
		glRotated(-90, 0, 1, 0);
		aladdin.draw();
		glPopMatrix();
	}
	if (flagLeft) {
		//PLAYER
		glPushMatrix();
		glTranslated(speX + 1.1, 0.0, speZ + 0.3);
		glRotated(90, 0, 1, 0);
		aladdin.draw();
		glPopMatrix();
	}
	//sky box


	drawScore();
	if (!endOne) {
		glPushMatrix();
		GLUquadricObj* qobj;
		qobj = gluNewQuadric();
		glTranslated(50, 0, 0);
		glRotated(90, 1, 0, 1);
		glBindTexture(GL_TEXTURE_2D, tex);
		gluQuadricTexture(qobj, true);
		gluQuadricNormals(qobj, GL_SMOOTH);
		gluSphere(qobj, 100, 100, 100);
		gluDeleteQuadric(qobj);
		glPopMatrix();
		glPushMatrix();

		cave.draw();
		glPopMatrix();



		for (GameObject snake : enemySnakes) {

			glPushMatrix();

			snake.draw();
			glPopMatrix();
		}

		for (GameObject wateri : water) {
			if (wateri.displayed) {
				glPushMatrix();
				glTranslatef(0.0, 1.1, 0.0);


				wateri.draw();
				glPopMatrix();
			}
		}

		for (GameObject rock : rocks) {
			glPushMatrix();
			// Dim the ground texture a bits

			rock.draw();
			glPopMatrix();
		}
	}
	else {
		glPushMatrix();
		glColor3f(0.2, 0.2, 0.2);
		GLUquadricObj* qobj;
		qobj = gluNewQuadric();
		glTranslated(50, 0, 0);
		glRotated(90, 1, 0, 1);
		glBindTexture(GL_TEXTURE_2D, tex);
		gluQuadricTexture(qobj, true);
		gluQuadricNormals(qobj, GL_SMOOTH);
		gluSphere(qobj, 100, 100, 100);
		gluDeleteQuadric(qobj);
		glPopMatrix();

	}
	glutSwapBuffers();
}

//=======================================================================
// Camera Position Function
//=======================================================================

void setCameraFollow() {

	float playerRotationAngle = aladdin.rotation;



	//float cameraPositionX = 0;
	//float cameraPositionZ = 0;

	// Set Camera Position Relative to player
	float cameraPositionX = -std::sinf(DEG2RAD(playerRotationAngle)) * cameraDistanceFromPlayer;
	float cameraPositionZ = -std::cosf(DEG2RAD(playerRotationAngle)) * cameraDistanceFromPlayer;

	cameraPositionX += aladdin.position.x;
	cameraPositionZ += aladdin.position.z;

	float cameraPositionY = aladdin.position.y + cameraHeightAbovePlayer;
	camera.eye = { cameraPositionX,cameraPositionY,cameraPositionZ };

	// Set Camera Target
	Vector cameraCenter = aladdin.position;


	if (firstPersonModeOn) {
		float cameraTargetX = std::sinf(DEG2RAD(playerRotationAngle)) * cameraDistanceFromPlayer * 10;
		float cameraTargetZ = std::cosf(DEG2RAD(playerRotationAngle)) * cameraDistanceFromPlayer * 10;

		cameraCenter.x -= cameraTargetX;
		cameraCenter.z -= cameraTargetZ;
	}

	cameraCenter.y += 7;
	camera.center = cameraCenter;

}

//=======================================================================
// Timer Function
//=======================================================================

bool checkCollitionObstacles() {

	for (int i = 0; i < enemySnakes.size(); i++) {
		if (compareDistances(aladdin.position, enemySnakes[i].position) < 3.0)
			std::cout << "d: " << q << std::endl;

		return true;
	}
	for (int i = 0; i < rocks.size(); i++) {
		if (compareDistances(aladdin.position, rocks[i].position) < 3.0)
			std::cout << "f: " << q << std::endl;

		return true;
	}

	return false;
}
void checkCollitionCollectables() {
	for (int i = 0; i < water.size(); i++) {
		if (compareDistances(aladdin.position, water[i].position) < aladdin.collisionRadius + water[i].collisionRadius) {

			water[i].setDisapear();
			score++;
		}
	}

}
void checkEndOne() {
	if (compareDistances(aladdin.position, cave.position) < aladdin.collisionRadius + cave.collisionRadius) { endOne = true; }


}

void myTimer(int) {

	setCameraFollow();
	setupCamera();

	// Add Enemy Snakes

	//std::cout << "Position X: " << aladdin.position.x << std::endl;
	//std::cout << "Position Z: " << aladdin.position.z << std::endl;


	while (enemySnakes.size() < MAX_NUMBER_OF_ENEMIES) {
		float newSnakeX;
		float newSnakeZ;
		Vector newSnakePosition;

		bool tooClose = false;

		while (true) {
			tooClose = false;

			newSnakeX = getRandomInt(-58, 58);
			newSnakeZ = getRandomInt(-58, 58);
			newSnakePosition = { newSnakeX,0,newSnakeZ };

			for (GameObject otherSnake : enemySnakes) {
				if (compareDistances(otherSnake.position, newSnakePosition) < MIN_ENEMY_CLOSENESS)
					tooClose = true;
			}

			if (!tooClose) {
				break;
			}

		}

		GameObject newSnake = GameObject(newSnakePosition, 0, 0.03, 0.5, "models/snake/snake.3ds", true);

		enemySnakes.push_back(newSnake);
	}
	while (rocks.size() < MAX_NUMBER_OF_ENEMIES) {
		float newRockX;
		float newRockZ;
		Vector newRockPosition;

		bool tooClose = false;

		while (true) {
			tooClose = false;

			newRockX = getRandomInt(-58, 58);
			newRockZ = getRandomInt(-58, 58);
			newRockPosition = { newRockX,0,newRockZ };

			for (GameObject otherSnake : enemySnakes) {
				if (compareDistances(otherSnake.position, newRockPosition) < MIN_ENEMY_CLOSENESS)
					tooClose = true;
			}
			for (GameObject otherRocks : rocks) {
				if (compareDistances(otherRocks.position, newRockPosition) < MIN_ENEMY_CLOSENESS)
					tooClose = true;
			}


			if (!tooClose) {
				break;
			}

		}

		GameObject newRock = GameObject(newRockPosition, 0, 0.3, 0.5, "models/rock1/rock.3ds", true);

		rocks.push_back(newRock);
	}

	while (water.size() < MAX_NUMBER_OF_ENEMIES) {
		float newWaterX;
		float newWaterZ;
		Vector newWaterPosition;

		bool tooClose = false;

		while (true) {
			tooClose = false;

			newWaterX = getRandomInt(-58, 58);
			newWaterZ = getRandomInt(-58, 58);
			newWaterPosition = { newWaterX,0,newWaterZ };

			for (GameObject otherSnake : enemySnakes) {
				if (compareDistances(otherSnake.position, newWaterPosition) < MIN_ENEMY_CLOSENESS)
					tooClose = true;
			}
			for (GameObject otherRocks : rocks) {
				if (compareDistances(otherRocks.position, newWaterPosition) < MIN_ENEMY_CLOSENESS)
					tooClose = true;
			}
			for (GameObject otherWater : water) {
				if (compareDistances(otherWater.position, newWaterPosition) < MIN_ENEMY_CLOSENESS)
					tooClose = true;
			}


			if (!tooClose) {
				break;
			}

		}

		GameObject newWater = GameObject(newWaterPosition, 0, 0.09, 0.5, "models/bottle/bottle.3ds", true);

		water.push_back(newWater);
	}
	aladdin.position.y += playerVerticalVelocity;
	playerVerticalVelocity += gravity;
	if (aladdin.position.y < 0) {
		aladdin.position.y = 0;
		playerVerticalVelocity = 0;
	}
	glutPostRedisplay();
	glutTimerFunc(1000 / 60, myTimer, 0);

}
float nplayerX = 0.0;
float nplayerZ = 0.0;
//=======================================================================
// Keyboard Function
//=======================================================================
void myKeyboard(unsigned char button, int x, int y)
{
	switch (button)
	{
	case 'w':
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

		break;
	case 'v':
		q += 5;
		break;
	case 'e':
		q -= 5;
		break;
	case 'r':
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		break;
	case 'u':
		flagFront = true;
		flagBack = false;
		flagRight = false;
		flagLeft = false;
		if (speZ >= -2.15)
			speZ += 0.5;
		break;
	case 'j':
		flagFront = false;
		flagBack = true;
		flagRight = false;
		flagLeft = false;
		if (speZ <= 2.5)
			speZ -= 0.5;
		break;
	case 'k':
		flagFront = false;
		flagBack = false;
		flagRight = true;
		flagLeft = false;
		if (speX <= 1.55)
			speX -= 0.5;
		break;
	case 'h':
		flagFront = false;
		flagBack = false;
		flagRight = false;
		flagLeft = true;
		if (speX >= -2.75)
			speX += 0.5;
		break;

	case 'x':
		jump();
		break;
	case 'f':
		if (!firstPersonModeOn) {
			firstPersonModeOn = true;
			cameraDistanceFromPlayer = -3;
			cameraHeightAbovePlayer = 7;
		}
		else {
			firstPersonModeOn = false;
			cameraDistanceFromPlayer = 17.0f;
			cameraHeightAbovePlayer = 10.0f;
		}
		break;
	case 27:
		exit(0);
		break;
	default:
		break;
	}

}
bool checkRotationCollision() {
	// Implement your rotation collision logic here
	// Return true if there is a collision, otherwise return false
	return false;
}

bool checkCollisionObstacles() {
	// Implement your collision logic with obstacles here
	// Return true if there is a collision, otherwise return false
	return false;
}

void checkCollisionCollectables() {
	// Implement your logic for collecting items here
}
//=======================================================================
// Special Function
//=======================================================================
int temp = 0;
constexpr float PI = 3.14159265358979323846f;

void updateDirection() {
	// Update the direction vector based on the current rotation
	aladdin.direction.x = std::cos(aladdin.rotation * PI / 180.0f);
	aladdin.direction.z = std::sin(aladdin.rotation * PI / 180.0f);
}
void mySpecial(int key, int x, int y) {
	float a = 1.0;
	float rh = std::sqrtf(1.0 / 2.0);
	const float xChange[] = { 0.0, -rh, -1.0, -rh, 0.0, rh, 1.0, rh };
	const float zChange[] = { 1.0, rh, 0.0, -rh, -1.0, -rh, 0.0, rh };

	float deltaX, deltaZ;  // Declare deltaX and deltaZ outside the switch statement

	switch (key) {
	case GLUT_KEY_UP:
		// Calculate movement based on current rotation
		deltaX = xChange[movementState];
		deltaZ = zChange[movementState];

		// Update player position
		aladdin.position.x += deltaX;
		aladdin.position.z += deltaZ;

		// Check collision
		if (checkCollisionObstacles()) {
			// If collision, revert the position change
			aladdin.position.x -= deltaX;
			aladdin.position.z -= deltaZ;
		}

		// Check collectables
		checkCollisionCollectables();
		break;

	case GLUT_KEY_DOWN:
		// Reverse the direction
		movementState = (movementState + 4) % 8;
		aladdin.setRotation(aladdin.rotation - 180);

		// Check collision after reversing direction
		if (checkCollisionObstacles()) {
			// If collision, revert the rotation change
			aladdin.setRotation(aladdin.rotation + 180);
		}
		checkCollisionCollectables();
		break;

	case GLUT_KEY_LEFT:
		temp = movementState;
		movementState = (movementState - 1 + 8) % 8;
		aladdin.setRotation(aladdin.rotation + 45);
		if (checkCollisionObstacles()) {
			movementState = temp;
		}
		checkCollisionCollectables();
		break;

	case GLUT_KEY_RIGHT:
		temp = movementState;
		movementState = (movementState + 1) % 8;
		aladdin.setRotation(aladdin.rotation - 45);
		if (checkCollisionObstacles()) {
			movementState = temp;
		}
		checkCollisionCollectables();
		break;
	}
}


//=======================================================================
// Motion Function
//=======================================================================
void myMotion(int x, int y)
{


	setupCamera();

	GLfloat light_position[] = { 0.0f, 10.0f, 0.0f, 1.0f };
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);


}

//=======================================================================
// Mouse Function
//=======================================================================
void myMouse(int button, int state, int x, int y)
{
	y = HEIGHT - y;

	if (state == GLUT_DOWN)
	{
		cameraZoom = y;
	}
	if (state == GLUT_UP)
	{

	}
}


//=======================================================================
// Reshape Function
//=======================================================================
void myReshape(int w, int h)
{
	if (h == 0) {
		h = 1;
	}

	WIDTH = w;
	HEIGHT = h;

	// set the drawable region of the window
	glViewport(0, 0, w, h);

	// set up the projection matrix 
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(fovy, (GLdouble)WIDTH / (GLdouble)HEIGHT, zNear, zFar);

	// go back to modelview matrix so we can move the objects about
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	setupCamera();
}

//=======================================================================
// Assets Loading Function
//=======================================================================
void LoadAssets()
{
	// Loading texture files
	if (!endOne) {
		tex_ground.Load("Textures/sand.bmp");
		loadBMP(&tex, "Textures/blu-sky-3.bmp", true);
	}
	else {
		tex_ground.Load("Textures/caveground.bmp");
		loadBMP(&tex, "Textures/caveground.bmp", true);

	}
}




//=======================================================================
// Main Function
//=======================================================================
void main(int argc, char** argv)
{
	glutInit(&argc, argv);

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(WIDTH, HEIGHT);
	glutInitWindowPosition(100, 150);
	glutCreateWindow(title);
	glutDisplayFunc(myDisplay);
	glutTimerFunc(0, myTimer, 0);
	glutKeyboardFunc(myKeyboard);
	glutSpecialFunc(mySpecial);
	// glutMotionFunc(myMotion);
	// glutMouseFunc(myMouse);
	// glutReshapeFunc(myReshape);

	myInit();

	LoadAssets();
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_NORMALIZE);
	glEnable(GL_COLOR_MATERIAL);

	glShadeModel(GL_SMOOTH);


	glutMainLoop();
}