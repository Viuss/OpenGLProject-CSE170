#include <GL/glew.h>
#include <GL/freeglut.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include <iostream>
#include "shader.h"
#include "shaderprogram.h"
#include <cmath>
#include <vector>
#include <string>

/*=================================================================================================
	DOMAIN
=================================================================================================*/

// Window dimensions
const int InitWindowWidth  = 800;
const int InitWindowHeight = 800;
int WindowWidth  = InitWindowWidth;
int WindowHeight = InitWindowHeight;

// Last mouse cursor position
int LastMousePosX = 0;
int LastMousePosY = 0;

// Arrays that track which keys are currently pressed
bool key_states[256];
bool key_special_states[256];
bool mouse_states[8];

// Other parameters
bool draw_wireframe = false;

//shape structs
struct cubeData
{
	std::vector<glm::vec4> vertices;
	std::vector<glm::vec4> colors;
	std::vector<glm::vec4> normals;
	std::vector<glm::vec2> texCoords;
	std::vector<glm::vec3> position;
	GLuint cube_VAO;
	GLuint cube_VBO[4];

	cubeData(): vertices(), colors(), normals(), texCoords(), position() {}
	void createCube(cubeData& cube, float length, float width, float height, float xPos, float yPos, float zPos);

	cubeData(float length, float width, float height, float xPos, float yPos, float zPos) {
		createCube(*this, length, width, height, xPos, yPos, zPos);
	}

	~cubeData() {
		vertices.clear();
		colors.clear();
		normals.clear();
		texCoords.clear();
		position.clear();
		
	}
};

struct planeData
{
	std::vector<glm::vec4> vertices;
	std::vector<glm::vec4> colors;
	std::vector<glm::vec4> normals;
	std::vector<glm::vec2> texCoords;
	std::vector<glm::vec3> position;

	planeData(): vertices(), colors(), normals(), texCoords(), position() {}
	void createPlane(planeData& plane, float length, float width, float xPos, float yPos, float zPos);

	planeData(float length, float width, float xPos, float yPos, float zPos) {
		createPlane(*this, length, width, xPos, yPos, zPos);
	}

	~planeData() {
		vertices.clear();
		colors.clear();
		normals.clear();
		texCoords.clear();
		position.clear();
	}
};
/*=================================================================================================
	SHADERS & TRANSFORMATIONS
=================================================================================================*/

ShaderProgram PassthroughShader;
ShaderProgram PerspectiveShader;

glm::mat4 PerspProjectionMatrix( 1.0f );
glm::mat4 PerspViewMatrix( 1.0f );
glm::mat4 PerspModelMatrix( 1.0f );

float perspZoom = 1.0f, perspSensitivity = 0.35f;
float perspRotationX = 0.0f, perspRotationY = 0.0f;

/*=================================================================================================
	OBJECTS
=================================================================================================*/

//VAO -> the object "as a whole", the collection of buffers that make up its data
//VBOs -> the individual buffers/arrays with data, for ex: one for coordinates, one for color, etc.

GLuint axis_VAO;
GLuint axis_VBO[2];

float axis_vertices[] = {
	//x axis
	-1.0f,  0.0f,  0.0f, 1.0f,
	1.0f,  0.0f,  0.0f, 1.0f,
	//y axis
	0.0f, -1.0f,  0.0f, 1.0f,
	0.0f,  1.0f,  0.0f, 1.0f,
	//z axis
	0.0f,  0.0f, -1.0f, 1.0f,
	0.0f,  0.0f,  1.0f, 1.0f
};

float axis_colors[] = {
	//x axis
	1.0f, 0.0f, 0.0f, 1.0f,
	1.0f, 0.0f, 0.0f, 1.0f,
	//y axis
	0.0f, 1.0f, 0.0f, 1.0f,
	0.0f, 1.0f, 0.0f, 1.0f,
	//z axis
	0.0f, 0.0f, 1.0f, 1.0f,
	0.0f, 0.0f, 1.0f, 1.0f
};


int cubeSize;

std::vector<cubeData> cubes;

