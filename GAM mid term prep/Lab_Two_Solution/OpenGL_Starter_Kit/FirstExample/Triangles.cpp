


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


const GLint NumBuffers = 2;

GLuint Buffers[NumBuffers];

const GLuint NumVertices = 24;

//Will be used to store the location of our model-view matrix in VRAM
GLuint model_view_matrix_location;

//Will be used to store the location of our camera matrix in VRAM
GLuint camera_matrix_location;

//Will be used to store the location of our projection matrix in VRAM
GLuint projection_matrix_location;

//Will be used to create animation (rotate the square)
GLfloat alpha = 0;

//Camera parameter values
glm::vec3 cam_pos = glm::vec3(-1.0, 1.0, 1.0);

//Camera looking point (used for direction of the camera)
glm::vec3 cam_dir = glm::vec3(0.0, 0.0, 0.0);

//Camera Up-Vector
glm::vec3 cam_up_vector = glm::vec3(0.0, 0.0, 1.0);


//---------------------------------------------------------------------
//
// initialization: Setting up our rendering pipeline. Loading Shaders. Creating buffers in VRAM. 
//
//---------------------------------------------------------------------
void init(void)
{
	glEnable(GL_DEPTH_TEST);	//Enabling the depth test so that closer objects to camera obscure further objects

	ShaderInfo shaders[] = {
		{ GL_VERTEX_SHADER, "triangles.vert" },
		{ GL_FRAGMENT_SHADER, "triangles.frag" },
		{ GL_NONE, NULL }
	};

	GLuint program = LoadShaders(shaders);	//Loading the shaders from files: triangles.vert & triangles.frag
	glUseProgram(program);	//Compiling and running the shaders. And now, my Pipeline is set up :-)

	GLfloat vertices[NumVertices][3] = {
		// We use two triangles and connect them together to get a square. We're ONLY allowed to use triangles in this course.

		//First triangle
		{ -0.45, 0.45, 0.0 },
		{ -0.45, 0.45, 0.9 },
		{ 0.45, 0.45, 0.9 },

		//Second triangle
		{ 0.45, 0.45, 0.9 },
		{ 0.45, 0.45, 0.0 },
		{ -0.45, 0.45, 0.0 },



		//Third triangle
		{ -0.45, -0.45, 0.0 },
		{ -0.45, -0.45, 0.9 },
		{ 0.45, -0.45, 0.9 },

		//Fourth triangle
		{ 0.45, -0.45, 0.9 },
		{ 0.45, -0.45, 0.0 },
		{ -0.45,-0.45, 0.0 },



		//Fifth triangle
		{ 0.45, -0.45, 0.0 },
		{ 0.45, -0.45, 0.9 },
		{ 0.45, 0.45, 0.9 },

		//Sixth triangle
		{ 0.45, 0.45, 0.9 },
		{ 0.45, -0.45, 0.0 },
		{ 0.45,0.45, 0.0 },



		//Seventh triangle
		{ -0.45, -0.45, 0.0 },
		{ -0.45, -0.45, 0.9 },
		{ -0.45, 0.45, 0.9 },

		//Eighth triangle
		{ -0.45, 0.45, 0.9 },
		{ -0.45, -0.45, 0.0 },
		{ -0.45,0.45, 0.0 },


	};

	GLfloat colorData[NumVertices][3] = {
		{ 1,0,0 }, // colors for vertices of the first triangle
		{ 1,0,0 },
		{ 1,0,0 },

		{ 1,0,0 }, // colors for vertices of the second triangle
		{ 1,0,0 },
		{ 1,0,0 },

		{ 0,1,0 }, // colors for vertices of the third triangle
		{ 0,1,0 },
		{ 0,1,0 },

		{ 0,1,0 }, // colors for vertices of the fourth triangle
		{ 0,1,0 },
		{ 0,1,0 },

		{ 0,0,1 }, // colors for vertices of the third triangle
		{ 0,0,1 },
		{ 0,0,1 },

		{ 0,0,1 }, // colors for vertices of the fourth triangle
		{ 0,0,1 },
		{ 0,0,1 },

		{ 0,1,1 }, // colors for vertices of the third triangle
		{ 0,1,1 },
		{ 0,1,1 },

		{ 0,1,1 }, // colors for vertices of the fourth triangle
		{ 0,1,1 },
		{ 0,1,1 }

	};

	//We allocate two buffers in VRAM: One for vertex data and the other for colors
	//Once created, we use Buffers[0] to point to the first, and Buffers[1] to point to the second.
	glGenBuffers(2, Buffers);

	//-----------------------------------------
	//Initializing the first buffer: Buffers[0]
	//-----------------------------------------

	//Selecting Buffers[0]
	glBindBuffer(GL_ARRAY_BUFFER, Buffers[0]);

	//Pushing the vertices data into the buffer (transmission from RAM to VRAM)
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	//During the run-time, the buffer data should be transferred to the "vPosition" variable in the vertex shader
	glBindAttribLocation(program, 0, "vPosition");

	//We specify the format of the data in the buffer: 
	// GL_FLOAT: They are float as we have used a GLfloat[] array up in the code
	// 2: They must be considered as couples since we have 2D vertices
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	//Enable the buffer
	glEnableVertexAttribArray(0);
	//-------------------------------------------------------------------------
	//---------------We are done initializing the first buffer-----------------
	//-------------------------------------------------------------------------


	//-----------------------------------------
	//Initializing the second buffer: Buffers[1]
	//-----------------------------------------

	//Selecting Buffers[1]
	glBindBuffer(GL_ARRAY_BUFFER, Buffers[1]);

	//Pushing the color data into the buffer (transmission from RAM to VRAM)
	glBufferData(GL_ARRAY_BUFFER, sizeof(colorData), colorData, GL_STATIC_DRAW);

	//During the run-time, the buffer data should be transferred to the "vertexColor" variable in the vertex shader
	glBindAttribLocation(program, 1, "vertexColor");

	//We specify the format of the data in the buffer: 
	// GL_FLOAT: They are float as we have used a GLfloat[] array up in the code
	// 3: They must be considered as triplets since we use 3 values for each color(red, green, blue)
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	//Enable the buffer
	glEnableVertexAttribArray(1);
	//-------------------------------------------------------------------------
	//---------------We are done initializing the second buffer----------------
	//-------------------------------------------------------------------------


	//Retreiving the memory address of the "model_view" matrix in VRAM (in vertex shader)
	model_view_matrix_location = glGetUniformLocation(program, "model_view");

	//Retreiving the memory address of the "camera" matrix in VRAM (in vertex shader)
	camera_matrix_location = glGetUniformLocation(program, "camera");

	//Retreiving the memory address of the "projection" matrix in VRAM (in vertex shader)
	projection_matrix_location = glGetUniformLocation(program, "projection");
}
//---------------Initialization accomplished :-)




