#include <iostream>
#include <sstream>
#include <iterator>
#include <vector>
#include <algorithm>
#include <Windows.h>
#include <gl/GL.h>



using namespace std;

typedef char GLchar;



// GL extension constants
#define GL_FRAGMENT_SHADER 35632
#define GL_VERTEX_SHADER 35633
#define GL_COMPILE_STATUS 35713
#define GL_LINK_STATUS 35714
#define GL_INFO_LOG_LENGTH 35716
#define GL_SHADING_LANGUAGE_VERSION 35724



// GL extension functions
typedef GLuint(__stdcall * GL_CREATE_SHADER_FUNC)(GLenum);
typedef GLuint(__stdcall* GL_CREATE_PROGRAM_FUNC)();
typedef GLvoid(__stdcall* GL_ATTACH_SHADER_FUNC)(GLuint, GLuint);
typedef GLvoid(__stdcall* GL_LINK_PROGRAM_FUNC)(GLuint);
typedef GLvoid(__stdcall* GL_SHADER_SOURCE_FUNC)(GLuint, GLsizei, const char**, const GLint*);
typedef GLvoid(__stdcall* GL_COMPILE_SHADER_FUNC)(GLuint);
typedef GLvoid(__stdcall* GL_USE_PROGRAM_FUNC)(GLuint);
typedef GLvoid(__stdcall* GL_GET_SHADER_IV_FUNC)(GLuint, GLenum, GLint*);
typedef GLvoid(__stdcall* GL_GET_PROGRAM_IV_FUNC)(GLuint, GLenum, GLint*);
typedef GLvoid(__stdcall* GL_GET_SHADER_INFO_LOG_FUNC)(GLuint, GLsizei, GLsizei*, GLchar*);
typedef GLvoid(__stdcall* GL_GET_PROGRAM_INFO_LOG_FUNC)(GLuint, GLsizei, GLsizei*, GLchar*);
typedef GLint(__stdcall* GL_GET_UNIFORM_LOCATION_FUNC)(GLuint, const GLchar*);
typedef GLvoid(__stdcall* GL_UNIFORM_1I_FUNC)(GLint, GLint);

GL_CREATE_SHADER_FUNC glCreateShader;
GL_CREATE_PROGRAM_FUNC glCreateProgram;
GL_ATTACH_SHADER_FUNC glAttachShader;
GL_LINK_PROGRAM_FUNC glLinkProgram;
GL_SHADER_SOURCE_FUNC glShaderSource;
GL_COMPILE_SHADER_FUNC glCompileShader;
GL_USE_PROGRAM_FUNC glUseProgram;
GL_GET_SHADER_IV_FUNC glGetShaderiv;
GL_GET_SHADER_INFO_LOG_FUNC glGetShaderInfoLog;
GL_GET_PROGRAM_IV_FUNC glGetProgramiv;
GL_GET_PROGRAM_INFO_LOG_FUNC glGetProgramInfoLog;
GL_GET_UNIFORM_LOCATION_FUNC glGetUniformLocation;
GL_UNIFORM_1I_FUNC glUniform1i;



GLint maxIterUniformLoc = -1;
GLint maxIter = 30;



// Shader source
const char* vertexShaderSource =
"#version 400 \n\
	\n\
	void main() \n\
	{ \
		gl_TexCoord[0] = gl_MultiTexCoord0; \n\
		gl_Position = ftransform(); \n\
	}";

const char* fragmentShaderSource =
"#version 400 \n\
	uniform int maxIter; \n\
	\n\
	void main() \n\
	{ \n\
		double iter = 0; \n\
		double x0 = gl_TexCoord[0].x; \n\
		double y0 = gl_TexCoord[0].y; \n\
		double x = 0, y = 0; \n\
		double t, log_zn, nu; \n\
		double log2 = log(2.0); \n\
		while (x*x + y*y < 2*2*2*2*2*2*2*2 && iter < maxIter) \n\
		{ \n\
			t = x*x - y*y + x0; \n\
			y = 2*x*y + y0; \n\
			x = t; \n\
			iter = iter + 1; \n\
		} \n\
		if (iter < maxIter) \n\
		{ \n\
			log_zn = log(float(x*x + y*y)) * 0.5; \n\
			nu = log(float(log_zn / log2)) / log2; \n\
			iter = iter + 1; \n\
		} \n\
		double mag = mix(iter / maxIter, (iter + 1) / maxIter, 1 - nu); \n\
		gl_FragColor = vec4(mag, mag, mag, 1); \n\
	}";



