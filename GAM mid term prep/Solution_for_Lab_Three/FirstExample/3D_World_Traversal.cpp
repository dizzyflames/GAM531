


///////////////////////////////////////////////////////////////////////
//
// 3D_World_Traversal.cpp
//
//
// Patrick O'Reilly - 109646174
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

//Player motion speed and key controls
float height = 0.8f;
float yaw_speed = 0.1f;
float travel_speed = 60.0f;
float mouse_sensitivity = 0.01f;

// to change to look at ground for top view
float look_level = height;
float ground = 0.0;

//Used for tracking mouse cursor position on screen
int x0 = 0;
int y_0 = 0;

//Transformation matrices and camera vectors
glm::mat4 model_view;
glm::vec3 cam_pos = glm::vec3(0.0f, 0.0f, height);
glm::vec3 forward_vector = glm::vec3(1, 1, 0);
glm::vec3 up_vector = glm::vec3(0, 0, 1);
glm::vec3 side_vector = glm::cross(up_vector, forward_vector);

//Used to measure time between two frames
int oldTimeSinceStart = 0;
int deltaTime;

//Creating and rendering bunch of objects on the scene to interact with
const int Num_Obstacles = 100;
float obstacle_data[Num_Obstacles][3];

// Camera Mode
size_t cam_mode = 0; // 0 is perspective, 1 is top down

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
		{ 1,0,0 }, // color for cube vertices
		{ 0,1,0 },
		{ 0,0,1 },
		{ 1,1,0 },
		{ 1,0,1 },
		{ 0,1,1 },
		{ 1,0,1 },
		{ 1,1,0 },

		{ 0,0.5,0 }, // color for plane vertices
		{ 0,0.5,0 },
		{ 0,0.5,0 },
		{ 0,0.5,0 },
		{ 0.5,0.85,0.9 },
		{ 0.5,0.85,0.9 },
		{ 0.5,0.85,0.9 },
		{ 0.5,0.85,0.9 }

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

	//We normalize the vectors below. Normalizing means to shrink the length of a vector to 1 (make it a unit vector). 
	//Note: The direction of the vector does not change.

	up_vector = glm::normalize(up_vector);
	forward_vector = glm::normalize(forward_vector);

	glm::vec3 look_at = glm::vec3(cam_pos.x + forward_vector.x, cam_pos.y + forward_vector.y, look_level);

	glm::mat4 camera_matrix = glm::lookAt(glm::vec3(cam_pos.x, cam_pos.y, cam_pos.z), glm::vec3(look_at.x, look_at.y, look_at.z), up_vector);
	glUniformMatrix4fv(cam_mat_location, 1, GL_FALSE, &camera_matrix[0][0]);

	glm::mat4 proj_matrix = glm::frustum(-0.01f, +0.01f, -0.01f, +0.01f, 0.01f, 100.0f);
	glUniformMatrix4fv(proj_mat_location, 1, GL_FALSE, &proj_matrix[0][0]);

	//Render the level
	draw_level();

	glFlush();
}


void keyboard(unsigned char key, int x, int y)
{
	if (cam_mode)
	{
		if (key == 'w')
		{
			cam_pos += up_vector * travel_speed * ((float)deltaTime) / 1000.0f;
		}
		if (key == 's')
		{
			cam_pos -= up_vector * travel_speed * ((float)deltaTime) / 1000.0f;
		}
		if (key == 'a')
		{
			//Moving camera along opposit direction of side vector
			cam_pos += glm::cross(up_vector, forward_vector) * travel_speed * ((float)deltaTime) / 1000.0f;
		}
		if (key == 'd')
		{
			//Moving camera along side vector
			cam_pos -= glm::cross(up_vector, forward_vector) * travel_speed * ((float)deltaTime) / 1000.0f;
		}
		if (key == 'p')
		{
			cam_mode = 0;
			height = 0.8f;
			look_level = height;
			up_vector = glm::rotate(up_vector, -1.57f, side_vector);
			forward_vector = glm::rotate(forward_vector, -1.57f, side_vector);
			cam_pos = glm::vec3(cam_pos.x, cam_pos.y, height);
		}
	}
	else {

		if (key == 'a')
		{
			//Moving camera along opposit direction of side vector
			cam_pos += glm::cross(up_vector, forward_vector) * travel_speed * ((float)deltaTime) / 1000.0f;
		}
		if (key == 'd')
		{
			//Moving camera along side vector
			cam_pos -= glm::cross(up_vector, forward_vector) * travel_speed * ((float)deltaTime) / 1000.0f;
		}
		if (key == 'w')
		{
			//Moving camera along forward vector. To be more realistic, we use X=V.T equation in physics
			cam_pos += forward_vector * travel_speed * ((float)deltaTime) / 1000.0f;
		}
		if (key == 's')
		{
			//Moving camera along backward vector. To be more realistic, we use X=V.T equation in physics
			cam_pos -= forward_vector * travel_speed * ((float)deltaTime) / 1000.0f;
		}
		if (key == 't')
		{
			cam_mode = 1;
			height = 30.0;
			look_level = ground;
			up_vector = glm::rotate(up_vector, 1.57f, side_vector);
			forward_vector = glm::rotate(forward_vector, 1.57f, side_vector);
			cam_pos = glm::vec3(cam_pos.x, cam_pos.y, height);
		}
	}

}

void mouse(int x, int y)
{
	if (!cam_mode) {
		int delta_x = x - x0;
		forward_vector = glm::rotate(forward_vector, -delta_x * mouse_sensitivity, up_vector);
		side_vector = glm::cross(up_vector, forward_vector);
		//cout << x0 << " " << x << " " << delta_x << endl;
		x0 = x;
	}
}

void idle()
{
	//Calculating the delta time between two frames
	//We will use this delta time when moving forward (in keyboard function)
	int timeSinceStart = glutGet(GLUT_ELAPSED_TIME);
	deltaTime = timeSinceStart - oldTimeSinceStart;
	oldTimeSinceStart = timeSinceStart;
	//cout << timeSinceStart << " " << oldTimeSinceStart << " " << deltaTime << endl;
	glutPostRedisplay();
}

//---------------------------------------------------------------------
//
// main
//

int
main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA);
	glutInitWindowSize(1920, 1080);
	glutCreateWindow("Camera and Projection");

	glewInit();	//Initializes the glew and prepares the drawing pipeline.

	init();

	glutDisplayFunc(display);

	glutKeyboardFunc(keyboard);

	glutIdleFunc(idle);

	glutPassiveMotionFunc(mouse);

	glutMainLoop();



}

