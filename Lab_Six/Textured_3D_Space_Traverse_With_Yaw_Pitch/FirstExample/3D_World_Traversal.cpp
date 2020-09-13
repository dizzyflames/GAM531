
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
#include "..\SOIL\src\SOIL.h"
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>

enum VAO_IDs { Triangles, NumVAOs };
enum Buffer_IDs { ArrayBuffer};
enum Attrib_IDs { vPosition = 0 };

const GLint NumBuffers = 2;
GLuint VAOs[NumVAOs];
GLuint Buffers[NumBuffers];
GLuint location;
GLuint cam_mat_location;
GLuint proj_mat_location;
//GLuint texture[2];	//Array of pointers to textrure data in VRAM. We use two textures in this example.
GLfloat vertices[483][3];
GLuint sphereF[960][3];

const GLuint NumVertices = 28;

//Height of camera (player) from the level
float height = 0.8f;
GLfloat alpha = 0.0f;
float look_level = 0.0;

//Player motion speed for movement and pitch/yaw
float travel_speed = 300.0f;		//Motion speed
float mouse_sensitivity = 0.01f;	//Pitch/Yaw speed

//Used for tracking mouse cursor position on screen
int x0 = 0;	
int y_0 = 0;
 
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
const int Num_Obstacles = 1;
float obstacle_data[Num_Obstacles][3];