void createCube(cubeData& cube, float length, float width, float height, float xPos, float yPos, float zPos)
{
	// Calculate half dimensions
	float halfLength = length / 2.0f;
	float halfWidth = width / 2.0f;
	float halfHeight = height / 2.0f;

	// Vertices
	cube.vertices = {
		// Front face
		glm::vec4(-halfLength + xPos, -halfHeight + yPos, halfWidth + zPos, 1.0f),
		glm::vec4(halfLength + xPos, -halfHeight + yPos, halfWidth + zPos, 1.0f),
		glm::vec4(halfLength + xPos, halfHeight + yPos, halfWidth + zPos, 1.0f),
		glm::vec4(-halfLength + xPos, -halfHeight + yPos, halfWidth + zPos, 1.0f),
		glm::vec4(halfLength + xPos, halfHeight + yPos, halfWidth + zPos, 1.0f),
		glm::vec4(-halfLength + xPos, halfHeight + yPos, halfWidth + zPos, 1.0f),

		// Back face
		glm::vec4(-halfLength + xPos, -halfHeight + yPos, -halfWidth + zPos, 1.0f),
		glm::vec4(-halfLength + xPos, halfHeight + yPos, -halfWidth + zPos, 1.0f),
		glm::vec4(halfLength + xPos, halfHeight + yPos, -halfWidth + zPos, 1.0f),
		glm::vec4(-halfLength + xPos, -halfHeight + yPos, -halfWidth + zPos, 1.0f),
		glm::vec4(halfLength + xPos, halfHeight + yPos, -halfWidth + zPos, 1.0f),
		glm::vec4(halfLength + xPos, -halfHeight + yPos, -halfWidth + zPos, 1.0f),

		// Top face
		glm::vec4(-halfLength + xPos, halfHeight + yPos, -halfWidth + zPos, 1.0f),
		glm::vec4(-halfLength + xPos, halfHeight + yPos, halfWidth + zPos, 1.0f),
		glm::vec4(halfLength + xPos, halfHeight + yPos, halfWidth + zPos, 1.0f),
		glm::vec4(-halfLength + xPos, halfHeight + yPos, -halfWidth + zPos, 1.0f),
		glm::vec4(halfLength + xPos, halfHeight + yPos, halfWidth + zPos, 1.0f),
		glm::vec4(halfLength + xPos, halfHeight + yPos, -halfWidth + zPos, 1.0f),

		// Bottom face
		glm::vec4(-halfLength + xPos, -halfHeight + yPos, -halfWidth + zPos, 1.0f),
		glm::vec4(halfLength + xPos, -halfHeight + yPos, -halfWidth + zPos, 1.0f),
		glm::vec4(halfLength + xPos, -halfHeight + yPos, halfWidth + zPos, 1.0f),
		glm::vec4(-halfLength + xPos, -halfHeight + yPos, -halfWidth + zPos, 1.0f),
		glm::vec4(halfLength + xPos, -halfHeight + yPos, halfWidth + zPos, 1.0f),
		glm::vec4(-halfLength + xPos, -halfHeight + yPos, halfWidth + zPos, 1.0f),

		// Right face
		glm::vec4(halfLength + xPos, -halfHeight + yPos, -halfWidth + zPos, 1.0f),
		glm::vec4(halfLength + xPos, halfHeight + yPos, -halfWidth + zPos, 1.0f),
		glm::vec4(halfLength + xPos, halfHeight + yPos, halfWidth + zPos, 1.0f),
		glm::vec4(halfLength + xPos, -halfHeight + yPos, -halfWidth + zPos, 1.0f),
		glm::vec4(halfLength + xPos, halfHeight + yPos, halfWidth + zPos, 1.0f),
		glm::vec4(halfLength + xPos, -halfHeight + yPos, halfWidth + zPos, 1.0f),

		// Left face
		glm::vec4(-halfLength + xPos, -halfHeight + yPos, -halfWidth + zPos, 1.0f),
		glm::vec4(-halfLength + xPos, -halfHeight + yPos, halfWidth + zPos, 1.0f),
		glm::vec4(-halfLength + xPos, halfHeight + yPos, halfWidth + zPos, 1.0f),
		glm::vec4(-halfLength + xPos, -halfHeight + yPos, -halfWidth + zPos, 1.0f),
		glm::vec4(-halfLength + xPos, halfHeight + yPos, halfWidth + zPos, 1.0f),
		glm::vec4(-halfLength + xPos, halfHeight + yPos, -halfWidth + zPos, 1.0f)
	};

	// Colors, we want all to be white
	for (int i = 0; i < 36; i++)
	{
		cube.colors.push_back(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
	}

	// Calculate normals
	for (int i = 0; i < 36; i += 3)
	{
		glm::vec3 v1 = glm::vec3(cube.vertices[i + 1] - cube.vertices[i]);
		glm::vec3 v2 = glm::vec3(cube.vertices[i + 2] - cube.vertices[i]);
		glm::vec3 normal = glm::normalize(glm::cross(v1, v2));

		cube.normals.push_back(glm::vec4(normal, 0.0f));
		cube.normals.push_back(glm::vec4(normal, 0.0f));
		cube.normals.push_back(glm::vec4(normal, 0.0f));
	}
	cubes.push_back(cube);
}

GLuint plane_VAO;
GLuint plane_VBO[4];

void createPlane(planeData& plane, float length, float width, float xPos, float yPos, float zPos)
{
	// Calculate half dimensions
	float halfLength = length / 2.0f;
	float halfWidth = width / 2.0f;

	// Vertices, we want it to be our floor
	plane.vertices = {
		glm::vec4(-halfLength + xPos, 0.0f + yPos, halfWidth + zPos, 1.0f),
		glm::vec4(halfLength + xPos, 0.0f + yPos, halfWidth + zPos, 1.0f),
		glm::vec4(halfLength + xPos, 0.0f + yPos, -halfWidth + zPos, 1.0f),
		glm::vec4(-halfLength + xPos, 0.0f + yPos, halfWidth + zPos, 1.0f),
		glm::vec4(halfLength + xPos, 0.0f + yPos, -halfWidth + zPos, 1.0f),
		glm::vec4(-halfLength + xPos, 0.0f + yPos, -halfWidth + zPos, 1.0f)
	};
	

	for (int i = 0; i < 6; i++)
	{
		plane.colors.push_back(glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
	}

	// Calculate normals
	for (int i = 0; i < 6; i += 3)
	{
		glm::vec3 v1 = glm::vec3(plane.vertices[i + 1] - plane.vertices[i]);
		glm::vec3 v2 = glm::vec3(plane.vertices[i + 2] - plane.vertices[i]);
		glm::vec3 normal = glm::normalize(glm::cross(v1, v2));

		plane.normals.push_back(glm::vec4(normal, 0.0f));
		plane.normals.push_back(glm::vec4(normal, 0.0f));
		plane.normals.push_back(glm::vec4(normal, 0.0f));
	}
}


/*=================================================================================================
	HELPER FUNCTIONS
=================================================================================================*/

void window_to_scene( int wx, int wy, float& sx, float& sy )
{
	sx = ( 2.0f * (float)wx / WindowWidth ) - 1.0f;
	sy = 1.0f - ( 2.0f * (float)wy / WindowHeight );
}

/*=================================================================================================
	SHADERS
=================================================================================================*/

void CreateTransformationMatrices( void )
{
	// PROJECTION MATRIX
	PerspProjectionMatrix = glm::perspective<float>( glm::radians( 60.0f ), (float)WindowWidth / (float)WindowHeight, 0.01f, 1000.0f );

	// VIEW MATRIX
	glm::vec3 eye   ( 0.0, 0.0, 2.0 );
	glm::vec3 center( 0.0, 0.0, 0.0 );
	glm::vec3 up    ( 0.0, 1.0, 0.0 );

	PerspViewMatrix = glm::lookAt( eye, center, up );

	// MODEL MATRIX
	PerspModelMatrix = glm::mat4( 1.0 );
	PerspModelMatrix = glm::rotate( PerspModelMatrix, glm::radians( perspRotationX ), glm::vec3( 1.0, 0.0, 0.0 ) );
	PerspModelMatrix = glm::rotate( PerspModelMatrix, glm::radians( perspRotationY ), glm::vec3( 0.0, 1.0, 0.0 ) );
	PerspModelMatrix = glm::scale( PerspModelMatrix, glm::vec3( perspZoom ) );
}

void CreateShaders( void )
{
	// Renders without any transformations
	PassthroughShader.Create( "./shaders/simple.vert", "./shaders/simple.frag" );

	// Renders using perspective projection
	PerspectiveShader.Create( "./shaders/persp.vert", "./shaders/persp.frag" );

	//
	// Additional shaders would be defined here
	//
}

/*=================================================================================================
	BUFFERS
=================================================================================================*/

void CreateAxisBuffers( void )
{
	glGenVertexArrays( 1, &axis_VAO ); //generate 1 new VAO, its ID is returned in axis_VAO
	glBindVertexArray( axis_VAO ); //bind the VAO so the subsequent commands modify it

	glGenBuffers( 2, &axis_VBO[0] ); //generate 2 buffers for data, their IDs are returned to the axis_VBO array

	// first buffer: vertex coordinates
	glBindBuffer( GL_ARRAY_BUFFER, axis_VBO[0] ); //bind the first buffer using its ID
	glBufferData( GL_ARRAY_BUFFER, sizeof( axis_vertices ), axis_vertices, GL_STATIC_DRAW ); //send coordinate array to the GPU
	glVertexAttribPointer( 0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof( float ), (void*)0 ); //let GPU know this is attribute 0, made up of 4 floats
	glEnableVertexAttribArray( 0 );

	// second buffer: colors
	glBindBuffer( GL_ARRAY_BUFFER, axis_VBO[1] ); //bind the second buffer using its ID
	glBufferData( GL_ARRAY_BUFFER, sizeof( axis_colors ), axis_colors, GL_STATIC_DRAW ); //send color array to the GPU
	glVertexAttribPointer( 1, 4, GL_FLOAT, GL_FALSE, 4 * sizeof( float ), (void*)0 ); //let GPU know this is attribute 1, made up of 4 floats
	glEnableVertexAttribArray( 1 );

	glBindVertexArray( 0 ); //unbind when done

	//NOTE: You will probably not use an array for your own objects, as you will need to be
	//      able to dynamically resize the number of vertices. Remember that the sizeof()
	//      operator will not give an accurate answer on an entire vector. Instead, you will
	//      have to do a calculation such as sizeof(v[0]) * v.size().
}

void CreateCubeBuffers(void)
{
	cubeData cube;
	createCube(cube, 1.0f, 1.0f, 1.0f, 0, .5, 0);
	cubes.push_back(cube);

	cubeData cube2;
	createCube(cube2, 1.0f, 1.0f, 1.0f, 1, 1, 1);
	cubes.push_back(cube2);

	cubeData cube3;
	createCube(cube3, 1.0f, 1.0f, 1.0f, -1, 1, -1);
	cubes.push_back(cube3);

	// Create VAOs and VBOs for each cube
	for (int i = 0; i < cubes.size(); i++)
	{
		GLuint cubeVAO;
		glGenVertexArrays(1, &cubeVAO);
		glBindVertexArray(cubeVAO);

		GLuint cubeVBO[4];
		glGenBuffers(4, cubeVBO);

		// first buffer: vertex coordinates
		glBindBuffer(GL_ARRAY_BUFFER, cubeVBO[0]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(cubes[i].vertices[0]) * cubes[i].vertices.size(), &cubes[i].vertices[0], GL_STATIC_DRAW);
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);

		// second buffer: colors
		glBindBuffer(GL_ARRAY_BUFFER, cubeVBO[1]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(cubes[i].colors[0]) * cubes[i].colors.size(), &cubes[i].colors[0], GL_STATIC_DRAW);
		glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);

		// third buffer: normals
		glBindBuffer(GL_ARRAY_BUFFER, cubeVBO[2]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(cubes[i].normals[0]) * cubes[i].normals.size(), &cubes[i].normals[0], GL_STATIC_DRAW);
		glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(2);

		// fourth buffer: texture coordinates
		glBindBuffer(GL_ARRAY_BUFFER, cubeVBO[3]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(cubes[i].texCoords[0]) * cubes[i].texCoords.size(), &cubes[i].texCoords[0], GL_STATIC_DRAW);
		glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(3);

		glBindVertexArray(0);

		// Save the VAO and VBO IDs for this cube
		cubes[i].cube_VAO = cubeVAO;
		for (int j = 0; j < 4; j++) {
			cubes[i].cube_VBO[j] = cubeVBO[j];
		}

		cubeSize = cubes[i].vertices.size();
	}
}