// GL render method
void Display(double zoom, double x, double y, double aspectRatio)
{
	// Calculate viewing bounds of the fractal in complex space
	double left = x - (2 / zoom) - .75;
	double right = x + (2 / zoom) - .75;
	double top = y - (2 / zoom);
	double bottom = y + (2 / zoom);

	// Render a square in the window
	glClear(GL_COLOR_BUFFER_BIT);
	glBegin(GL_QUADS);
	glTexCoord2d(left, top);
	glVertex2d(-1, 1 / aspectRatio);
	glTexCoord2d(right, top);
	glVertex2d(1, 1 / aspectRatio);
	glTexCoord2d(right, bottom);
	glVertex2d(1, -1 / aspectRatio);
	glTexCoord2d(left, bottom);
	glVertex2d(-1, -1 / aspectRatio);
	glEnd();
	glFlush();
}



// Print controls for the user
void PrintControls()
{
	cout << endl;
	cout << "--- CONTROLS ---" << endl;
	cout << "W/S/A/D        Pan up/down/left/right" << endl;
	cout << "R/F            Increase/decrease zoom level" << endl;
	cout << "E/Q            Increase/decrease detail level" << endl;
	cout << "Z              Reset parameters" << endl;
	cout << "H              Print these controls" << endl;
	cout << endl;
}



// Window procedure
LRESULT WINAPI WinFunc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static PAINTSTRUCT ps;
	static int zoomLevel = 1;
	static double zoom = 1;
	static double offsetX = 0;
	static double offsetY = 0;
	static double aspectRatio = 1;

	switch (msg)
	{
	case WM_PAINT:
		// Print current parameters when fractal is drawn
		cout << "X:" << offsetX << " Y:" << offsetY << " Z:" << zoomLevel << " I:" << maxIter << endl;

		// Draw fractal
		Display(zoom, offsetX, offsetY, aspectRatio);

		BeginPaint(hwnd, &ps);
		EndPaint(hwnd, &ps);
		break;

	case WM_SIZE:
		// Re-calculate aspect ratio
		aspectRatio = (double)(HIWORD(lParam)) / (double)(LOWORD(lParam));

		// Resize GL viewport to cover client area
		glViewport(0, 0, LOWORD(lParam), HIWORD(lParam));

		// Redraw window
		PostMessage(hwnd, WM_PAINT, 0, 0);
		break;

	case WM_CHAR:
		switch (wParam)
		{
			// Increase zoom level
		case 'r':
		case 'R':
			zoomLevel += 1;
			if (zoomLevel > 999)
			{
				zoomLevel = 999;
			}
			else
			{
				zoom = pow(zoomLevel, 2);
				PostMessage(hwnd, WM_PAINT, 0, 0);
			}
			break;

			// Decrease zoom level
		case 'f':
		case 'F':
			zoomLevel -= 1;
			if (zoomLevel < 1)
			{
				zoomLevel = 1;
			}
			else
			{
				zoom = pow(zoomLevel, 2);
				PostMessage(hwnd, WM_PAINT, 0, 0);
			}
			break;

			// Decrease maximum iterations
		case 'q':
		case 'Q':
			--maxIter;
			if (maxIter < 1)
			{
				maxIter = 1;
			}
			else if (maxIterUniformLoc != -1)
			{
				glUniform1i(maxIterUniformLoc, maxIter);
				PostMessage(hwnd, WM_PAINT, 0, 0);
			}
			break;

			// Increase maximum iterations
		case 'e':
		case 'E':
			++maxIter;
			if (maxIter > 999)
			{
				maxIter = 999;
			}
			else if (maxIterUniformLoc != -1)
			{
				glUniform1i(maxIterUniformLoc, maxIter);
				PostMessage(hwnd, WM_PAINT, 0, 0);
			}
			break;

			// Pan up
		case 'w':
		case 'W':
			if (offsetY > -2)
			{
				offsetY -= 0.1f / zoom;
				PostMessage(hwnd, WM_PAINT, 0, 0);
			}
			break;

			// Pan down
		case 's':
		case 'S':
			if (offsetY < 2)
			{
				offsetY += 0.1f / zoom;
				PostMessage(hwnd, WM_PAINT, 0, 0);
			}
			break;

			// Pan right
		case 'd':
		case 'D':
			if (offsetX < 2)
			{
				offsetX += 0.1f / zoom;
				PostMessage(hwnd, WM_PAINT, 0, 0);
			}
			break;

			// Pan left
		case 'a':
		case 'A':
			if (offsetX > -2)
			{
				offsetX -= 0.1f / zoom;
				PostMessage(hwnd, WM_PAINT, 0, 0);
			}
			break;

			// Reset parameters
		case 'z':
		case 'Z':
			offsetX = 0;
			offsetY = 0;
			zoomLevel = 1;
			zoom = 1;
			maxIter = 30;
			glUniform1i(maxIterUniformLoc, maxIter);
			PostMessage(hwnd, WM_PAINT, 0, 0);
			break;

			// Print controls
		case 'h':
		case 'H':
			PrintControls();
			break;
		}

		break;

	case WM_CLOSE:
		// End program
		PostQuitMessage(0);
		break;

	default:
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}

	return 0;
}