int load(std::string filename, GLfloat vertexArray[][3], GLuint faces[][3]) {
	//open file
	std::cout << " -- Read file started -- " << std::endl;

	std::ifstream file(filename);

	if (file.is_open())
	{
		std::cout << " --File Opened --" << std::endl;
		std::string line;
		int ln = 0;
		int vertNum = 1; //starts at one as faces starts looking at index 1
		int faceNum = 0;
		while (getline(file, line))
		{
			//std::cout << "Reading Line: " << ln << " : " << line << std::endl;
			ln++;
			if (!line.empty())
			{
				if (line.at(0) == 'v')
				{
					float temp;
					float temparr[3];
					std::stringstream ss;
					ss << line;

					int i = 0;
					std::string t;
					while (!ss.eof()) {
						ss >> t;
						//std::cout << "Token: " << std::endl;
						if (std::stringstream(t) >> temp && i <= 3) {
							//std::cout << "Store: " << temp << std::endl;
							temparr[i] = temp;
							i++;
						}
						t = "";
					}

					if (true) //put out of bounds checking
					{
						for (int i = 0; i < 3; i++)
						{
							vertexArray[vertNum][i] = temparr[i];
						}
					}
					else
					{
						std::cout << "ERROR: Could not add vertex to vertexArray" << std::endl;
						return 0;
					}
					vertNum++;
				}


				if (line.at(0) == 'f')
				{
					int temp;
					int temparr[3];
					std::stringstream ss;
					ss << line;

					int i = 0;
					std::string t;
					while (!ss.eof()) {
						ss >> t;
						//std::cout << "Token: " << t << std::endl;
						if (std::stringstream(t) >> temp && i <= 3) {
							temparr[i] = temp;
							i++;
						}
						t = "";
					}

					if (true) //put out of bounds checking here
					{
						for (int i = 0; i < 3; i++)
						{
							faces[faceNum][i] = temparr[i];
						}
					}
					else
					{
						std::cout << "ERROR: Could not add face to faces" << std::endl;
						return 0;
					}
					faceNum++;
				}
			}
		}
		file.close();
		std::cout << "Done!" << std::endl;
		return 1;
	}
	else
	{
		std::cout << " ERROR: Cannot open file " << filename << std::endl;
		return -1;
	}
}

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
	
	//GLfloat vertices[482][3];
	height = 60.0;
	up_vector = glm::rotate(up_vector, 1.57f, side_vector);
	forward_vector = glm::rotate(forward_vector, 1.57f, side_vector);
	cam_pos = glm::vec3(cam_pos.x, cam_pos.y, height);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	/*for (int i = 0; i < 960; i++) {
		GLuint v1 = sphereF[i][0];
		GLuint v2 = sphereF[i][1];
		GLuint v3 = sphereF[i][2];
		GLuint f[3] = { v1, v2, v3 };
		// last parameter of gldrawelements
		// GL_TRIANGLE, 3, GLuint, f
		glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, f);
	}*/

	glEnable(GL_DEPTH_TEST);
	load("Sphere.obj", vertices, sphereF); // add code here set the sphere vertices and faces
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
	/*obstacl_data[0][0] = 0;
	obstacle_data[0][1] = 0;
	obstacl_data[0][2] = 0;

	obstacle_data[1][0] = 1;
	obstacle_data[1][1] = 0;
	obstacle_data[1][2] = 0;

	obstacl_data[2][0] = 1.1;
	obstacle_data[2][1] = 0;
	obstacle_data[2][2] = 0;*/

	ShaderInfo shaders[] = {
		{ GL_VERTEX_SHADER, "triangles.vert" },
		{ GL_FRAGMENT_SHADER, "triangles.frag" },
		{ GL_NONE, NULL }
	};

	GLuint program = LoadShaders(shaders);
	glUseProgram(program);	//My Pipeline is set up

	/*GLfloat vertices[NumVertices][3] = {
		
		{ -100.0, -100.0, 0.0 }, //Plane to walk on and a sky
		{ 100.0, -100.0, 0.0 },
		{ 100.0, 100.0, 0.0 },
		{ -100.0, 100.0, 0.0 },

		{ -0.45, -0.45 ,0.01 }, // bottom face
		{ 0.45, -0.45 ,0.01 },
		{ 0.45, 0.45 ,0.01 },
		{ -0.45, 0.45 ,0.01 },

		{ -0.45, -0.45 ,0.9 }, //top face
		{ 0.45, -0.45 ,0.9 },
		{ 0.45, 0.45 ,0.9 },
		{ -0.45, 0.45 ,0.9 },

		{ 0.45, -0.45 , 0.01 }, //left face
		{ 0.45, 0.45 , 0.01 },
		{ 0.45, 0.45 ,0.9 },
		{ 0.45, -0.45 ,0.9 },

		{ -0.45, -0.45, 0.01 }, //right face
		{ -0.45, 0.45 , 0.01 },
		{ -0.45, 0.45 ,0.9 },
		{ -0.45, -0.45 ,0.9 },

		{ -0.45, 0.45 , 0.01 }, //front face
		{ 0.45, 0.45 , 0.01 },
		{ 0.45, 0.45 ,0.9 },
		{ -0.45, 0.45 ,0.9 },
	
		{ -0.45, -0.45 , 0.01 }, //back face
		{ 0.45, -0.45 , 0.01 },
		{ 0.45, -0.45 ,0.9 },
		{ -0.45, -0.45 ,0.9 },
	};*/

	//These are the texture coordinates for the second texture
	/*GLfloat textureCoordinates[28][2] = {
		0.0f, 0.0f,
		200.0f, 0.0f,
		200.0f, 200.0f,
		0.0f, 200.0f,
		
		0.0f, 1.0f,
		1.0f, 1.0f,
		1.0f, 0.0f,
		0.0f, 0.0f,
		
		0.0f, 1.0f,
		1.0f, 1.0f,
		1.0f, 0.0f,
		0.0f, 0.0f,

		0.0f, 1.0f,
		1.0f, 1.0f,
		1.0f, 0.0f,
		0.0f, 0.0f,

		0.0f, 1.0f,
		1.0f, 1.0f,
		1.0f, 0.0f,
		0.0f, 0.0f,

		0.0f, 1.0f,
		1.0f, 1.0f,
		1.0f, 0.0f,
		0.0f, 0.0f,

		0.0f, 1.0f,
		1.0f, 1.0f,
		1.0f, 0.0f,
		0.0f, 0.0f,
	};*/


	//Creating our texture:
	//This texture is loaded from file. To do this, we use the SOIL (Simple OpenGL Imaging Library) library.
	//When using the SOIL_load_image() function, make sure the you are using correct patrameters, or else, your image will NOT be loaded properly, or will not be loaded at all.
	GLint width1, height1;
	//unsigned char* textureData1 = SOIL_load_image("grass.png", &width1, &height1, 0, SOIL_LOAD_RGB);

	GLint width2, height2;
	//unsigned char* textureData2 = SOIL_load_image("apple.png", &width2, &height2, 0, SOIL_LOAD_RGB);

	glGenBuffers(2, Buffers);
	glBindBuffer(GL_ARRAY_BUFFER, Buffers[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glBindAttribLocation(program, 0, "vPosition");
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, Buffers[1]);
	//glBufferData(GL_ARRAY_BUFFER, sizeof(textureCoordinates), textureCoordinates, GL_STATIC_DRAW);
	glBindAttribLocation(program, 1, "vTexCoord");
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(1);

	location = glGetUniformLocation(program, "model_matrix");
	cam_mat_location = glGetUniformLocation(program, "camera_matrix");
	proj_mat_location = glGetUniformLocation(program, "projection_matrix");

	///////////////////////TEXTURE SET UP////////////////////////
	
	//Allocating two buffers in VRAM
	//glGenTextures(2, texture);

	//First Texture: 

	//Set the type of the allocated buffer as "TEXTURE_2D"
	//glBindTexture(GL_TEXTURE_2D, texture[0]); commented here

	//Loading the second texture into the second allocated buffer:
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width1, height1, 0, GL_RGB, GL_UNSIGNED_BYTE, textureData1);

	//Setting up parameters for the texture that recently pushed into VRAM
	/*glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);*/


	//And now, second texture: 

	//Set the type of the allocated buffer as "TEXTURE_2D"
	//glBindTexture(GL_TEXTURE_2D, texture[1]); commented here

	//Loading the second texture into the second allocated buffer:
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width2, height2, 0, GL_RGB, GL_UNSIGNED_BYTE, textureData2);

	//Setting up parameters for the texture that recently pushed into VRAM
	/*glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);*/
	//////////////////////////////////////////////////////////////
}