void CreatePlaneBuffers(void)
{
	planeData plane;
	createPlane(plane, 10.0f, 10.0f, 0, 0, 0);

	glGenVertexArrays( 1, &plane_VAO );
	glBindVertexArray( plane_VAO );

	glGenBuffers( 4, &plane_VBO[0] );

	// first buffer: vertex coordinates
	glBindBuffer( GL_ARRAY_BUFFER, plane_VBO[0] );
	glBufferData( GL_ARRAY_BUFFER, sizeof( plane.vertices[0] ) * plane.vertices.size(), &plane.vertices[0], GL_STATIC_DRAW );
	glVertexAttribPointer( 0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof( float ), (void*)0 );
	glEnableVertexAttribArray( 0 );

	// second buffer: colors
	glBindBuffer( GL_ARRAY_BUFFER, plane_VBO[1] );
	glBufferData( GL_ARRAY_BUFFER, sizeof( plane.colors[0] ) * plane.colors.size(), &plane.colors[0], GL_STATIC_DRAW );
	glVertexAttribPointer( 1, 4, GL_FLOAT, GL_FALSE, 4 * sizeof( float ), (void*)0 );
	glEnableVertexAttribArray( 1 );

	// third buffer: normals
	glBindBuffer( GL_ARRAY_BUFFER, plane_VBO[2] );
	glBufferData( GL_ARRAY_BUFFER, sizeof( plane.normals[0] ) * plane.normals.size(), &plane.normals[0], GL_STATIC_DRAW );
	glVertexAttribPointer( 2, 4, GL_FLOAT, GL_FALSE, 4 * sizeof( float ), (void*)0 );
	glEnableVertexAttribArray( 2 );

	// fourth buffer: texture coordinates
	glBindBuffer( GL_ARRAY_BUFFER, plane_VBO[3] );
	glBufferData( GL_ARRAY_BUFFER, sizeof( plane.texCoords[0] ) * plane.texCoords.size(), &plane.texCoords[0], GL_STATIC_DRAW );
	glVertexAttribPointer( 3, 2, GL_FLOAT, GL_FALSE, 2 * sizeof( float ), (void*)0 );
	glEnableVertexAttribArray( 3 );

	glBindVertexArray( 0 );
}

