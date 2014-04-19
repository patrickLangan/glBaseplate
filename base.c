#include<stdio.h>
#include<stdlib.h>

#include<X11/X.h>
#include<X11/Xlib.h>

#include<GL/gl.h>
#include<GL/glx.h>
#include<GL/glu.h>

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

	GLXContext glc;

	const GLchar* vertexSource =
	       "#version 150 core\n \
		in vec2 position; \
		void main () \
		{ \
			gl_Position = vec4(position, 0.0, 1.0); \
		}";
	const GLchar* fragmentSource =
	       "#version 150 core\n \
		out vec4 outColor; \
		void main () \
		{ \
			outColor = vec4 (1.0, 1.0, 1.0, 1.0); \
		}";

	GLuint vertexArray;
	GLuint vertexBufferObject;
	GLfloat vertices[] = {
		-0.5f,  0.5f,
		0.5f,  0.5f,
		0.5f, -0.5f,
		-0.5f, -0.5f,
	};
	GLint posAttrib;

	GLuint elementArray;
	GLuint elements[] = {
		0, 1, 2,
		2, 3, 0
	};

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
	glc = glXCreateContext (display, visualInfo, NULL, GL_TRUE);
	glXMakeCurrent (display, xWindow, glc);


	//Compiling the vertex shader
	GLuint vertexShader = glCreateShader (GL_VERTEX_SHADER);
	glShaderSource (vertexShader, 1, &vertexSource, NULL);
	glCompileShader (vertexShader);

	//Compiling the fragment shader
	GLuint fragmentShader = glCreateShader (GL_FRAGMENT_SHADER);
	glShaderSource (fragmentShader, 1, &fragmentSource, NULL);
	glCompileShader (fragmentShader);

	//Sticking the vertex and fragment shaders together
	GLuint shaderProgram = glCreateProgram ();
	glAttachShader (shaderProgram, vertexShader);
	glAttachShader (shaderProgram, fragmentShader);
	glBindFragDataLocation (shaderProgram, 0, "outColor");
	glLinkProgram (shaderProgram);
	glUseProgram (shaderProgram);


	//Making the vertex buffer object
	glGenVertexArrays (1, &vertexArray);
	glBindVertexArray (vertexArray);
	glGenBuffers (1, &vertexBufferObject);
	glBindBuffer (GL_ARRAY_BUFFER, vertexBufferObject);
	glBufferData (GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	//Setting the vertex buffer's data format
	posAttrib = glGetAttribLocation (shaderProgram, "position");
	glEnableVertexAttribArray (posAttrib);
	glVertexAttribPointer (posAttrib, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), 0);


	//Making the element array
	glGenBuffers (1, &elementArray);
	glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, elementArray);
	glBufferData (GL_ELEMENT_ARRAY_BUFFER, sizeof(elements), elements, GL_STATIC_DRAW);


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
			glClear (GL_COLOR_BUFFER_BIT);

			//Drawing the triangles
			glDrawElements (GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

			//Writing the openGL buffer to the xWindow
			glXSwapBuffers (display, xWindow);
		} else if (xEvent.type == KeyPress) {
			//Shutting down
			glXMakeCurrent (display, None, NULL);
			glXDestroyContext (display, glc);
			XDestroyWindow (display, xWindow);
			XCloseDisplay (display);

			glDeleteProgram (shaderProgram);
			glDeleteShader (fragmentShader);
			glDeleteShader (vertexShader);

			glDeleteBuffers (1, &elementArray);
			glDeleteBuffers (1, &vertexBufferObject);

			glDeleteVertexArrays (1, &vertexArray);

			exit (0);
		}
	}
}