// Create the window
HWND CreateMandelbrotWindow()
{
	// Get main module handle
	HINSTANCE hInstance = GetModuleHandle(NULL);

	// Create and register window class
	WNDCLASS wc;
	ZeroMemory(&wc, sizeof(wc));
	wc.hInstance = hInstance;
	wc.style = CS_OWNDC;
	wc.lpfnWndProc = WinFunc;
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.lpszClassName = "Mandelbrot";

	if (!RegisterClass(&wc))
	{
		MessageBox(NULL, "Failed to register window class.", "Cannot register window class", MB_OK);
		return 0;
	}

	// Create window
	HWND hwnd = CreateWindow("Mandelbrot", "Mandelbrot", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 640, 480, NULL, NULL, hInstance, NULL);

	if (hwnd == NULL)
	{
		MessageBox(NULL, "Failed to create window.", "Failed to create window", MB_OK);
	}

	return hwnd;
}



// Initiate the GL context for the window
void InitDeviceContext(HWND hwnd, HDC* hdc, HGLRC* hrc)
{
	*hdc = GetDC(hwnd);

	PIXELFORMATDESCRIPTOR pfd;
	ZeroMemory(&pfd, sizeof(pfd));
	pfd.nSize = sizeof(pfd);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 32;

	int pixelFormat = ChoosePixelFormat(*hdc, &pfd);

	if (pixelFormat == 0)
	{
		MessageBox(NULL, "Failed to choose pixel format.", "Failed to choose pixel format", MB_OK);
	}

	if (SetPixelFormat(*hdc, pixelFormat, &pfd) == FALSE)
	{
		MessageBox(NULL, "Failed to set pixel format.", "Failed to set pixel format", MB_OK);
	}

	DescribePixelFormat(*hdc, pixelFormat, sizeof(PIXELFORMATDESCRIPTOR), &pfd);

	*hrc = wglCreateContext(*hdc);
	wglMakeCurrent(*hdc, *hrc);
}



// Initialise GL state
void InitGL()
{
	// Cull back face
	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CW);
	glCullFace(GL_BACK);

	// Load extension functions
	glCreateShader = (GL_CREATE_SHADER_FUNC)wglGetProcAddress("glCreateShader");
	glCreateProgram = (GL_CREATE_PROGRAM_FUNC)wglGetProcAddress("glCreateProgram");
	glAttachShader = (GL_ATTACH_SHADER_FUNC)wglGetProcAddress("glAttachShader");
	glLinkProgram = (GL_LINK_PROGRAM_FUNC)wglGetProcAddress("glLinkProgram");
	glShaderSource = (GL_SHADER_SOURCE_FUNC)wglGetProcAddress("glShaderSource");
	glCompileShader = (GL_COMPILE_SHADER_FUNC)wglGetProcAddress("glCompileShader");
	glUseProgram = (GL_USE_PROGRAM_FUNC)wglGetProcAddress("glUseProgram");
	glGetShaderiv = (GL_GET_SHADER_IV_FUNC)wglGetProcAddress("glGetShaderiv");
	glGetShaderInfoLog = (GL_GET_SHADER_INFO_LOG_FUNC)wglGetProcAddress("glGetShaderInfoLog");
	glGetProgramiv = (GL_GET_SHADER_IV_FUNC)wglGetProcAddress("glGetProgramiv");
	glGetProgramInfoLog = (GL_GET_PROGRAM_INFO_LOG_FUNC)wglGetProcAddress("glGetProgramInfoLog");
	glGetUniformLocation = (GL_GET_UNIFORM_LOCATION_FUNC)wglGetProcAddress("glGetUniformLocation");
	glUniform1i = (GL_UNIFORM_1I_FUNC)wglGetProcAddress("glUniform1i");
}