/*=================================================================================================
	CALLBACKS
=================================================================================================*/

//-----------------------------------------------------------------------------
// CALLBACK DOCUMENTATION
// https://www.opengl.org/resources/libraries/glut/spec3/node45.html
// http://freeglut.sourceforge.net/docs/api.php#WindowCallback
//-----------------------------------------------------------------------------

void idle_func()
{
	//uncomment below to repeatedly draw new frames
	glutPostRedisplay();
}

void reshape_func( int width, int height )
{
	WindowWidth  = width;
	WindowHeight = height;

	glViewport( 0, 0, width, height );
	glutPostRedisplay();
}

void keyboard_func( unsigned char key, int x, int y )
{
	key_states[ key ] = true;

	switch( key )
	{
		case 'w':
		{
			draw_wireframe = !draw_wireframe;
			if( draw_wireframe == true )
				std::cout << "Wireframes on.\n";
			else
				std::cout << "Wireframes off.\n";
			break;
		}

		// Exit on escape key press
		case '\x1B':
		{
			exit( EXIT_SUCCESS );
			break;
		}
	}
}

void key_released( unsigned char key, int x, int y )
{
	key_states[ key ] = false;
}

void key_special_pressed( int key, int x, int y )
{
	key_special_states[ key ] = true;
}