//Helper function to draw a cube
void drawCube(float scale)
{
	model_view = glm::scale(model_view, glm::vec3(scale, scale, scale));
	glUniformMatrix4fv(location, 1, GL_FALSE, &model_view[0][0]);

	//Select the second texture (apple.png) when drawing the second geometry (cube)
	glBindTexture(GL_TEXTURE_2D, texture[1]); commented here
	glDrawArrays(GL_QUADS, 4, 24);
	//int numFaces = sizeof(sphereF) / (3 * sizeof(GLfloat));
	/*for (int i = 0; i < numFaces; i++) {
		GLuint v1 = sphereF[i][0];
		GLuint v2 = sphereF[i][1];
		GLuint v3 = sphereF[i][2];
		GLuint f[3] = { v1, v2, v3 };
		// last parameter of gldrawelements
		// GL_TRIANGLE, 3, GLuint, f
		glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, f);
	}*/
}

void drawVehicle(float scaleX, float scaleY, float scaleZ) {
	//drawCube();
	// wheel : transformation, drawcube

	// collision update coordinates in idle function if cam coordinates overlap with vehicle
	// x - D/2 < cam pos x < x + D/2
	// y - D/2 < cam pos y < y + D / 2
}
//Renders level
void draw_level()
{
	//Select the first texture (grass.png) when drawing the first geometry (floor)
	//glBindTexture(GL_TEXTURE_2D, texture[0]); commented here
	//glDrawArrays(GL_QUADS, 0, 4);

	//Rendering obstacles obstacles
	/*for (int i = 0; i < Num_Obstacles; i++)
	{
		model_view = glm::translate(model_view, glm::vec3(obstacle_data[i][0], obstacle_data[i][1], 2.0));
		glUniformMatrix4fv(location, 1, GL_FALSE, &model_view[0][0]);
		drawCube(obstacle_data[i][2]);
		model_view = glm::mat4(1.0);
	}*/
	/*for (int i = 0; i < 960; i++) {
		GLuint v1 = sphereF[i][0];
		GLuint v2 = sphereF[i][1];
		GLuint v3 = sphereF[i][2];
		GLuint f[3] = { v1, v2, v3 };
		// last parameter of gldrawelements
		// GL_TRIANGLE, 3, GLuint, f
		glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, f);
	}*/
	glm::mat4 sun = glm::mat4(1.0);
	glm::mat4 earth;
	glm::mat4 moon;

	drawCube(1.0);

	earth = glm::rotate(sun, alpha, glm::vec3(0.0, 0.0, -1));
	earth = glm::translate(earth, glm::vec3(30.0, 0.0, 0.0));
	earth = glm::scale(earth, glm::vec3(0.5, 0.5, 0.5));

	glUniformMatrix4fv(location, 1, GL_FALSE, &earth[0][0]);

	drawCube(0.5);

	moon = glm::rotate(earth, alpha, glm::vec3(0.0, 0.0, -1));
	moon = glm::translate(moon, glm::vec3(15.0, 0.0, 0.0));
	moon = glm::scale(moon, glm::vec3(0.3, 0.3, 0.3));

	glUniformMatrix4fv(location, 1, GL_FALSE, &moon[0][0]);

	drawCube(0.2);

	//drawCube(1.0);
}

