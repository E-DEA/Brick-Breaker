#include <iostream>
#include <cmath>
#include <fstream>
#include <vector>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace std;

struct VAO {
    GLuint VertexArrayID;
    GLuint VertexBuffer;
    GLuint ColorBuffer;

    GLenum PrimitiveMode;
    GLenum FillMode;
    int NumVertices;
};
typedef struct VAO VAO;

struct GLMatrices {
	glm::mat4 projection;
	glm::mat4 model;
	glm::mat4 view;
	GLuint MatrixID;
} Matrices;

GLuint programID;

/* Function to load Shaders - Use it as it is */
GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path) {

	// Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	// Read the Vertex Shader code from the file
	std::string VertexShaderCode;
	std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
	if(VertexShaderStream.is_open())
	{
		std::string Line = "";
		while(getline(VertexShaderStream, Line))
			VertexShaderCode += "\n" + Line;
		VertexShaderStream.close();
	}

	// Read the Fragment Shader code from the file
	std::string FragmentShaderCode;
	std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
	if(FragmentShaderStream.is_open()){
		std::string Line = "";
		while(getline(FragmentShaderStream, Line))
			FragmentShaderCode += "\n" + Line;
		FragmentShaderStream.close();
	}

	GLint Result = GL_FALSE;
	int InfoLogLength;

	// Compile Vertex Shader
	printf("Compiling shader : %s\n", vertex_file_path);
	char const * VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
	glCompileShader(VertexShaderID);

	// Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> VertexShaderErrorMessage(InfoLogLength);
	glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
	fprintf(stdout, "%s\n", &VertexShaderErrorMessage[0]);

	// Compile Fragment Shader
	printf("Compiling shader : %s\n", fragment_file_path);
	char const * FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
	glCompileShader(FragmentShaderID);

	// Check Fragment Shader
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> FragmentShaderErrorMessage(InfoLogLength);
	glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
	fprintf(stdout, "%s\n", &FragmentShaderErrorMessage[0]);

	// Link the program
	fprintf(stdout, "Linking program\n");
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> ProgramErrorMessage( max(InfoLogLength, int(1)) );
	glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
	fprintf(stdout, "%s\n", &ProgramErrorMessage[0]);

	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	return ProgramID;
}

static void error_callback(int error, const char* description)
{
    //fprintf(stderr, "Error: %s\n", description);
}

void quit(GLFWwindow *window)
{
    glfwDestroyWindow(window);
    glfwTerminate();
//    exit(EXIT_SUCCESS);
}


/* Generate VAO, VBOs and return VAO handle */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat* color_buffer_data, GLenum fill_mode=GL_FILL)
{
    struct VAO* vao = new struct VAO;
    vao->PrimitiveMode = primitive_mode;
    vao->NumVertices = numVertices;
    vao->FillMode = fill_mode;

    // Create Vertex Array Object
    // Should be done after CreateWindow and before any other GL calls
    glGenVertexArrays(1, &(vao->VertexArrayID)); // VAO
    glGenBuffers (1, &(vao->VertexBuffer)); // VBO - vertices
    glGenBuffers (1, &(vao->ColorBuffer));  // VBO - colors

    glBindVertexArray (vao->VertexArrayID); // Bind the VAO 
    glBindBuffer (GL_ARRAY_BUFFER, vao->VertexBuffer); // Bind the VBO vertices 
    glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), vertex_buffer_data, GL_STATIC_DRAW); // Copy the vertices into VBO
    glVertexAttribPointer(
                          0,                  // attribute 0. Vertices
                          3,                  // size (x,y,z)
                          GL_FLOAT,           // type
                          GL_FALSE,           // normalized?
                          0,                  // stride
                          (void*)0            // array buffer offset
                          );

    glBindBuffer (GL_ARRAY_BUFFER, vao->ColorBuffer); // Bind the VBO colors 
    glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), color_buffer_data, GL_STATIC_DRAW);  // Copy the vertex colors
    glVertexAttribPointer(
                          1,                  // attribute 1. Color
                          3,                  // size (r,g,b)
                          GL_FLOAT,           // type
                          GL_FALSE,           // normalized?
                          0,                  // stride
                          (void*)0            // array buffer offset
                          );

    return vao;
}

/* Generate VAO, VBOs and return VAO handle - Common Color for all vertices */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat red, const GLfloat green, const GLfloat blue, GLenum fill_mode=GL_FILL)
{
    GLfloat* color_buffer_data = new GLfloat [3*numVertices];
    for (int i=0; i<numVertices; i++) {
        color_buffer_data [3*i] = red;
        color_buffer_data [3*i + 1] = green;
        color_buffer_data [3*i + 2] = blue;
    }

    return create3DObject(primitive_mode, numVertices, vertex_buffer_data, color_buffer_data, fill_mode);
}

/* Render the VBOs handled by VAO */
void draw3DObject (struct VAO* vao)
{
    // Change the Fill Mode for this object
    glPolygonMode (GL_FRONT_AND_BACK, vao->FillMode);

    // Bind the VAO to use
    glBindVertexArray (vao->VertexArrayID);

    // Enable Vertex Attribute 0 - 3d Vertices
    glEnableVertexAttribArray(0);
    // Bind the VBO to use
    glBindBuffer(GL_ARRAY_BUFFER, vao->VertexBuffer);

    // Enable Vertex Attribute 1 - Color
    glEnableVertexAttribArray(1);
    // Bind the VBO to use
    glBindBuffer(GL_ARRAY_BUFFER, vao->ColorBuffer);

    // Draw the geometry !
    glDrawArrays(vao->PrimitiveMode, 0, vao->NumVertices); // Starting from vertex 0; 3 vertices total -> 1 triangle
}