void key_special_released( int key, int x, int y )
{
	key_special_states[ key ] = false;
}

void mouse_func( int button, int state, int x, int y )
{
	// Key 0: left button
	// Key 1: middle button
	// Key 2: right button
	// Key 3: scroll up
	// Key 4: scroll down

	if( x < 0 || x > WindowWidth || y < 0 || y > WindowHeight )
		return;

	float px, py;
	window_to_scene( x, y, px, py );

	if( button == 3 )
	{
		perspZoom += 0.03f;
	}
	else if( button == 4 )
	{
		if( perspZoom - 0.03f > 0.0f )
			perspZoom -= 0.03f;
	}

	mouse_states[ button ] = ( state == GLUT_DOWN );

	LastMousePosX = x;
	LastMousePosY = y;
}

void passive_motion_func( int x, int y )
{
	if( x < 0 || x > WindowWidth || y < 0 || y > WindowHeight )
		return;

	float px, py;
	window_to_scene( x, y, px, py );

	LastMousePosX = x;
	LastMousePosY = y;
}

void active_motion_func( int x, int y )
{
	if( x < 0 || x > WindowWidth || y < 0 || y > WindowHeight )
		return;

	float px, py;
	window_to_scene( x, y, px, py );

	if( mouse_states[0] == true )
	{
		perspRotationY += ( x - LastMousePosX ) * perspSensitivity;
		perspRotationX += ( y - LastMousePosY ) * perspSensitivity;
	}

	LastMousePosX = x;
	LastMousePosY = y;
}

