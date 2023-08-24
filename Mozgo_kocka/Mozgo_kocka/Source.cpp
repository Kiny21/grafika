#include <array>
#include <fstream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <math.h>
#include <string>
#include <vector>

using namespace std;

extern void cleanUpScene();

GLFWwindow* window;

// normal keys are fom [0..255], arrow and special keys are from [256..511]
GLboolean	keyboard[512] = { GL_FALSE };

int		window_width = 600;
int		window_height = 600;
char	window_title[] = "Camera";

float objectYPos = 0.0f;

bool rotateEnabled = true;

float vertices[] = {
	// positions          colors
	-0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 0.0f,
	 0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 0.0f,
	 0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f,
	 0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f,
	-0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f,
	-0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 0.0f,

	-0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f,
	 0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f,
	 0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f,
	 0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f,
	-0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f,
	-0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f,

	-0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 0.0f,
	-0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 0.0f,
	-0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 0.0f,
	-0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 0.0f,
	-0.5f, -0.5f,  0.5f,  1.0f, 1.0f, 0.0f,
	-0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 0.0f,

	 0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 1.0f,
	 0.5f,  0.5f, -0.5f,  0.0f, 0.0f, 1.0f,
	 0.5f, -0.5f, -0.5f,  0.0f, 0.0f, 1.0f,
	 0.5f, -0.5f, -0.5f,  0.0f, 0.0f, 1.0f,
	 0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 1.0f,
	 0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 1.0f,

	-0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 1.0f,
	 0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 1.0f,
	 0.5f, -0.5f,  0.5f,  0.0f, 1.0f, 1.0f,
	 0.5f, -0.5f,  0.5f,  0.0f, 1.0f, 1.0f,
	-0.5f, -0.5f,  0.5f,  0.0f, 1.0f, 1.0f,
	-0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 1.0f,

	-0.5f,  0.5f, -0.5f,  1.0f, 0.0f, 1.0f,
	 0.5f,  0.5f, -0.5f,  1.0f, 0.0f, 1.0f,
	 0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 1.0f,
	 0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 1.0f,
	-0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 1.0f,
	-0.5f,  0.5f, -0.5f,  1.0f, 0.0f, 1.0f

};

/* Vertex buffer objektum �s vertex array objektum az adatt�rol�shoz.*/
#define numVBOs	1
#define numVAOs	1
GLuint			VBO[numVBOs];
GLuint			VAO[numVAOs];

GLuint			renderingProgram;

unsigned int	modelLoc;
unsigned int	viewLoc;
unsigned int	projectionLoc;

/** Vet�t�si �s kamera m�trixok felv�tele. */
glm::mat4		view, projection = glm::perspective(glm::radians(45.0f), (float)window_width / (float)window_height, 0.1f, 100.0f);

GLdouble		currentTime, deltaTime, lastTime = 0.0f;
GLfloat			cameraSpeed;
glm::mat4		model = glm::mat4(1.0f),
				translateM = glm::mat4(1.0f);
glm::mat4		rotateM = glm::mat4(1.0f);


/* Vegy�k fel a kamera pozic�t t�rol� v�ltoz�t, illetve a tengelyekhezz sz�ks�ges vektorokat. */
glm::vec3		cameraPos = glm::vec3(0.0f, 0.0f, 3.0f),
cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f),
cameraUpVector = glm::vec3(0.0f, 1.0f, 0.0f),
cameraDirection = glm::vec3(0.0f, 0.0f, -1.0f); // direction for camera


bool checkOpenGLError() {
	bool foundError = false;
	int glErr = glGetError();
	while (glErr != GL_NO_ERROR) {
		cout << "glError: " << glErr << endl;
		foundError = true;
		glErr = glGetError();
	}
	return foundError;
}

void printShaderLog(GLuint shader) {
	int len = 0;
	int chWrittn = 0;
	char* log;
	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);
	if (len > 0) {
		log = (char*)malloc(len);
		glGetShaderInfoLog(shader, len, &chWrittn, log);
		cout << "Shader Info Log: " << log << endl;
		free(log);
	}
}

void printProgramLog(int prog) {
	int len = 0;
	int chWrittn = 0;
	char* log;
	glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &len);
	if (len > 0) {
		log = (char*)malloc(len);
		glGetProgramInfoLog(prog, len, &chWrittn, log);
		cout << "Program Info Log: " << log << endl;
		free(log);
	}
}

string readShaderSource(const char* filePath) {
	string content;
	ifstream fileStream(filePath, ios::in);
	string line = "";

	while (!fileStream.eof()) {
		getline(fileStream, line);
		content.append(line + "\n");
	}
	fileStream.close();
	return content;
}