/**************************
 * Customizable functions *
 **************************/

float triangle_rot_dir = 1;
float rectangle_rot_dir = 1;
bool triangle_rot_status = true;
bool rectangle_rot_status = true;
float tr_x[1001];
float tr_y[1001];
float tr_z[1001];
float rb_x[501];
float rb_y[501];
float rb_z[501];
float gb_x[501];
float gb_y[501];
float gb_z[501];
float red_x = -1.0f;
float green_x = 1.0f;
float speed = 1.0;
bool firestatus[10];
bool chargestatus = true;
float beamx[10],beamy[10], beamz[10];
float laserx = -4.0f, lasery, laserz;
float laserangle = 0; 
float beamangle[10];
double mytime;
int currlaser = 0;
int score = 0;
int lives = 5;
int objectsel = 0;
double xpos, ypos;

/* Executed when a regular key is pressed/released/held-down */
/* Prefered for Keyboard events */

/* Executed for character input (like in text boxes) */
void keyboardChar (GLFWwindow* window, unsigned int key)
{
	switch (key) {
		case 'Q':
		case 'q':
            quit(window);
            break;
		default:
			break;
	}
}


/* Executed when a mouse button is pressed/released */
void mouseButton (GLFWwindow* window, int button, int action, int mods)
{
   	switch (button) 
   	{
        case GLFW_MOUSE_BUTTON_LEFT:
            if (action == GLFW_PRESS and xpos > 0 and xpos < 100 and ypos > fabs(lasery + 0.5 - 4) * 100 and ypos < fabs(lasery - 0.5 - 4)*100 and lasery < 3.0)lasery += 0.2;
            break;
   	    case GLFW_MOUSE_BUTTON_RIGHT:
            if (action == GLFW_PRESS and xpos > 0 and xpos < 100 and ypos > fabs(lasery + 0.5 - 4) * 100 and ypos < fabs(lasery - 0.5 - 4)*100 and lasery > -2.8)lasery -= 0.2;
            break;
        default:
            break;
    }
}

void scroll (GLFWwindow* window, double xoffset, double yoffset)
{
	if(yoffset > 0 and laserangle < 45 and xpos > 0 and xpos < 100 and ypos > (fabs(lasery + 0.5 - 4) * 100) and ypos < fabs(lasery - 0.5 - 4)*100)
	{
		laserangle += 9;
	}
	else if(yoffset < 0 and laserangle > -45 and xpos > 0 and xpos < 100 and ypos > fabs(lasery + 0.5 - 4) * 100 and ypos < fabs(lasery - 0.5 - 4)*100)
	{
		laserangle -= 9;
	}
	else if(yoffset < 0 and red_x > -2 and xpos < fabs(red_x + 4)*100 + 38 and xpos > fabs(red_x + 4)*100 - 38 and ypos > (3.278 + 4)*100 - 32 and ypos < (3.278 + 4)*100 + 32)red_x -= 0.05;
	else if(yoffset < 0 and green_x > -2 and xpos < fabs(green_x + 4)*100 + 38 and xpos > fabs(green_x + 4)*100 - 38 and ypos > (3.278 + 4)*100 - 32 and ypos < (3.278 + 4)*100 + 32)green_x -= 0.05;
	else if(yoffset > 0 and red_x < 2 and xpos < fabs(red_x + 4)*100 + 38 and xpos > fabs(red_x + 4)*100 - 38 and ypos > (3.278 + 4)*100 - 32 and ypos < (3.278 + 4)*100 + 32)red_x += 0.05;
    else if(yoffset > 0 and green_x < 2 and xpos < fabs(green_x + 4)*100 + 38 and xpos > fabs(green_x + 4)*100 - 38 and ypos > (3.278 + 4)*100 - 32 and ypos < (3.278 + 4)*100 + 32)green_x += 0.05;
}

void cursor_pos_callback(GLFWwindow* window, double xcoord, double ycoord)
{
	xpos = xcoord;
	ypos = ycoord;	
}


/* Executed when window is resized to 'width' and 'height' */
/* Modify the bounds of the screen here in glm::ortho or Field of View in glm::Perspective */
void reshapeWindow (GLFWwindow* window, int width, int height)
{
    int fbwidth=width, fbheight=height;
    /* With Retina display on Mac OS X, GLFW's FramebufferSize
     is different from WindowSize */
    glfwGetFramebufferSize(window, &fbwidth, &fbheight);

	GLfloat fov = 90.0f;

	// sets the viewport of openGL renderer
	glViewport (0, 0, (GLsizei) fbwidth, (GLsizei) fbheight);

	// set the projection matrix as perspective
	/* glMatrixMode (GL_PROJECTION);
	   glLoadIdentity ();
	   gluPerspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1, 500.0); */
	// Store the projection matrix in a variable for future use
    // Perspective projection for 3D views
    // Matrices.projection = glm::perspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1f, 500.0f);

    // Ortho projection for 2D views
    Matrices.projection = glm::ortho(-4.0f, 4.0f, -4.0f, 4.0f, 0.1f, 500.0f);
}

VAO *mirror,*triangle, *rectangle, *barside, *barfloor, *redbucket, *greenbucket, *redrectangle, *greenrectangle, *beam, *turret;