/*=================================================================================================
	RENDERING
=================================================================================================*/

void display_func( void )
{
	// Clear the contents of the back buffer
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	// Update transformation matrices
	CreateTransformationMatrices();

	// Choose which shader to user, and send the transformation matrix information to it
	PerspectiveShader.Use();
	PerspectiveShader.SetUniform( "projectionMatrix", glm::value_ptr( PerspProjectionMatrix ), 4, GL_FALSE, 1 );
	PerspectiveShader.SetUniform( "viewMatrix", glm::value_ptr( PerspViewMatrix ), 4, GL_FALSE, 1 );
	PerspectiveShader.SetUniform( "modelMatrix", glm::value_ptr( PerspModelMatrix ), 4, GL_FALSE, 1 );

	// Drawing in wireframe?
	if( draw_wireframe == true )
		glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
	else
		glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );

	// Bind the axis Vertex Array Object created earlier, and draw it
	glBindVertexArray( axis_VAO );
	glDrawArrays( GL_LINES, 0, 6 ); // 6 = number of vertices in the object

	//
	// Bind and draw your object here
	//

	for (int i = 0; i < cubes.size(); i++)
	{
		glBindVertexArray(cubes[i].cube_VAO);
		glDrawArrays(GL_TRIANGLES, 0, cubeSize);
	}

	glBindVertexArray( plane_VAO );
	glDrawArrays( GL_TRIANGLES, 0, 6);


	// Unbind when done
	glBindVertexArray( 0 );

	// Swap the front and back buffers
	glutSwapBuffers();
}

/*=================================================================================================
	INIT
=================================================================================================*/

void init( void )
{
	// Print some info
	std::cout << "Vendor:         " << glGetString( GL_VENDOR   ) << "\n";
	std::cout << "Renderer:       " << glGetString( GL_RENDERER ) << "\n";
	std::cout << "OpenGL Version: " << glGetString( GL_VERSION  ) << "\n";
	std::cout << "GLSL Version:   " << glGetString( GL_SHADING_LANGUAGE_VERSION ) << "\n\n";

	// Set OpenGL settings
	glClearColor( 0.0f, 0.0f, 0.0f, 0.0f ); // background color
	glEnable( GL_DEPTH_TEST ); // enable depth test
	glEnable( GL_CULL_FACE ); // enable back-face culling

	// Create shaders
	CreateShaders();

	// Create axis buffers
	CreateAxisBuffers();

	// Create object buffers
	CreateCubeBuffers();
	CreatePlaneBuffers();

	//
	// Consider calling a function to create your object here
	//

	std::cout << "Finished initializing...\n\n";
}

/*=================================================================================================
	MAIN
=================================================================================================*/

int main( int argc, char** argv )
{
	// Create and initialize the OpenGL context
	glutInit( &argc, argv );

	glutInitWindowPosition( 100, 100 );
	glutInitWindowSize( InitWindowWidth, InitWindowHeight );
	glutInitDisplayMode( GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH );

	glutCreateWindow( "CSE-170 Computer Graphics" );

	// Initialize GLEW
	GLenum ret = glewInit();
	if( ret != GLEW_OK ) {
		std::cerr << "GLEW initialization error." << std::endl;
		glewGetErrorString( ret );
		return -1;
	}

	// Register callback functions
	glutDisplayFunc( display_func );
	glutIdleFunc( idle_func );
	glutReshapeFunc( reshape_func );
	glutKeyboardFunc( keyboard_func );
	glutKeyboardUpFunc( key_released );
	glutSpecialFunc( key_special_pressed );
	glutSpecialUpFunc( key_special_released );
	glutMouseFunc( mouse_func );
	glutMotionFunc( active_motion_func );
	glutPassiveMotionFunc( passive_motion_func );

	// Do program initialization
	init();

	// Enter the main loop
	glutMainLoop();

	return EXIT_SUCCESS;
}

void cubeData::createCube(cubeData& cube, float length, float width, float height, float xPos, float yPos, float zPos)
{
}
