#include<stdio.h>
#include<stdlib.h>

#include<X11/X.h>
#include<X11/Xlib.h>

#include<GL/gl.h>
#include<GL/glx.h>
#include<GL/glu.h>

//Add error checking

void loadCharFile (char *fileName, char **source)
{
	FILE *file;
	unsigned int length;

	file = fopen (fileName, "r");

	fseek (file, 0, SEEK_END);
	length = ftell (file);
	fseek (file, 0, SEEK_SET);

	*source = malloc (length + 1);

	fread (*source, 1, length, file);

	fclose (file);
}

int main(int argc, char **argv)
{
	Display *display;
	Window root;
	int attributeList[] = {GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None};
	XVisualInfo *visualInfo;
	Colormap colorMap;
	XSetWindowAttributes setWindowAttributes;
	Window xWindow;
	XWindowAttributes windowAttributes;
	XEvent xEvent;

	GLXContext glContext;

	char *vertexSource;
	char *fragmentSource;
	GLuint vertexShader;
	GLuint fragmentShader;

	FILE *file;

	GLuint vertexArray;
	GLuint vertexBufferObject;
	int vertexNum;
	GLfloat *vertices;
	GLint posAttrib;

	GLuint elementArray;
	int elementNum;
	GLuint *elements;

	unsigned int i;


	//Loading the shader files into char arrays
	loadCharFile ("shader.vert", &vertexSource);
	loadCharFile ("shader.frag", &fragmentSource);


	//Creating the xWindow
	display = XOpenDisplay (NULL);
	root = DefaultRootWindow (display);
	visualInfo = glXChooseVisual (display, 0, attributeList);
	colorMap = XCreateColormap (display, root, visualInfo->visual, AllocNone);
	setWindowAttributes.colormap = colorMap;
	setWindowAttributes.event_mask = ExposureMask | KeyPressMask;
	xWindow = XCreateWindow (display, root, 0, 0, 600, 600, 0, visualInfo->depth, InputOutput, visualInfo->visual, CWColormap | CWEventMask, &setWindowAttributes);
	XMapWindow (display, xWindow);
	XStoreName (display, xWindow, "glBaseplate");

	//Making a openGL rendering context
	glContext = glXCreateContext (display, visualInfo, NULL, GL_TRUE);
	XFree (visualInfo);
	glXMakeCurrent (display, xWindow, glContext);


	//Compiling the vertex shader
	vertexShader = glCreateShader (GL_VERTEX_SHADER);
	glShaderSource (vertexShader, 1, &vertexSource, NULL);
	glCompileShader (vertexShader);
	free (vertexSource);

	//Compiling the fragment shader
	fragmentShader = glCreateShader (GL_FRAGMENT_SHADER);
	glShaderSource (fragmentShader, 1, &fragmentSource, NULL);
	glCompileShader (fragmentShader);
	free (fragmentSource);

	//Sticking the vertex and fragment shaders together
	GLuint shaderProgram = glCreateProgram ();
	glAttachShader (shaderProgram, vertexShader);
	glAttachShader (shaderProgram, fragmentShader);
	glBindFragDataLocation (shaderProgram, 0, "outColor");
	glLinkProgram (shaderProgram);
	glUseProgram (shaderProgram);


	//Loading the 3D model into the vertex and face arrays
	file = fopen ("model.bin", "r");

	fread (&vertexNum, sizeof(int), 1, file);
	vertices = malloc (vertexNum * 3 * sizeof(float));
	fread (vertices, sizeof(float), vertexNum * 3, file);

	fread (&elementNum, sizeof(int), 1, file);
	elements = malloc (elementNum * 3 * sizeof(int));
	fread (elements, sizeof(int), elementNum * 3, file);

	fclose (file);


	//Making the vertex buffer object
	glGenVertexArrays (1, &vertexArray);
	glBindVertexArray (vertexArray);
	glGenBuffers (1, &vertexBufferObject);
	glBindBuffer (GL_ARRAY_BUFFER, vertexBufferObject);
	glBufferData (GL_ARRAY_BUFFER, vertexNum * 3 * sizeof(GLfloat), vertices, GL_STATIC_DRAW);
	free (vertices);

	posAttrib = glGetAttribLocation (shaderProgram, "position");
	glEnableVertexAttribArray (posAttrib);
	glVertexAttribPointer (posAttrib, 3, GL_FLOAT, GL_FALSE, 0, 0);


	//Making the element array
	glGenBuffers (1, &elementArray);
	glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, elementArray);
	glBufferData (GL_ELEMENT_ARRAY_BUFFER, elementNum * 3 * sizeof(GLuint), elements, GL_STATIC_DRAW);
	free (elements);


	glEnable (GL_DEPTH_TEST);


	//Rendering loop
	while (1) {
		//Getting the current x event.
		XNextEvent(display, &xEvent);

		if (xEvent.type == Expose) {
			//Getting the xWindow's dimentions
			XGetWindowAttributes (display, xWindow, &windowAttributes);
			//Setting the openGL buffer size
			glViewport (0, 0, windowAttributes.width, windowAttributes.height);

			//Setting a color for the following glClear function
			glClearColor (0.0f, 0.0f, 0.0f, 1.0f);
			//Filling the selected buffer with the color set by the previsualInfoous function
			glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			//Drawing the triangles
			glDrawElements (GL_TRIANGLES, elementNum * 3, GL_UNSIGNED_INT, 0);

			//Writing the openGL buffer to the xWindow
			glXSwapBuffers (display, xWindow);
		} else if (xEvent.type == KeyPress) {
			//Shutting down
			glXMakeCurrent (display, None, NULL);
			glXDestroyContext (display, glContext);
			XDestroyWindow (display, xWindow);
			XCloseDisplay (display);

			glDeleteProgram (shaderProgram);
			glDeleteShader (fragmentShader);
			glDeleteShader (vertexShader);

			glDeleteBuffers (1, &elementArray);
			glDeleteBuffers (1, &vertexBufferObject);

			glDeleteVertexArrays (1, &vertexArray);

			return 0;
		}
	}
}