GLuint createShaderProgram() {

	GLint vertCompiled;
	GLint fragCompiled;
	GLint linked;

	string vertShaderStr = readShaderSource("vertexShader.glsl");
	string fragShaderStr = readShaderSource("fragmentShader.glsl");

	GLuint vShader = glCreateShader(GL_VERTEX_SHADER);
	GLuint fShader = glCreateShader(GL_FRAGMENT_SHADER);

	const char* vertShaderSrc = vertShaderStr.c_str();
	const char* fragShaderSrc = fragShaderStr.c_str();

	glShaderSource(vShader, 1, &vertShaderSrc, NULL);
	glShaderSource(fShader, 1, &fragShaderSrc, NULL);

	glCompileShader(vShader);
	checkOpenGLError();
	glGetShaderiv(vShader, GL_COMPILE_STATUS, &vertCompiled);
	if (vertCompiled != 1) {
		cout << "vertex compilation failed" << endl;
		printShaderLog(vShader);
	}


	glCompileShader(fShader);
	checkOpenGLError();
	glGetShaderiv(vShader, GL_COMPILE_STATUS, &fragCompiled);
	if (fragCompiled != 1) {
		cout << "fragment compilation failed" << endl;
		printShaderLog(fShader);
	}

	// Shader program objektum l�trehoz�sa. Elt�roljuk az ID �rt�ket.
	GLuint vfProgram = glCreateProgram();
	glAttachShader(vfProgram, vShader);
	glAttachShader(vfProgram, fShader);

	glLinkProgram(vfProgram);
	checkOpenGLError();
	glGetProgramiv(vfProgram, GL_LINK_STATUS, &linked);
	if (linked != 1) {
		cout << "linking failed" << endl;
		printProgramLog(vfProgram);
	}

	glDeleteShader(vShader);
	glDeleteShader(fShader);

	return vfProgram;
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if ((action == GLFW_PRESS) && (key == GLFW_KEY_ESCAPE))
		cleanUpScene();

/*Le�ll�t�s space megnyom�sakor, majd �jraind�t�s*/
	if (action == GLFW_PRESS) {
		keyboard[key] = GL_TRUE;
		if (key == GLFW_KEY_SPACE)
		{
			rotateEnabled = !rotateEnabled;
		}
	}
	else if (action == GLFW_RELEASE)
		keyboard[key] = GL_FALSE;
}

void cursorPosCallback(GLFWwindow* window, double xPos, double yPos)
{

}

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{

}

/** Kisz�moljuk a kamera m�trixokat. */
void computeCameraMatrix() {
	/* A param�terek rendre: az �j koordin�tarendszer�nk k�z�ppontja (hol a kamera), merre n�z a kamera, mit tekint�nk ,,f�lfele" ir�nynak */
	view = glm::lookAt(cameraPos, cameraPos + cameraDirection, cameraUpVector);
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
}

void init(GLFWwindow* window) {

	renderingProgram = createShaderProgram();
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS);

	/* L�trehozzuk a sz�ks�ges Vertex buffer �s vertex array objektumot. */
	glGenBuffers(numVBOs, VBO);
	glGenVertexArrays(numVAOs, VAO);

	/* T�pus meghat�roz�sa: a GL_ARRAY_BUFFER neves�tett csatol�ponthoz kapcsoljuk a buffert (ide ker�lnek a vertex adatok). */
	glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);

	/* M�soljuk az adatokat a pufferbe! Megadjuk az aktu�lisan csatolt puffert,  azt hogy h�ny b�jt adatot m�solunk,
	a m�soland� adatot, majd a feldolgoz�s m�dj�t is meghat�rozzuk: most az adat nem v�ltozik a felt�lt�s ut�n */
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	/* Csatoljuk a vertex array objektumunkat a konfigur�l�shoz. */
	glBindVertexArray(VAO[0]);
	
	/*Kocka attrib�tum*/
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	/* Sz�n attrib�tum */
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	/* Lev�lasztjuk a vertex array objektumot �s a puffert is.*/
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// aktiv�ljuk a shader-program objektumunkat.
	// activate the shader program
	glUseProgram(renderingProgram);

	// get the location of the vertex variables
	modelLoc = glGetUniformLocation(renderingProgram, "model");
	viewLoc = glGetUniformLocation(renderingProgram, "view");
	projectionLoc = glGetUniformLocation(renderingProgram, "projection");
	// set the projection, since it change rarely
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
	// set black for clearing
	glClearColor(0.0, 0.0, 0.0, 1.0);
}