// Initialise pixel shader
void InitShader()
{
	GLint success = 0;

	// Compile vertex shader
	unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
	int vertShaderLen = strlen(vertexShaderSource);
	glShaderSource(vertexShader, 1, &vertexShaderSource, &vertShaderLen);
	glCompileShader(vertexShader);

	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);

	if (success == GL_FALSE)
	{
		cout << "Error compiling vertex shader" << endl;
		GLint maxLength = 0;
		glGetShaderiv(vertexShader, GL_INFO_LOG_LENGTH, &maxLength);
		vector<GLchar> infoLog(maxLength);
		glGetShaderInfoLog(vertexShader, maxLength, &maxLength, &infoLog[0]);
		cout << (GLchar*)(&infoLog[0]) << endl;
	}

	// Compile fragment shader
	unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	int fragShaderLen = strlen(fragmentShaderSource);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, &fragShaderLen);
	glCompileShader(fragmentShader);

	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);

	if (success == GL_FALSE)
	{
		cout << "Error compiling fragment shader" << endl;
		GLint maxLength = 0;
		glGetShaderiv(fragmentShader, GL_INFO_LOG_LENGTH, &maxLength);
		vector<GLchar> infoLog(maxLength);
		glGetShaderInfoLog(fragmentShader, maxLength, &maxLength, &infoLog[0]);
		cout << (GLchar*)(&infoLog[0]) << endl;
	}

	// Link shader program
	unsigned int program = glCreateProgram();
	glAttachShader(program, vertexShader);
	glAttachShader(program, fragmentShader);
	glLinkProgram(program);

	glGetProgramiv(program, GL_LINK_STATUS, &success);

	if (success == GL_FALSE)
	{
		cout << "Error linking shader" << endl;
		GLint maxLength = 0;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);
		vector<GLchar> infoLog(maxLength);
		glGetProgramInfoLog(program, maxLength, &maxLength, &infoLog[0]);
		cout << (GLchar*)(&infoLog[0]) << endl;
	}

	// Use shader
	glUseProgram(program);

	// Get location of maxIter uniform and set value
	maxIterUniformLoc = glGetUniformLocation(program, "maxIter");
	glUniform1i(maxIterUniformLoc, maxIter);
}



int main()
{
	// Create window
	HWND hwnd = CreateMandelbrotWindow();

	// Initialise device context
	HDC hdc = NULL;
	HGLRC hrc = NULL;
	InitDeviceContext(hwnd, &hdc, &hrc);

	// Initialise GL
	InitGL();

	// Create shader
	InitShader();

	// Print OpenGL version
	cout << "GL version: " << glGetString(GL_VERSION) << endl;

	// Print GLSL version
	cout << "GLSL version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;

	// Sort and print GL extensions
	istringstream extStream((const char*)glGetString(GL_EXTENSIONS));
	istream_iterator<string> beg(extStream), end;
	vector<string> exts(beg, end);
	sort(exts.begin(), exts.end());

	if (find(exts.begin(), exts.end(), "GL_ARB_gpu_shader_fp64") != exts.end())
		cout << "GPU supports double precision" << endl;

	//cout << "GL extensions" << endl;
	//for (int i = 0; i < exts.size(); ++i)
	//	cout << exts[i] << endl;

	// Print program information
	cout << "\nMANDELBROT" << endl;
	PrintControls();

	// Show window
	ShowWindow(hwnd, SW_SHOW);

	// Message loop
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0) != 0)
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	// Clean up
	wglMakeCurrent(NULL, NULL);
	ReleaseDC(hwnd, hdc);
	wglDeleteContext(hrc);
	DestroyWindow(hwnd);

	return msg.wParam;
}