//---------------------------------------------------------------------
//
// display function. All drawings happen here in the "display" function
//
//---------------------------------------------------------------------
void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT);	//Clears the screen and ready to draw

	//Clearing the depth buffer
	glClear(GL_DEPTH_BUFFER_BIT);

	//Building our transformation matrices using glm library

	//NOTE: 
	//THE ORDER OF TRANSFOEMATION DOES MATTER. A rotation followed by a translation- 
	//might return a different outcome than a translation followed by a rotation.

	//We'd like to translate our geometry 0.5 units along the X-axis
	glm::mat4 model_view_matrix = glm::mat4(1.0);


	//Passing the built model-view_matrix to the vertex shader. Recall: variable 'location' contains the memory address of the model_view
	//matrix that is used in our vertex shader.
	glUniformMatrix4fv(model_view_matrix_location, 1, GL_FALSE, &model_view_matrix[0][0]);



	//And now, building our camera matrix using glm library.
	//More details about camera canbe found here: https://learnopengl.com/Getting-started/Camera
	//Setting up my camera in the scene: location, direction and up_vector.
	//We use glm to generate the matrix representing the state of my camera (location, direction, up_vector).
	//The first parameter is the vector representing the location of the camera corresponding to world-coordinate-system.
	//The second parameter is the point in space that the camera is looking at.
	//The third parameter is the up-vector of the camera.
	cam_pos.x = 1 * cos(alpha);
	cam_pos.y = 1 * sin(alpha);

	glm::mat4 camera_mat = glm::lookAt(cam_pos, cam_dir, cam_up_vector);
	glUniformMatrix4fv(camera_matrix_location, 1, GL_FALSE, &camera_mat[0][0]);



	//Finally, developing our projection matrix using glm library
	glm::mat4 projection_mat = glm::frustum(-0.1, +0.1, -0.1, +0.1, 0.01, 10.0);
	glUniformMatrix4fv(projection_matrix_location, 1, GL_FALSE, &projection_mat[0][0]);


	glDrawArrays(GL_TRIANGLES, 0, NumVertices);	//Connect the vertices using "GL_TRIANGLES" modes.
										//Read more about draw modes here:
										//https://www.glprogramming.com/red/chapter02.html
										//https://www.khronos.org/registry/OpenGL-Refpages/es2.0/xhtml/glDrawArrays.xml




	glFlush();							//Flush the rendered contents on the screen.
}

void keyboard(unsigned char key, int x, int y)
{
	//Handler function for keyboard events
	//To be developed in next lectures

	switch (key)
	{
	case 'i':
		cam_pos.z -= 0.005;
		break;
	case 'o':
		cam_pos.z += 0.005;
		break;
	case 'w':
		cam_pos.y += 0.001;
		break;
	case 's':
		cam_pos.y -= 0.001;
		break;
	case 'a':
		cam_pos.x -= 0.001;
		break;
	case 'd':
		cam_pos.x += 0.001;
		break;
	}

	glutPostRedisplay();
}

void mouse(int state, int button, int x, int y)
{
	//Handler function for keyboard events
	//To be developed in next lectures
}

//This function gets called for every frame. This will be used to animate the world 
void idle()
{
	alpha += 0.0005;
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

	//The dimensions of the window
	glutInitWindowSize(712, 712);

	//The title for the window
	glutCreateWindow("Hello World");

	glewInit();	//Initializes the glew and prepares the drawing pipeline.

	init();

	//Defining our 'display', 'idle', 'mouse' and 'keyboard' functions.
	glutDisplayFunc(display);
	glutIdleFunc(idle);
	glutKeyboardFunc(keyboard);
	glutMouseFunc(mouse);

	//The following function makes the OpenGL to go through an infinite loop and waits for any event from keyboard, mouse, etc.
	glutMainLoop();



}

