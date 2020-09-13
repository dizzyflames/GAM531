
///////////////////////////////////////////////////////////////////////
//
// triangles.cpp
//
///////////////////////////////////////////////////////////////////////

using namespace std;

#include "vgl.h"
#include "LoadShaders.h"
#include "glm\glm.hpp"
#include "glm\gtc\matrix_transform.hpp"
#include "glm\gtx\rotate_vector.hpp"
#include <iostream>

enum VAO_IDs { Triangles, NumVAOs }; 
enum Buffer_IDs { ArrayBuffer, NumBuffers };
enum Attrib_IDs { vPosition = 0 };

GLuint VAOs[NumVAOs];
GLuint Buffers[NumBuffers];
GLuint location;
GLuint cam_mat_location;
GLuint proj_mat_location;

const GLuint NumVertices = 16;

//Height of camera (player) from the level
float height = 0.8f;

//Player motion speed for movement and pitch/yaw
float travel_speed = 30.0f;		//Motion speed
float mouse_sensitivity = 0.01f;	//Pitch/Yaw speed

//Used for tracking mouse cursor position on screen
int x0 = 0;
int y_0 = 0;
float theta = 0.0f;
float lookBackRate = 0.04f;
float tempBack = 0;

//Transformation matrices and camera vectors
glm::mat4 model_view;
glm::vec3 unit_z_vector = glm::vec3(0, 0, 1);	//Assigning a meaningful name to (0,0,1) :-)
glm::vec3 cam_pos = glm::vec3(0.0f, 0.0f, height);
glm::vec3 forward_vector = glm::vec3(1, 1, 0);	//Forward vector is parallel to the level at all times (No pitch)

//The direction which the camera is looking, at any instance
glm::vec3 looking_dir_vector = glm::vec3(1, 1, 0);
glm::vec3 up_vector = unit_z_vector;
glm::vec3 side_vector = glm::cross(up_vector, forward_vector);


//Used to measure time between two frames
int oldTimeSinceStart = 0;
int deltaTime;

//Creating and rendering bunch of objects on the scene to interact with
const int Num_Obstacles = 100;
float obstacle_data[Num_Obstacles][3];

//Used to check if camera is looking back or crouching
bool backLook = false;
bool backLookReverse = false;
bool crouch = false;

//check if key is pressed
bool keyPressed = false;

//Helper function to generate a random float number within a range
float randomFloat(float a, float b)
{
	float random = ((float)rand()) / (float)RAND_MAX;
	float diff = b - a;
	float r = random * diff;
	return a + r;
}