//---------------------------------------------------------------------
//
// display
//
void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	model_view = glm::mat4(1.0);
	glUniformMatrix4fv(location, 1, GL_FALSE, &model_view[0][0]);

	//The 3D point in space that the camera is looking
	glm::vec3 look_at = glm::vec3(cam_pos.x + forward_vector.x, cam_pos.y + forward_vector.y, look_level);//cam_pos + looking_dir_vector;

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
		//cam_pos += side_vector * travel_speed * ((float)deltaTime) / 1000.0f;
		cam_pos += glm::cross(up_vector, forward_vector) * travel_speed * ((float)deltaTime) / 1000.0f;
	}
	if (key == 'd')
	{
		//Moving camera along side vector
		//cam_pos -= side_vector * travel_speed * ((float)deltaTime) / 1000.0f;
		cam_pos -= glm::cross(up_vector, forward_vector) * travel_speed * ((float)deltaTime) / 1000.0f;
	}
	if (key == 'w')
	{
		//Moving camera along forward vector. To be more realistic, we use X=V.T equation in physics
		//cam_pos += forward_vector * travel_speed * ((float)deltaTime) / 1000.0f;
		cam_pos += up_vector * travel_speed * ((float)deltaTime) / 1000.0f;
		
	}
	if (key == 's')
	{
		//Moving camera along backward (negative forward) vector. To be more realistic, we use X=V.T equation in physics
		//cam_pos -= forward_vector * travel_speed * ((float)deltaTime) / 1000.0f;
		cam_pos -= up_vector * travel_speed * ((float)deltaTime) / 1000.0f;
	}
	if (key == 'j') {
		cam_pos -= forward_vector * travel_speed * ((float)deltaTime) / 1000.0f;
	}
	if (key == 'u') {
		cam_pos += forward_vector * travel_speed * ((float)deltaTime) / 1000.0f;
	}
}

//Controlling Pitch with vertical mouse movement
/*void mouse(int x, int y)
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
}*/

void idle()
{
	//Calculating the delta time between two frames
	//We will use this delta time when moving forward (in keyboard function)
	int timeSinceStart = glutGet(GLUT_ELAPSED_TIME);
	deltaTime = timeSinceStart - oldTimeSinceStart;
	oldTimeSinceStart = timeSinceStart;
	//cout << timeSinceStart << " " << oldTimeSinceStart << " " << deltaTime << endl;

	alpha += 0.01;
	glutPostRedisplay();
	draw_level();
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

	glutIdleFunc(idle);

	//glutPassiveMotionFunc(mouse);

	glutMainLoop();
	
	

}