// Creates the triangle object used in this sample code
void createTriangle ()
{
  /* ONLY vertices between the bounds specified in glm::ortho will be visible on screen */

  /* Define vertex array as used in glBegin (GL_TRIANGLES) */
  static const GLfloat vertex_buffer_data [] = {
    0, 0,0, // vertex 0
    0.25,0.433,0, // vertex 1
    0.5,0,0, // vertex 2
  };

  static const GLfloat color_buffer_data [] = {
    0,0.5,1, // color 0
    0,0.5,1, // color 1
    0,0.5,1, // color 2
  };

  // create3DObject creates and returns a handle to a VAO that can be used later
  triangle = create3DObject(GL_TRIANGLES, 3, vertex_buffer_data, color_buffer_data, GL_LINE);
}

// Creates the rectangle object used in this sample code
void createRectangle ()
{
  // GL3 accepts only Triangles. Quads are not supported
  static const GLfloat vertex_buffer_data [] = {
    -0.15,-0.3,0, // vertex 1
    0.15,-0.3,0, // vertex 2
    0.15, 0.3,0, // vertex 3

    0.15, 0.3,0, // vertex 3
    -0.15, 0.3,0, // vertex 4
    -0.15,-0.3,0  // vertex 1
  };

  static const GLfloat color_buffer_data [] = {
    0.2,0.2,0.2, // color 1
    0.2,0.2,0.2, // color 2
    0.2,0.2,0.2, // color 3

    0.2,0.2,0.2, // color 3
    0.2,0.2,0.2, // color 4
    0.2,0.2,0.2  // color 1
  };

  // create3DObject creates and returns a handle to a VAO that can be used later
  rectangle = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}

void createRectangle_r ()
{
  // GL3 accepts only Triangles. Quads are not supported
  static const GLfloat vertex_buffer_data [] = {
    -0.15,-0.3,0, // vertex 1
    0.15,-0.3,0, // vertex 2
    0.15, 0.3,0, // vertex 3

    0.15, 0.3,0, // vertex 3
    -0.15, 0.3,0, // vertex 4
    -0.15,-0.3,0  // vertex 1
  };

  static const GLfloat color_buffer_data [] = {
    0.8,0.1,0.1, // color 1
    0.8,0.1,0.1, // color 2
    0.8,0.1,0.1, // color 3

    0.8,0.1,0.1, // color 3
    0.8,0.1,0.1, // color 4
    0.8,0.1,0.1  // color 1
  };

  // create3DObject creates and returns a handle to a VAO that can be used later
  redrectangle = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}

void createRectangle_g ()
{
  // GL3 accepts only Triangles. Quads are not supported
  static const GLfloat vertex_buffer_data [] = {
    -0.15,-0.3,0, // vertex 1
    0.15,-0.3,0, // vertex 2
    0.15, 0.3,0, // vertex 3

    0.15, 0.3,0, // vertex 3
    -0.15, 0.3,0, // vertex 4
    -0.15,-0.3,0  // vertex 1
  };

  static const GLfloat color_buffer_data [] = {
    0.1,0.8,0.1, // color 1
    0.1,0.8,0.1, // color 2
    0.1,0.8,0.1, // color 3

    0.1,0.8,0.1, // color 3
    0.1,0.8,0.1, // color 4
    0.1,0.8,0.1  // color 1
  };

  // create3DObject creates and returns a handle to a VAO that can be used later
  greenrectangle = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}
void createfloor ()
{
  // GL3 accepts only Triangles. Quads are not supported
  static const GLfloat vertex_buffer_data [] = {
    -4,-0.4,0, // vertex 1
    4,-0.4,0, // vertex 2
    4, 0.4,0, // vertex 3

    4, 0.4,0, // vertex 3
    -4, 0.4,0, // vertex 4
    -4,-0.4,0  // vertex 1
  };

  static const GLfloat color_buffer_data [] = {
    0.45,0.45,0.45, // color 1
    0.45,0.45,0.45, // color 2
    0.45,0.45,0.45, // color 3

    0.45,0.45,0.45, // color 3
    0.45,0.45,0.45, // color 4
    0.45,0.45,0.45  // color 1
  };

  // create3DObject creates and returns a handle to a VAO that can be used later
  barfloor = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}

void createwall ()
{
  // GL3 accepts only Triangles. Quads are not supported
  static const GLfloat vertex_buffer_data [] = {
    -0.4,-4,0, // vertex 1
    0.4,-4,0, // vertex 2
    0.4,4,0, // vertex 3

    0.4,4,0, // vertex 3
    -0.4,4,0, // vertex 4
    -0.4,-4,0  // vertex 1
  };

  static const GLfloat color_buffer_data [] = {
    0.45,0.45,0.45, // color 1
    0.45,0.45,0.45, // color 2
    0.45,0.45,0.45, // color 3

    0.45,0.45,0.45, // color 3
    0.45,0.45,0.45, // color 4
    0.45,0.45,0.45  // color 1
  };

  // create3DObject creates and returns a handle to a VAO that can be used later
  barside = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}

void createredbucket()
{
	// GL3 accepts only Triangles. Quads are not supported
  static const GLfloat vertex_buffer_data [] = {
    -0.38,-0.32,0, // vertex 1
    0.38,-0.32,0, // vertex 2
    0.38, 0.32,0, // vertex 3

    0.38, 0.32,0, // vertex 3
    -0.38, 0.32,0, // vertex 4
    -0.38,-0.32,0  // vertex 1
  };

  static const GLfloat color_buffer_data [] = {
    0.8,0.1,0.1,// color 1
    0.8,0.1,0.1, // color 2
    0.8,0.1,0.1, // color 3

    0.8,0.1,0.1, // color 3
    0.8,0.1,0.1, // color 4
    0.8,0.1,0.1  // color 1
  };

  // create3DObject creates and returns a handle to a VAO that can be used later
  redbucket = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);	
}