// inititializing buffers, coordinates, setting up pipeline, etc.
void init(void)
{
	glEnable(GL_DEPTH_TEST);

	//Normalizing all vectors
	up_vector = glm::normalize(up_vector);
	forward_vector = glm::normalize(forward_vector);
	looking_dir_vector = glm::normalize(looking_dir_vector);
	side_vector = glm::normalize(side_vector);

	//Randomizing the position and scale of obstacles
	for (int i = 0; i < Num_Obstacles; i++)
	{
		obstacle_data[i][0] = randomFloat(-50, 50); //X
		obstacle_data[i][1] = randomFloat(-50, 50); //Y
		obstacle_data[i][2] = randomFloat(0.1, 10.0); //Scale
	}

	ShaderInfo shaders[] = {
		{ GL_VERTEX_SHADER, "triangles.vert" },
		{ GL_FRAGMENT_SHADER, "triangles.frag" },
		{ GL_NONE, NULL }
	};

	GLuint program = LoadShaders(shaders);
	glUseProgram(program);	//My Pipeline is set up

	GLfloat vertices[NumVertices][3] = {
		{ -0.45, -0.45 ,0.45 }, // Cube
		{ 0.45, -0.45 ,0.45 },
		{ 0.45, 0.45 ,0.45 },
		{ -0.45, 0.45 ,0.45 },
		{ -0.45, -0.45 ,-0.45 },
		{ 0.45, -0.45 ,-0.45 },
		{ 0.45, 0.45 ,-0.45 },
		{ -0.45, 0.45 ,-0.45 },

		{ -100.0, -100.0, 0.0 }, //Plane to walk on and a sky
		{ 100.0, -100.0, 0.0 },
		{ 100.0, 100.0, 0.0 },
		{ -100.0, 100.0, 0.0 },
		{ -100.0, -100.0, 10.0 },
		{ 100.0, -100.0, 10.0 },
		{ 100.0, 100.0, 10.0 },
		{ -100.0, 100.0, 10.0 }

	};

	GLfloat colorData[NumVertices][3] = {
		{ 0,1,1 }, // color for cube vertices
		{ 1,1,0 },
		{ 1,0,0 },
		{ 0,1,0 },
		{ 0,1,0 },
		{ 0,0,1 },
		{ 1,1,1 },
		{ 1,0,1 },

		{ 0,1,0 }, // color for plane vertices
		{ 0,1,0 },
		{ 0,1,0 },
		{ 0,1,0 },
		{ 0,0,1 },
		{ 0,0,1 },
		{ 0,0,1 },
		{ 0,0,1 }
	};

	glGenBuffers(2, Buffers);
	glBindBuffer(GL_ARRAY_BUFFER, Buffers[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glBindAttribLocation(program, 0, "vPosition");
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, Buffers[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(colorData), colorData, GL_STATIC_DRAW);
	glBindAttribLocation(program, 1, "vertexColor");
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(1);

	location = glGetUniformLocation(program, "model_matrix");
	cam_mat_location = glGetUniformLocation(program, "camera_matrix");
	proj_mat_location = glGetUniformLocation(program, "projection_matrix");
}

//Helper function to draw a cube
void drawCube(float scale)
{
	GLubyte top_face[] = { 0, 1, 2, 3 };
	GLubyte bottom_face[] = { 4, 5, 6, 7 };
	GLubyte left_face[] = { 0, 4, 7, 3 };
	GLubyte right_face[] = { 1, 5, 6, 2 };
	GLubyte front_face[] = { 2, 3, 7, 6 };
	GLubyte back_face[] = { 2, 3, 7, 6 };
	model_view = glm::scale(model_view, glm::vec3(scale, scale, scale));
	glUniformMatrix4fv(location, 1, GL_FALSE, &model_view[0][0]);
	glDrawElements(GL_QUADS, 4, GL_UNSIGNED_BYTE, top_face);
	glDrawElements(GL_QUADS, 4, GL_UNSIGNED_BYTE, bottom_face);
	glDrawElements(GL_QUADS, 4, GL_UNSIGNED_BYTE, left_face);
	glDrawElements(GL_QUADS, 4, GL_UNSIGNED_BYTE, right_face);
	glDrawElements(GL_QUADS, 4, GL_UNSIGNED_BYTE, front_face);
	glDrawElements(GL_QUADS, 4, GL_UNSIGNED_BYTE, back_face);
}

//Renders level
void draw_level()
{
	//Drawing the floor and the sky
	GLubyte ground[] = { 8, 9, 10, 11 };
	GLubyte sky[] = { 12, 13, 14, 15 };
	glDrawElements(GL_QUADS, 4, GL_UNSIGNED_BYTE, ground);
	glDrawElements(GL_QUADS, 4, GL_UNSIGNED_BYTE, sky);

	//Rendering obstacles obstacles
	for (int i = 0; i < Num_Obstacles; i++)
	{
		model_view = glm::translate(model_view, glm::vec3(obstacle_data[i][0], obstacle_data[i][1], 0.0));
		glUniformMatrix4fv(location, 1, GL_FALSE, &model_view[0][0]);
		drawCube(obstacle_data[i][2]);
		model_view = glm::mat4(1.0);
	}
}

//---------------------------------------------------------------------
//
// display
//
void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	model_view = glm::mat4(1.0);

	//The 3D point in space that the camera is looking
	glm::vec3 look_at = cam_pos + looking_dir_vector;

	glm::mat4 camera_matrix = glm::lookAt(cam_pos, look_at, up_vector);
	glUniformMatrix4fv(cam_mat_location, 1, GL_FALSE, &camera_matrix[0][0]);

	glm::mat4 proj_matrix = glm::frustum(-0.01f, +0.01f, -0.01f, +0.01f, 0.01f, 100.0f);
	glUniformMatrix4fv(proj_mat_location, 1, GL_FALSE, &proj_matrix[0][0]);

	draw_level();

	glFlush();
}


void keyboard(unsigned char key, int x, int y)
{
	if (key == 'a')
	{
		//Moving camera along opposit direction of side vector
		cam_pos += side_vector * travel_speed * ((float)deltaTime) / 1000.0f;

	}
	if (key == 'd')
	{
		//Moving camera along side vector
		cam_pos -= side_vector * travel_speed * ((float)deltaTime) / 1000.0f;
	}
	if (key == 'w')
	{
		//Moving camera along forward vector. To be more realistic, we use X=V.T equation in physics
		cam_pos += forward_vector * travel_speed * ((float)deltaTime) / 1000.0f;
		cam_pos.z += 0.05 * sin(4 * theta);
		theta += 0.1;

	}
	if (key == 's')
	{
		//Moving camera along backward (negative forward) vector. To be more realistic, we use X=V.T equation in physics
		cam_pos -= forward_vector * travel_speed * ((float)deltaTime) / 1000.0f;
		cam_pos.z += 0.05 * sin(2 * theta);
		theta += 0.1;
	}
	if (key == 'b') {
			//The value 100 is used to slow down the rotation of head toward back.
			//We will turn the head back in 100 steps as folows:
			//The total rotation angle is 180 degrees which is equal to PI in radian (3.141).
			for (int a = 0; a < 100; a++) {
			looking_dir_vector = glm::rotate(looking_dir_vector, 3.141f/100.0f, unit_z_vector);
			up_vector = glm::rotate(up_vector, 3.141f / 100.0f, unit_z_vector);
			display();
		}
	}
	if (key == 'c') {
		if (!crouch) {
			cam_pos.z -= 0.5f;
			crouch = true;
		}
		else {
			cam_pos.z += 0.5f;
			crouch = false;
		}
	}

	if (glutGetModifiers() == GLUT_ACTIVE_SHIFT) {
		travel_speed *= 0.5;
	}
	if (key == 'z') {
		travel_speed *= 0.5;
		keyPressed = true;
	}
}

void keyboardUP(unsigned char key, int x, int y) {
	if (key == 'z') {
		travel_speed *= 2;
		keyPressed = false;
	}
}

//Controlling Pitch with vertical mouse movement
void mouse(int x, int y)
{
	//Controlling Yaw with horizontal mouse movement
	int delta_x = x - x0;

	//The following vectors must get updated during a yaw movement
	forward_vector = glm::rotate(forward_vector, -delta_x * mouse_sensitivity, unit_z_vector);
	looking_dir_vector = glm::rotate(looking_dir_vector, -delta_x * mouse_sensitivity, unit_z_vector);
	side_vector = glm::rotate(side_vector, -delta_x * mouse_sensitivity, unit_z_vector);
	up_vector = glm::rotate(up_vector, -delta_x * mouse_sensitivity, unit_z_vector);
	x0 = x;

	//The following vectors must get updated during a pitch movement
	int delta_y = y - y_0;
	glm::vec3 tmp_up_vec = glm::rotate(up_vector, delta_y * mouse_sensitivity, side_vector);
	glm::vec3 tmp_looking_dir = glm::rotate(looking_dir_vector, delta_y * mouse_sensitivity, side_vector);

	//The dot product is used to prevent the user from over-pitch (pitching 360 degrees)
	//The dot product is equal to cos(theta), where theta is the angle between looking_dir and forward vector
	GLfloat dot_product = glm::dot(tmp_looking_dir, forward_vector);

	//If the angle between looking_dir and forward vector is between (-90 and 90) degress 
	if (dot_product > 0)
	{
		up_vector = glm::rotate(up_vector, delta_y * mouse_sensitivity, side_vector);
		looking_dir_vector = glm::rotate(looking_dir_vector, delta_y * mouse_sensitivity, side_vector);
	}
	y_0 = y;
}

void idle()
{
	//Calculating the delta time between two frames
	//We will use this delta time when moving forward (in keyboard function)
	int timeSinceStart = glutGet(GLUT_ELAPSED_TIME);
	deltaTime = timeSinceStart - oldTimeSinceStart;
	oldTimeSinceStart = timeSinceStart;
	//cout << timeSinceStart << " " << oldTimeSinceStart << " " << deltaTime << endl;
	if (backLook)
	{
		if (!backLookReverse) {
			looking_dir_vector = glm::rotate(looking_dir_vector, lookBackRate, unit_z_vector);
			if (looking_dir_vector.y >= -tempBack) {
				backLook = false;
				backLookReverse = true;
			}
		}
		else {
			looking_dir_vector = glm::rotate(looking_dir_vector, -lookBackRate, unit_z_vector);
			if (looking_dir_vector.y <= -tempBack) {
				backLook = false;
				backLookReverse = false;
			}


		}
	}



	glutPostRedisplay();
}

//---------------------------------------------------------------------
//
// main
//
int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA);
	glutInitWindowSize(1024, 1024);
	glutCreateWindow("Camera and Projection");

	glewInit();	//Initializes the glew and prepares the drawing pipeline.

	init();

	glutDisplayFunc(display);

	glutKeyboardFunc(keyboard);
	glutKeyboardUpFunc(keyboardUP);

	glutIdleFunc(idle);

	glutPassiveMotionFunc(mouse);

	glutMainLoop();



}