/** A jelenet�nk ut�ni takar�t�s. */
void cleanUpScene() {
	/* t�r�lj�k a GLFW ablakot. */
	glfwDestroyWindow(window);

	/** T�r�lj�k a vertex puffer �s vertex array objektumokat. */
	glDeleteVertexArrays(numVAOs, VAO);
	glDeleteBuffers(numVBOs, VBO);

	/** T�r�lj�k a shader programot. */
	glDeleteProgram(renderingProgram);

	/* Le�ll�tjuk a GLFW-t */
	// stop GLFW
	glfwTerminate();

	exit(EXIT_SUCCESS);
}

void display() {



	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 

	currentTime = glfwGetTime();
	deltaTime = currentTime - lastTime;
	
	//fel-le mozg�s
	objectYPos = sin(currentTime) * 0.5f;
	lastTime = currentTime;


	if (rotateEnabled) {
		//forgat�s
		rotateM = glm::rotate(glm::mat4(1.0f), (float)glfwGetTime() * glm::radians(50.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		translateM = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -objectYPos, 0.0f));
		model = rotateM * translateM;
	}


	// this makes the animation continous, because camera speed proportional with elapsed time
	cameraSpeed = 2.5f * (GLfloat)deltaTime;
	// we will use two set ups for moving: WASD and the arrow keys
	if ((keyboard[GLFW_KEY_W]) || (keyboard[GLFW_KEY_UP]))
		cameraPos += cameraSpeed * cameraDirection;

	if ((keyboard[GLFW_KEY_S]) || (keyboard[GLFW_KEY_DOWN]))
		cameraPos -= cameraSpeed * cameraDirection;

	if ((keyboard[GLFW_KEY_A]) || (keyboard[GLFW_KEY_LEFT]))
		cameraPos -= cameraSpeed * glm::normalize(glm::cross(cameraDirection, cameraUpVector)); // compute side moving vector

	if ((keyboard[GLFW_KEY_D]) || (keyboard[GLFW_KEY_RIGHT]))
		cameraPos += cameraSpeed * glm::normalize(glm::cross(cameraDirection, cameraUpVector)); // compute side moving vector


	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	computeCameraMatrix();

	/*Csatoljuk a vertex array objektumunkat. */
	glBindVertexArray(VAO[0]);
	//glPolygonMode(GL_FRONT, GL_TRIANGLES);
	glDrawArrays(GL_TRIANGLES, 0, sizeof(vertices));

	/* Lev�lasztjuk, nehogy b�rmilyen �rt�k fel�l�r�djon.*/
	glBindVertexArray(0);
}

void framebufferSizeCallback(GLFWwindow* window, int width, int height) {
	window_width = width;
	window_height = height;

	glViewport(0, 0, width, height);

	projection = glm::perspective(glm::radians(45.0f), (float)window_width / (float)window_height, 0.1f, 100.0f);
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
}

int main(void) {

	/* Pr�b�ljuk meg inicializ�lni a GLFW-t! */
	if (!glfwInit()) { exit(EXIT_FAILURE); }


	/* A k�v�nt OpenGL verzi� (4.3) */
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

	/* Pr�b�ljuk meg l�trehozni az ablakunkat. */
	window = glfwCreateWindow(window_width, window_height, window_title, nullptr, nullptr);

	/* V�lasszuk ki az ablakunk OpenGL kontextus�t, hogy haszn�lhassuk. */
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

	glfwSetKeyCallback(window, keyCallback);
	glfwSetCursorPosCallback(window, cursorPosCallback);
	glfwSetMouseButtonCallback(window, mouseButtonCallback);




	/* Incializ�ljuk a GLEW-t, hogy el�rhet�v� v�ljanak az OpenGL f�ggv�nyek. */
	if (glewInit() != GLEW_OK) { exit(EXIT_FAILURE); }
	glfwSwapInterval(1);

	/* Az alkalmaz�shoz kapcsol�d� el�k�sz�t� l�p�sek, pl. hozd l�tre a shader objektumokat. */
	init(window);

	while (!glfwWindowShouldClose(window)) {
		/* a k�d, amellyel rajzolni tudunk a GLFWwindow ojektumunkba. */
		display();
		/* double buffered */
		glfwSwapBuffers(window);
		/* esem�nyek kezel�se az ablakunkkal kapcsolatban, pl. gombnyom�s */
		glfwPollEvents();
	}

	cleanUpScene();

	return EXIT_SUCCESS;
}