void creategreenbucket()
{
	// GL3 accepts only Triangles. Quads are not supported
  static const GLfloat vertex_buffer_data [] = {
    -0.38,-0.32,0, // vertex 1
    0.38,-0.32,0, // vertex 2
    0.38, 0.32,0, // vertex 3

    0.38, 0.32,0, // vertex 3
    -0.38, 0.32,0, // vertex 4
    -0.38,-0.32,0  // vertex 1
  };

  static const GLfloat color_buffer_data [] = {
    0.1,0.8,0.1,// color 1
    0.1,0.8,0.1, // color 2
    0.1,0.8,0.1, // color 3

    0.1,0.8,0.1, // color 3
    0.1,0.8,0.1, // color 4
    0.1,0.8,0.1  // color 1
  };

  // create3DObject creates and returns a handle to a VAO that can be used later
  greenbucket = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);	
}

void createbeam()
{
	// GL3 accepts only Triangles. Quads are not supported
  static const GLfloat vertex_buffer_data [] = {
    -0.18,-0.06,0, // vertex 1
    0.18,-0.06,0, // vertex 2
    0.18, 0.06,0, // vertex 3

    0.18, 0.06,0, // vertex 3
    -0.18, 0.06,0, // vertex 4
    -0.18,-0.06,0  // vertex 1
  };

  static const GLfloat color_buffer_data [] = {
    0.35,0.75,1,// color 1
    0.35,0.75,1, // color 2
    0.35,0.75,1, // color 3

    0.35,0.75,1, // color 3
    0.35,0.75,1, // color 4
    0.35,0.75,1  // color 1
  };

  // create3DObject creates and returns a handle to a VAO that can be used later
  beam = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);	
}

void createturret()
{
	// GL3 accepts only Triangles. Quads are not supported
  static const GLfloat vertex_buffer_data [] = {
    -0.75,-0.09,0, // vertex 1
    0.75,-0.09,0, // vertex 2
    0.75, 0.09,0, // vertex 3

    0.75, 0.09,0, // vertex 3
    -0.75, 0.09,0, // vertex 4
    -0.75,-0.09,0  // vertex 1
  };

  static const GLfloat color_buffer_data [] = {
    0,0.5,1, // color 1
    0,0.5,1, // color 2
    0,0.5,1, // color 3

	0,0.5,1, // color 3
    0,0.5,1, // color 4
    0,0.5,1 // color 1
  };

  // create3DObject creates and returns a handle to a VAO that can be used later
  turret = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);	
}

void createmirror()
{
	// GL3 accepts only Triangles. Quads are not supported
  static const GLfloat vertex_buffer_data [] = {
    -0.7,-0.02,0, // vertex 1
    0.7,-0.02,0, // vertex 2
    0.7, 0.02,0, // vertex 3

    0.7, 0.02,0, // vertex 3
    -0.7, 0.02,0, // vertex 4
    -0.7,-0.02,0  // vertex 1
  };

  static const GLfloat color_buffer_data [] = {
    0.7,0.7,0.7, // color 1
    0.7,0.7,0.7, // color 2
    0.7,0.7,0.7, // color 3

	0.7,0.7,0.7, // color 3
    0.7,0.7,0.7, // color 4
    0.7,0.7,0.7 // color 1
  };

  // create3DObject creates and returns a handle to a VAO that can be used later
  mirror = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);	
}

float camera_rotation_angle = 90;
float triangle_rotation = 0;
float rectangle_rotation = 0;

glm::vec3 eye ( 0.0f , 0.0f, 3.0f);
// Target - Where is the camera looking at.  Don't change unless you are sure!!
glm::vec3 target (0.0f, 0.0f, -1.0f);
// Up - Up vector defines tilt of camera.  Don't change unless you are sure!!
glm::vec3 up (0.0f, 1.0f, 0.0f);

/* Render the scene with openGL */
/* Edit this function according to your assignment */

void print(int sc, int lives)
{
	cout << "SCORE: " << sc << endl;
	cout << "LIVES LEFT: " << lives << endl;
	cout << "\n" << endl;
}

void draw ()
{
  // clear the color and depth in the frame buffer
  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // use the loaded shader program
  // Don't change unless you know what you are doing
  glUseProgram (programID);

  // Eye - Location of camera. Don't change unless you are sure!!
  //glm::vec3 eye ( 0.0f , 0.0f, 3.0f);
  // Target - Where is the camera looking at.  Don't change unless you are sure!!
  //glm::vec3 target (0.0f, 0.0f, -1.0f);
  // Up - Up vector defines tilt of camera.  Don't change unless you are sure!!
  //glm::vec3 up (0.0f, 1.0f, 0.0f);

  // Compute Camera matrix (view)
  Matrices.view = glm::lookAt( eye , eye + target, up ); // Rotating Camera for 3D
  //  Don't change unless you are sure!!
  //Matrices.view = glm::lookAt(glm::vec3(0,0,3), glm::vec3(0,0,0), glm::vec3(0,1,0)); // Fixed camera for 2D (ortho) in XY plane

  // Compute ViewProject matrix as view/camera might not be changed for this frame (basic scenario)
  //  Don't change unless you are sure!!
  glm::mat4 VP = Matrices.projection * Matrices.view;

  // Send our transformation to the currently bound shader, in the "MVP" uniform
  // For each model you render, since the MVP will be different (at least the M part)
  //  Don't change unless you are sure!!
  glm::mat4 MVP;	// MVP = Projection * View * Model

  // Load identity to model matrix

  Matrices.model = glm::mat4(1.0f);

  glm::mat4 translatemirror = glm::translate (glm::vec3(3.4f, 2.4f, 0.0f));        // glTranslatef
  glm::mat4 rotatemirror = glm::rotate((float)(135*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
  Matrices.model *= (translatemirror * rotatemirror);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  // draw3DObject draws the VAO given to it using current MVP matrix
  draw3DObject(mirror);

  Matrices.model = glm::mat4(1.0f);

  translatemirror = glm::translate (glm::vec3(3.4f, -2.0f, 0.0f));        // glTranslatef
  rotatemirror = glm::rotate((float)(45*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
  Matrices.model *= (translatemirror * rotatemirror);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  // draw3DObject draws the VAO given to it using current MVP matrix
  draw3DObject(mirror);

  for (int lno = 0; lno < 10; lno ++)
  {
  	Matrices.model = glm::mat4(1.0f);

  	glm::mat4 translatebeam = glm::translate (glm::vec3(beamx[lno], beamy[lno], beamz[lno]));        // glTranslatef
  	glm::mat4 rotatebeam = glm::rotate((float)(beamangle[lno]*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
  	Matrices.model *= (translatebeam * rotatebeam);
  	MVP	 = VP * Matrices.model;
  	glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  	// draw3DObject draws the VAO given to it using current MVP matrix
  	if(firestatus[lno] == true)
  	{
  		if((beamy[lno] - 0.06 - ((beamx[lno] + 0.18)*tan(45*M_PI/180.0f) + (-2.0 - 3.4*tan(45*M_PI/180.0f))) < 0.01) and (beamx[lno] + 0.18 >= 3.4 - 0.7*cos(45*M_PI/180.0f)) and (beamy[lno] + 0.06 >= -2 - 0.7*sin(45*M_PI/180.0f)))
  		{
  			beamangle[lno] = 2*45 - beamangle[lno];
  			beamy[lno] += 0.18 * sin(beamangle[lno]*M_PI/180.0f) + 0.06 * cos(beamangle[lno]*M_PI/180.0f);
  			beamx[lno] += 0.18 * cos(beamangle[lno]*M_PI/180.0f) + 0.06 * sin(beamangle[lno]*M_PI/180.0f);
  		}
  		if(((2.4- beamy[lno]) - tan(135*M_PI/180.0f) * (3.4 - beamx[lno]) <= 0.001) and (beamx[lno]>3.4 - 0.7*cos(45*M_PI/180.0f)) and (beamx[lno] < 3.4 + 0.7*cos(45*M_PI/180.0f)) and (beamy[lno] < 2.4 + 0.7*sin(45*M_PI/180.0f)) and ((beamy[lno] > 2.4 - 0.7*sin(45*M_PI/180.0f))))
  		{
  			beamangle[lno] = 2*45 + 180 - beamangle[lno];
  			beamy[lno] += 0.18 * sin(beamangle[lno]*M_PI/180.0f) + 0.06 * cos(beamangle[lno]*M_PI/180.0f);
  			beamx[lno] += 0.18 * cos(beamangle[lno]*M_PI/180.0f) + 0.06 * sin(beamangle[lno]*M_PI/180.0f);
  		}
  		for(int j=0; j<=1000; j++)
  		{
  			if(tr_y[j] >= -3.9 and tr_y[j] <= 3.9)
  			{
  				if(beamx[lno] - 0.16 <= tr_x[j] + 0.16 and beamx[lno] + 0.16 >= tr_x[j] - 0.13 and beamy[lno] - 0.04 <= tr_y[j] + 0.28 and beamy[lno] + 0.04 >= tr_y[j] - 0.28)
  				{
  					beamx[lno] = laserx;
  					beamy[lno] = lasery;
  					firestatus[lno] = false;
  					tr_y[j] = tr_y[1000] + 5*j;
  					score += 100;
  					print(score, lives);
  				}
  			}
  		}
  		
  		for(int j=0; j<=500; j++)
  		{
  			if(rb_y[j] >= -3.9 and rb_y[j] <= 3.9)
  			{
  				if(beamx[lno] - 0.16 <= rb_x[j] + 0.16 and beamx[lno] + 0.16 >= rb_x[j] - 0.13 and beamy[lno] - 0.04 <= rb_y[j] + 0.28 and beamy[lno] + 0.04 >= rb_y[j] - 0.28)
  				{
  					beamx[lno] = laserx;
  					beamy[lno] = lasery;
  					firestatus[lno] = false;
  					rb_y[j] = rb_y[500] + 7*j;
  					score -= 10;
  					print(score, lives);
  				}
  			}
  			if(gb_y[j] >= -3.9 and gb_y[j] <= 3.9)
  			{
  				if(beamx[lno] - 0.16 <= gb_x[j] + 0.16 and beamx[lno] + 0.16 >= gb_x[j] - 0.13 and beamy[lno] - 0.04 <= gb_y[j] + 0.28 and beamy[lno] + 0.04 >= gb_y[j] - 0.28)
  				{
  					beamx[lno] = laserx;
  					beamy[lno] = lasery;
  					firestatus[lno] = false;
  					gb_y[j] = gb_y[500] + 7*j;
  					score -= 10;
  					print(score, lives);
  				}
  			}
  		}
  		if(firestatus[lno] == true)
  		{
  			beamx[lno] += 0.15 * cos(beamangle[lno]*M_PI/180.0f);
  			beamy[lno] += 0.15 * sin(beamangle[lno]*M_PI/180.0f);
  		}
  	}
  	else if(firestatus[lno] == false)
  	{
  		beamangle[lno] = laserangle;
  		beamx[lno] = laserx;
  		beamy[lno] = lasery;
  	}
  	if(beamx[lno] >= 3.9 || beamy[lno] >= 3.9 || beamy[lno] <= -3.5)
  	{
  		beamx[lno] = laserx;
  		beamy[lno] = lasery;
  		firestatus[lno] = false;
  	}
  	draw3DObject(beam);

  }


  Matrices.model = glm::mat4(1.0f);

  glm::mat4 translatebeam = glm::translate (glm::vec3(laserx, lasery, laserz));        // glTranslatef
  glm::mat4 rotatebeam = glm::rotate((float)(laserangle*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
  Matrices.model *= (translatebeam * rotatebeam);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  // draw3DObject draws the VAO given to it using current MVP matrix

  draw3DObject(turret);



  for(int j=0; j<300; j++)
  {
  	Matrices.model = glm::mat4(1.0f);

  	/* Render your scene */

  	glm::mat4 translateTriangle = glm::translate (glm::vec3(laserx, lasery, laserz)); // glTranslatef
  	glm::mat4 rotateTriangle = glm::rotate((float)(10*j*M_PI/1800.0f), glm::vec3(0,0,1));  // rotate about vector (1,0,0)
  	glm::mat4 triangleTransform = translateTriangle * rotateTriangle;
  	Matrices.model *= triangleTransform; 
  	MVP = VP * Matrices.model; // MVP = p * V * M

  	//  Don't change unless you are sure!!
  	glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  	// draw3DObject draws the VAO given to it using current MVP matrix
  	draw3DObject(triangle);
  }

  // Pop matrix to undo transformations till last push matrix instead of recomputing model matrix
  // glPopMatrix ();

  for (int i = 0; i <= 1000; i++)
  {

  	if(tr_y[i] <= -2.7 and ((tr_x[i] >= green_x - 0.38  && tr_x[i] <= green_x + 0.38) || (tr_x[i] >= red_x - 0.38 && tr_x[i] <= red_x + 0.38)))
  	{
  		tr_y[i] = tr_y[1000] + 5*i;
  		lives--;
  		cout << "Life Lost :(" << endl;
  		print(score, lives);
  	}

  	if(tr_y[i] <= -3.4)
  	{
  		tr_y[i] = tr_y[1000] + 5*i;
  		score -= 10;
  		print(score, lives);
  	}

  	Matrices.model = glm::mat4(1.0f);

  	glm::mat4 translateRectangle = glm::translate (glm::vec3(tr_x[i], tr_y[i], tr_z[i]));        // glTranslatef
  	glm::mat4 rotateRectangle = glm::rotate((float)(rectangle_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
  	Matrices.model *= (translateRectangle * rotateRectangle);
  	MVP = VP * Matrices.model;
  	glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  	// draw3DObject draws the VAO given to it using current MVP matrix
  	draw3DObject(rectangle);
  	tr_y[i] -= 0.015 * speed;
  }

  for (int i = 0; i <= 500; i++)
  {

  	if(rb_y[i] <= -2.7 and (rb_x[i] >= red_x - 0.38  && rb_x[i] <= red_x + 0.38)) 
  	{
  		rb_y[i] = rb_y[500] + 7*i;
  		score += 100;
  		print(score, lives);
  	}
  	else if(rb_y[i] <= -2.7 and (rb_x[i] >= green_x - 0.38  && rb_x[i] <= green_x + 0.38))
  	{
  		rb_y[i] = rb_y[500] + 7*i;
  		score -= 10;
  		print(score, lives);
  	}

	if(rb_y[i] <= -3.4) rb_y[i] = rb_y[500] + 7*i;

  	Matrices.model = glm::mat4(1.0f);

  	glm::mat4 translateRectangle = glm::translate (glm::vec3(rb_x[i], rb_y[i], rb_z[i]));        // glTranslatef
  	glm::mat4 rotateRectangle = glm::rotate((float)(rectangle_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
  	Matrices.model *= (translateRectangle * rotateRectangle);
  	MVP = VP * Matrices.model;
  	glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  	// draw3DObject draws the VAO given to it using current MVP matrix
  	draw3DObject(redrectangle);
  	rb_y[i] -= 0.015 * speed;
  }

  for (int i = 0; i <= 500; i++)
  {

  	if(gb_y[i] <= -2.7 and (gb_x[i] >= green_x - 0.38  && gb_x[i] <= green_x + 0.38)) 
  	{
  		gb_y[i] = gb_y[500] + 7*i;
  		score += 100;
  		print(score, lives);
  	}
  	else if(gb_y[i] <= -2.7 and (gb_x[i] >= red_x - 0.38  && gb_x[i] <= red_x + 0.38)) 
  	{
  		gb_y[i] = gb_y[500] + 7*i;
  		score -= 10;
  		print(score, lives);
  	}

  	if(gb_y[i] <= -3.4) gb_y[i] = gb_y[500] + 7*i;

  	Matrices.model = glm::mat4(1.0f);

  	glm::mat4 translateRectangle = glm::translate (glm::vec3(gb_x[i], gb_y[i], gb_z[i]));        // glTranslatef
  	glm::mat4 rotateRectangle = glm::rotate((float)(rectangle_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
  	Matrices.model *= (translateRectangle * rotateRectangle);
  	MVP = VP * Matrices.model;
  	glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  	// draw3DObject draws the VAO given to it using current MVP matrix
  	draw3DObject(greenrectangle);
  	gb_y[i] -= 0.015 * speed;
  }  
  
  Matrices.model = glm::mat4(1.0f);

  glm::mat4 translatefloor = glm::translate (glm::vec3(0.0f, -4.0f, 0.0f));        // glTranslatef
  glm::mat4 rotatefloor = glm::rotate((float)(rectangle_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
  Matrices.model *= (translatefloor * rotatefloor);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  // draw3DObject draws the VAO given to it using current MVP matrix
  draw3DObject(barfloor);

  Matrices.model = glm::mat4(1.0f);

  glm::mat4 translatebucket = glm::translate (glm::vec3(red_x, -3.278f, 0.0f));        // glTranslatef
  glm::mat4 rotatebucket = glm::rotate((float)(rectangle_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
  Matrices.model *= (translatebucket * rotatebucket);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  // draw3DObject draws the VAO given to it using current MVP matrix
  draw3DObject(redbucket);

  Matrices.model = glm::mat4(1.0f);

  translatebucket = glm::translate (glm::vec3(green_x, -3.278f, 0.0f));        // glTranslatef
  rotatebucket = glm::rotate((float)(rectangle_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
  Matrices.model *= (translatebucket * rotatebucket);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  // draw3DObject draws the VAO given to it using current MVP matrix
  draw3DObject(greenbucket);

  Matrices.model = glm::mat4(1.0f);

  glm::mat4 translatewall = glm::translate (glm::vec3(-4.3f, 0.0f, 0.0f));        // glTranslatef
  glm::mat4 rotatewall = glm::rotate((float)(rectangle_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
  Matrices.model *= (translatewall * rotatewall);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  // draw3DObject draws the VAO given to it using current MVP matrix
  draw3DObject(barside);

  // Increment angles
  //float increments = 1;

  //camera_rotation_angle++; // Simulating camera rotation
  //triangle_rotation = triangle_rotation + increments*triangle_rot_dir*triangle_rot_status;
  //rectangle_rotation = rectangle_rotation + increments*rectangle_rot_dir*rectangle_rot_status;
}

void keyboard (GLFWwindow* window, int key, int scancode, int action, int mods)
{
     // Function is called first on GLFW_PRESS.

    if (action == GLFW_REPEAT or action == GLFW_PRESS) {
        switch (key) {
            case (GLFW_KEY_RIGHT):
            	if(red_x < 2 && glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS) red_x += 0.05;
            	if(green_x < 2 && glfwGetKey(window, GLFW_KEY_RIGHT_ALT) == GLFW_PRESS) green_x += 0.05;
                break;
            case GLFW_KEY_LEFT:
                if(red_x > -2 && glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS) red_x -= 0.05;
                if(green_x > -2 && glfwGetKey(window, GLFW_KEY_RIGHT_ALT) == GLFW_PRESS) green_x -= 0.05;
                break;
            case GLFW_KEY_M:
            	if(speed < 10) speed += 1;
            	break;
            case GLFW_KEY_N:
            	if(speed > 1) speed -= 1;
            	break;
            case GLFW_KEY_A:
            	if(laserangle < 45) laserangle += 9;
            	break;
            case GLFW_KEY_D:
            	if(laserangle > -45) laserangle -= 9;
                break;
            case GLFW_KEY_SPACE:
            	if(chargestatus == true && glfwGetTime() - mytime >=1.0)
            		{
            			firestatus[currlaser] = true;
            			currlaser++;
            			if (currlaser == 10) currlaser = 0;
            			mytime = glfwGetTime();
            		}
            	break;
            case GLFW_KEY_W:
            	if(lasery < 3.0)
            		lasery+=0.5;
            	break;
            case GLFW_KEY_S:
            	if(lasery > -2.8)
            		lasery-=0.5;
            	break;
            default:
                break;
        }
    }
    else if (action == GLFW_PRESS) {
        switch (key) {
            case GLFW_KEY_ESCAPE:
                quit(window);
                break;
            default:
                break;
        }
    }
    GLfloat cameraSpeed = 0.05f;
    if(key == GLFW_KEY_8)
        eye += cameraSpeed * target;
    if(key == GLFW_KEY_2)
        eye -= cameraSpeed * target;
    if(key == GLFW_KEY_4)
        eye -= glm::normalize(glm::cross(target, up)) * cameraSpeed;
    if(key == GLFW_KEY_6)
       	eye += glm::normalize(glm::cross(target, up)) * cameraSpeed;  
}

/* Initialise glfw window, I/O callbacks and the renderer to use */
/* Nothing to Edit here */
GLFWwindow* initGLFW (int width, int height)
{
    GLFWwindow* window; // window desciptor/handle

    glfwSetErrorCallback(error_callback);
    if (!glfwInit()) {
//        exit(EXIT_FAILURE);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(width, height, "Brick Breaker", NULL, NULL);

    if (!window) {
        glfwTerminate();
//        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
    glfwSwapInterval( 1 );

    /* --- register callbacks with GLFW --- */

    /* Register function to handle window resizes */
    /* With Retina display on Mac OS X GLFW's FramebufferSize
     is different from WindowSize */
    glfwSetFramebufferSizeCallback(window, reshapeWindow);
    glfwSetWindowSizeCallback(window, reshapeWindow);

    /* Register function to handle window close */
    glfwSetWindowCloseCallback(window, quit);

    /* Register function to handle keyboard input */
    glfwSetKeyCallback(window, keyboard);
    glfwSetScrollCallback(window, scroll);      // general keyboard input
    glfwSetCharCallback(window, keyboardChar);  // simpler specific character handling
    glfwSetCursorPosCallback(window, cursor_pos_callback);

    /* Register function to handle mouse click */
    glfwSetMouseButtonCallback(window, mouseButton);  // mouse button clicks

    return window;
}

/* Initialize the OpenGL rendering properties */
/* Add all the models to be created here */
void initGL (GLFWwindow* window, int width, int height)
{
    /* Objects should be created before any other gl function and shaders */
	// Create the models
	createTriangle (); // Generate the VAO, VBOs, vertices data & copy into the array buffer
	createRectangle ();
	createfloor();
	createredbucket();
	creategreenbucket();
	createRectangle_r();
	createRectangle_g();
	createbeam();
	createturret();
	createmirror();
	createwall();
	
	// Create and compile our GLSL program from the shaders
	programID = LoadShaders( "Sample_GL.vert", "Sample_GL.frag" );
	// Get a handle for our "MVP" uniform
	Matrices.MatrixID = glGetUniformLocation(programID, "MVP");

	
	reshapeWindow (window, width, height);

    // Background color of the scene
	glClearColor (1, 1, 1, 0.0f); // R, G, B, A
	glClearDepth (1.0f);

	glEnable (GL_DEPTH_TEST);
	glDepthFunc (GL_LEQUAL);

    cout << "VENDOR: " << glGetString(GL_VENDOR) << endl;
    cout << "RENDERER: " << glGetString(GL_RENDERER) << endl;
    cout << "VERSION: " << glGetString(GL_VERSION) << endl;
    cout << "GLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
}

int main (int argc, char** argv)
{
	int width = 800;
	int height = 800;

	rb_y[0] = 3.9;
	gb_y[0] = 5.9;

	for (int i = 0; i <= 1000; ++i)
	{
		tr_x[i] = -2 + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/4));
		if (i!=0) tr_y[i] = 5 + tr_y[i-1];
		tr_z[i] = 0;
		if(i <= 500)
		{
			rb_x[i] = -2 + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/4));
			if (i!=0) rb_y[i] = 7 + rb_y[i-1];
			rb_z[i] = 0;
			gb_x[i] = -2 + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/4));
			if (i!=0) gb_y[i] = 7 + gb_y[i-1];
			gb_z[i] = 0;
		}
	}


    GLFWwindow* window = initGLFW(width, height);

	initGL (window, width, height);

    double last_update_time = glfwGetTime(), current_time;

    /* Draw in loop */

    cout << "\n\nWELCOME TO BRICK BREAKER\n" << endl;
    cout << "The aim of the game is to collect as many red and green bricks you can in respective colored buckets and destroy as many black bricks you can.\n" << endl;
    cout << "You lose a life for each black brick collected.\n" << endl;
    cout << "Scoring Scheme:\n" << endl;
    cout << "Collect brick in correct bucket ---->  +100" << endl;
    cout << "Collect brick in wrong bucket ------>  -10"  << endl;
    cout << "Destroy black brick ---------------->  +100" << endl;
    cout << "Destroy colored brick -------------->  -10\n" << endl;
    cout << "Controls:\n" << endl;
    cout << "Mouse :-" << endl;
    cout << "\tHover & Scroll Up ------> Moves the bucket left OR Rotates the laser anticlockwise." << endl;
    cout << "\tHover & Scroll Down ----> Moves the bucket right OR Rotates the laser clockwise." << endl;
    cout << "\tHover & Right click ----> Moves laser Down." << endl;
    cout << "\tHover & Left click -----> Moves laser Up.\n\n" << endl;
    cout << "Keyboard :-\n" << endl;
    cout << "\tSPACEBAR ---------------- > Fire laser." << endl;
    cout << "\tCTRL + LEFT/RIGHT ARROW --> Moves red bucket left/right." << endl;
    cout << "\tALT + LEFT/RIGHT ARROW ---> Moves green bucket left/right." << endl;
    cout << "\tW ------------------------> Moves laser Up." << endl;
    cout << "\tS ------------------------> Moves laser Down." << endl;
    cout << "\tA ------------------------> Rotates laser anticlockwise." << endl;
    cout << "\tD ------------------------> Rotates laser clockwise." << endl;
    cout << "\tN ------------------------> Decrease speed of bricks." << endl;
    cout << "\tM ------------------------> Increase speed of bricks." << endl;
    cout << "\tQ ------------------------> Quit Game.\n\n" << endl;
    cout << "Press number of lives to start the game with:" << endl;
    cin >> lives;

    while (!glfwWindowShouldClose(window) and lives != 0) {

        // OpenGL Draw commands
        draw();

        if(lives == 0)
        {
        	cout << "GAME OVER" << endl;
        	quit(window);	
        	//glfwTerminate();
        }

        // Swap Frame Buffer in double buffering
        glfwSwapBuffers(window);

        // Poll for Keyboard and mouse events
        glfwPollEvents();

        // Control based on time (Time based transformation like 5 degrees rotation every 0.5s)
        current_time = glfwGetTime(); // Time in seconds
        if ((current_time - last_update_time) >= 0.5) { // atleast 0.5s elapsed since last frame
            // do something every 0.5 seconds ..
            last_update_time = current_time;
        }
    }

    glfwTerminate();
    //exit(EXIT_SUCCESS);
}
