/*
*	PROYECTO GRUA EN 3.3
*
* Autores: Adrián Martínez Balea y Xabier Primoi Martínez
*
* Funcionamiento:
*	Movimiento:
*		W => Acelerar
*		X => Frenar
*		A => Giro a la izquierda
*		D => Giro a la derecha
*
*	Mover primer brazo:
*		T => Hacia arriba
*		G => Hacia abajo
*		F => Hacia la izquierda
*		H => Hacia la derecha
*
*	Mover segundo brazo:
*		Flecha hacia arriba => Hacia arriba
*		Flecha hacia abajo => Hacia abajo
*		Flecha hacia la izquierda => Hacia la izquierda
*		Flecha hacia la derecha => Hacia la derecha
*
*	Girar suelo:
*		K => En sentido de las agujas del reloj
*		L => En sentido contrario
*
*	Camara:
*		1 => Primera persona
*		2 => Camara exterior
*		3 => Tercera persona
*
*/



#include <glad.h>
#include <glfw3.h>
#include <stdio.h>
#include <iostream>
#include <math.h> 
#include "esfera.h"
#include "lecturaShader.h"

// transformaciones
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// texturas
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"


// constantes
#define ARADIANES 0.0174  // pasar de grados a radianes


// settings
unsigned int SCR_WIDTH = 800;
unsigned int SCR_HEIGHT = 800;


// variables globales
GLuint shaderProgram; //shader 
int camara = 2; // camara: 1 => primera persona, 2 => exterior, 3 => tercera persona
float angulo = 0.0f; // angulo de giro del suelo
float velocidad = 0.0f; // velocidad de la grua
int indiceTexturaPiscina = 0; // indice para seleccionar textura de piscina
int cuentaPiscina = 0; // cuenta para cambiar el indice de la piscina


// VAOs
unsigned int VAOCuadrado;
unsigned int VAOCubo;
unsigned int VAOEsfera;

// Texturas
unsigned int texturaGrua;
unsigned int texturaSuelo;
unsigned int texturaPiscina[16];


typedef struct {
	float px, py, pz;			// posicion inicial
	float angulo_trans;			// angulo giro x
	float angulo_trans_2;		// angulo giro z
	float sx, sy, sz;			// escalado en los dos ejes
	unsigned int listaRender;	// VAO
	unsigned int nVertices;		// numero de vertices
} objeto;


//					posicion inicial	angulo x	angulo y	escalado en los ejes	lista render    nVertices

objeto base =	{ 0.0f, 0.0f, 0.15f,	 0.0f,		 0.0f,		0.30f, 0.20f, 0.20f,		0,				0 };		// base
objeto baseA1 = { 0.0f, 0.0f, 0.10f,	 0.0f,		 0.0f,		0.07f, 0.07f, 0.07f,		0,				0 };		// primera articulacion (esfera)
objeto base1 =	{ 0.0f, 0.0f, 0.10f,	 0.0f,		 0.0f,		0.05f, 0.05f, 0.30f,		0,				0 };		// primer brazo (cubo)
objeto baseA2 = { 0.0f, 0.0f, 0.15f,	 0.0f,		 0.0f,		0.05f, 0.05f, 0.05f,		0,				0 };		// segunda articulacion (esfera)
objeto base2 =	{ 0.0f, 0.0f, 0.11f,	 0.0f,		 0.0f,		0.05f, 0.05f, 0.30f,		0,				0 };		// segundo brazo (cubo)


typedef struct {
	float px, py, pz;
} punto;


//				px	   py	  pz
punto pview = { 0.0f, 0.0f,  0.0f };
punto  pluz = { 0.0f, 0.0f,  10.0f };


// Funciones de dibujo generales
void dibujaCubo();
void dibujaCuadrado();
void dibujaEsfera();

// Funciones de dibujo de la grua
void dibujaSuelo(unsigned int transformLoc);
void dibujaPiscina(unsigned int transformLoc);
void dibujaBrazoArticulacion(unsigned int transformLoc, glm::mat4* transformtemp, objeto obj, int flag);

// Funciones de camara
void myPrimeraPersona(float posicionx, float posiciony, float posicionz, float angulo);
void myCamaraExterior();
void myTerceraPersona(float posicionx, float posiciony, float posicionz, float angulo);

// Funcion de iluminacion
void iluminacion(int id);

// Funcion de texturas
void cargarTextura(const char* nombreText, unsigned int* texture);

// Funciones de opengl y configuraciones
void openGlInit();
void restriccionesMovimiento();
extern GLuint setShaders(const char* nVertx, const char* nFrag);
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
void processInput(GLFWwindow* window);
void frameBufferSizeCallback(GLFWwindow* window, int width, int height);


int main()
{

	// glfw: initialize and configure
	// ------------------------------
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	//Creo la ventana
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "GRUA", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, frameBufferSizeCallback);

	// glad: load all OpenGL function pointers
	// ---------------------------------------
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	glfwSetKeyCallback(window, keyCallback); // Registro la funcion keyCallBack
	openGlInit();

	//generar shader
	shaderProgram = setShaders("shader.vert", "shader.frag");

	// Genero los VAOs
	dibujaCuadrado();
	dibujaCubo();
	dibujaEsfera();

	// Cargo las texturas
	cargarTextura("hierba.jpg", &texturaSuelo);
	cargarTextura("grua.jpg", &texturaGrua);
	cargarTextura("caust00.png", &(texturaPiscina[0]));
	cargarTextura("caust01.png", &(texturaPiscina[1]));
	cargarTextura("caust02.png", &(texturaPiscina[2]));
	cargarTextura("caust03.png", &(texturaPiscina[3]));
	cargarTextura("caust04.png", &(texturaPiscina[4]));
	cargarTextura("caust05.png", &(texturaPiscina[5]));
	cargarTextura("caust06.png", &(texturaPiscina[6]));
	cargarTextura("caust07.png", &(texturaPiscina[7]));
	cargarTextura("caust08.png", &(texturaPiscina[8]));
	cargarTextura("caust09.png", &(texturaPiscina[9]));
	cargarTextura("caust10.png", &(texturaPiscina[10]));
	cargarTextura("caust011.png", &(texturaPiscina[11]));
	cargarTextura("caust12.png", &(texturaPiscina[12]));
	cargarTextura("caust13.png", &(texturaPiscina[13]));
	cargarTextura("caust14.png", &(texturaPiscina[14]));
	cargarTextura("caust15.png", &(texturaPiscina[15]));

	// Asigno las listas de render a los VAOs
	base.listaRender = VAOCubo; base.nVertices = 36;
	baseA1.listaRender = VAOEsfera; baseA1.nVertices = 1080;
	base1.listaRender = VAOCubo; base1.nVertices = 36;
	baseA2.listaRender = VAOEsfera; baseA2.nVertices = 1080;
	base2.listaRender = VAOCubo; base2.nVertices = 36;

	glUseProgram(shaderProgram);
	glUniform1i(glGetUniformLocation(shaderProgram, "texture1"), 0);

	// Lazo de la ventana mientras no la cierre
	// -----------
	while (!glfwWindowShouldClose(window))
	{
		// input
		// -----
		processInput(window);

		// render
		// ------
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f); //Borro el Buffer the la ventana
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		switch (camara) {	// En funcion del valor de camara se emplea un tipo de camara
		case 1:
			myPrimeraPersona(base.px, base.py, base.pz, base.angulo_trans);
			break;
		case 2:
			myCamaraExterior();
			break;
		case 3:
			myTerceraPersona(base.px, base.py, base.pz, base.angulo_trans);
			break;
		}

		glm::mat4 transform;		// es la matriz de transformacion que se aplica a los objetos
		glm::mat4 transformtemp;	// es la matriz de transformacion temporal (necesaria porque no hay pila como en 1.2)

		// la busco en el shader
		unsigned int transformLoc = glGetUniformLocation(shaderProgram, "transform");


		// Dibujo suelo
		dibujaPiscina(transformLoc);
		dibujaSuelo(transformLoc);
		
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		// Base
		base.px += velocidad * cos(base.angulo_trans * ARADIANES); // Actualizamos sus posicion en los ejes en funcion de la velocidad
		base.py += velocidad * sin(base.angulo_trans * ARADIANES);

		transform = glm::mat4(); // identity matrix
		transform = glm::rotate(transform, (float)(angulo * ARADIANES), glm::vec3(1.0f, 0.0f, 0.0f));
		transform = glm::translate(transform, glm::vec3(base.px, base.py, base.pz));
		transform = glm::rotate(transform, (float)(base.angulo_trans * ARADIANES), glm::vec3(0.0f, 0.0f, 1.0f)); // para que gire el objeto
		transformtemp = transform;
		transform = glm::scale(transform, glm::vec3(base.sx, base.sy, base.sz));
		glEnable(GL_TEXTURE_2D);
		iluminacion(0);
		glBindTexture(GL_TEXTURE_2D, texturaGrua);
		glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transform));
		glBindVertexArray(base.listaRender);
		glDrawArrays(GL_TRIANGLES, 0, base.nVertices);
		glBindTexture(GL_TEXTURE_2D, 0);
		glBindTexture(GL_TEXTURE_2D, 0);

		dibujaBrazoArticulacion(transformLoc, &transformtemp, baseA1, 1);
		dibujaBrazoArticulacion(transformLoc, &transformtemp, base1, 2);
		dibujaBrazoArticulacion(transformLoc, &transformtemp, baseA2, 3);
		dibujaBrazoArticulacion(transformLoc, &transformtemp, base2, 4);

		restriccionesMovimiento();

		// glfw: swap 
		// -------------------------------------------------------------------------------
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glDeleteVertexArrays(1, &VAOEsfera);
	glDeleteVertexArrays(1, &VAOCubo);
	glDeleteVertexArrays(1, &VAOCuadrado);

	glfwTerminate();
	return 0;
}


/* FUNCIONES DE DIBUJO GENERALES*/
void dibujaCubo() {
	unsigned int VBO;


	float vertices[] = {
		// Posiciones			 Normales				Coordenadas de texturas
		// Triangulo 1
		-0.5f, -0.5f, -0.5f,	-1.0f, 0.0f, 0.0f,		0.0f, 0.0f,
		-0.5f, -0.5f, 0.5f,		-1.0f, 0.0f, 0.0f,		1.0f, 0.0f,
		-0.5f, 0.5f, 0.5f,		-1.0f, 0.0f, 0.0f,		1.0f, 1.0f,

		// Triangulo 2
		0.5f, 0.5f, -0.5f,      0.0f, 0.0f, -1.0f,		0.0f, 0.0f,
		-0.5f, -0.5f, -0.5f,    0.0f, 0.0f, -1.0f,		1.0f, 0.0f,
		-0.5f, 0.5f, -0.5f,     0.0f, 0.0f, -1.0f,		1.0f, 1.0f,

		// Triangulo 3
		0.5f, -0.5f, 0.5f,      0.0f, -1.0f, 0.0f,		0.0f, 0.0f,
		-0.5f, -0.5f, -0.5f,    0.0f, -1.0f, 0.0f,		1.0f, 0.0f,
		0.5f, -0.5f, -0.5f,     0.0f, -1.0f, 0.0f,		1.0f, 1.0f,

		// Triangulo 4
		0.5f, 0.5f, -0.5f,      0.0f, 0.0f, -1.0f,		0.0f, 0.0f,
		0.5f, -0.5f, -0.5f,     0.0f, 0.0f, -1.0f,		1.0f, 0.0f,
		-0.5f, -0.5f, -0.5f,    0.0f, 0.0f, -1.0f,		1.0f, 1.0f,

		// Triangulo 5
		-0.5f, -0.5f, -0.5f,	0.0f, 0.0f, -1.0f,		0.0f, 0.0f,
		-0.5f, 0.5f, 0.5f,		0.0f, 0.0f, -1.0f,		1.0f, 0.0f,
		-0.5f, 0.5f, -0.5f,		0.0f, 0.0f, -1.0f,		1.0f, 1.0f,

		// Triangulo 6
		0.5f, -0.5f, 0.5f,		0.0f, -1.0f, 0.0f,		0.0f, 0.0f,
		-0.5f, -0.5f, 0.5f,		0.0f, -1.0f, 0.0f,		1.0f, 0.0f,
		-0.5f, -0.5f, -0.5f,	0.0f, -1.0f, 0.0f,		1.0f, 1.0f,

		// Triangulo 7
		-0.5f, 0.5f, 0.5f,		0.0f, 1.0f, 0.0f,		0.0f, 0.0f,
		-0.5f, -0.5f, 0.5f,		0.0f, 1.0f, 0.0f,		1.0f, 0.0f,
		0.5f, -0.5f, 0.5f,		0.0f, 1.0f, 0.0f,		1.0f, 1.0f,

		// Triangulo 8
		0.5f, 0.5f, 0.5f,		1.0f, 0.0f, 0.0f,		0.0f, 0.0f,
		0.5f, -0.5f, -0.5f,		1.0f, 0.0f, 0.0f,		1.0f, 0.0f,
		0.5f, 0.5f, -0.5f,		1.0f, 0.0f, 0.0f,		1.0f, 1.0f,

		// Triangulo 9
		0.5f, -0.5f, -0.5f,		0.0f, -1.0f,  0.0f,		0.0f, 0.0f,
		0.5f, 0.5f, 0.5f,		0.0f, -1.0f,  0.0f,		1.0f, 0.0f,
		0.5f, -0.5f, 0.5f,		0.0f, -1.0f,  0.0f,		1.0f, 1.0f,

		// Triangulo 10
		0.5f, 0.5f, 0.5f,		1.0f,  0.0f,  0.0f,		0.0f, 0.0f,
		0.5f, 0.5f, -0.5f,		1.0f,  0.0f,  0.0f,		1.0f, 0.0f,
		-0.5f, 0.5f, -0.5f,		1.0f,  0.0f,  0.0f,		1.0f, 1.0f,

		// Triangulo 11
		0.5f, 0.5f, 0.5f,		0.0f,  1.0f,  0.0f,		0.0f, 0.0f,
		-0.5f, 0.5f, -0.5f,		0.0f,  1.0f,  0.0f,		1.0f, 0.0f,
		-0.5f, 0.5f, 0.5f,		0.0f,  1.0f,  0.0f,		1.0f, 1.0f,

		// Triangulo 12
		0.5f, 0.5f, 0.5f,		0.0f,  0.0f,  1.0f,		0.0f, 0.0f,
		-0.5f, 0.5f, 0.5f,		0.0f,  0.0f,  1.0f,		1.0f, 0.0f,
		0.5f, -0.5f, 0.5f,		0.0f,  0.0f,  1.0f,		1.0f, 1.0f
	};

	glGenVertexArrays(1, &VAOCubo);
	glGenBuffers(1, &VBO);

	glBindVertexArray(VAOCubo);


	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// Posicion de los vertices
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	// Normales
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	// Texturas
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);


	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}


void dibujaCuadrado() {
	unsigned int VBO;  // Vertex Buffer Object

	// Se dibuja como dos triangulos
	float  vertices[] = {
		//	Posicion				  Normales				Texturas
			-0.5f, -0.5f, -0.5f,	  0.0f, 0.0f, -1.0f,	0.0f, 0.0f,
			 0.5f, -0.5f, -0.5f,	  0.0f, 0.0f, -1.0f,	1.0f, 0.0f,
			 0.5f,  0.5f, -0.5f,	  0.0f, 0.0f, -1.0f,	1.0f, 1.0f,
			 0.5f,  0.5f, -0.5f,	  0.0f, 0.0f, -1.0f,	1.0f, 1.0f,
			-0.5f,  0.5f, -0.5f,	  0.0f, 0.0f, -1.0f,	0.0f, 1.0f,
			-0.5f, -0.5f, -0.5f,	  0.0f, 0.0f, -1.0f,	0.0f, 0.0f
	};

	glGenVertexArrays(1, &VAOCuadrado);
	glGenBuffers(1, &VBO);

	glBindVertexArray(VAOCuadrado);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// Posicion de los vértices
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	// Normales
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	// Texturas
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

}


void dibujaEsfera() {
	unsigned int VBO;	// Vertex Buffer Object

	glGenVertexArrays(1, &VAOEsfera);
	glGenBuffers(1, &VBO);

	glBindVertexArray(VAOEsfera);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices_esfera), vertices_esfera, GL_STATIC_DRAW);	// A partir de la variable global de esfera.h

	//Normales
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);

	//Texturas
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(2);

	//Vertices
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float)));
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

}

/* FUNCIONES DE DIBUJO PARA LA GRUA*/

void dibujaSuelo(unsigned int transformLoc) {
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	glm::mat4 transform;

	float i, j, escalaSuelo = 4.0f;
	for (i = -2; i <= 2; i += (1.0f / escalaSuelo)) {		// cubre el suelo desde -2 a 2 en x
		for (j = -2; j <= 2; j += (1.0f / escalaSuelo)) {	// cubre el suelo desde -2 a 2 en y

			// Calculo la matriz
			transform = glm::mat4(); // identity matrix
			transform = glm::rotate(transform, (float)(angulo * ARADIANES), glm::vec3(1.0f, 0.0f, 0.0f)); // hago el rotate antes para girar todos los cuadrados juntos (si no giraria cada cuadrado individualmente)
			transform = glm::translate(transform, glm::vec3(i, j, 0.0f));
			transform = glm::scale(transform, glm::vec3((1 / escalaSuelo), (1 / escalaSuelo), (1 / escalaSuelo)));

			// Cargo la matriz de transformacion en el shader
			glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transform));

			glBindVertexArray(VAOCuadrado);

			// Cargo la textura
			glBindTexture(GL_TEXTURE_2D, texturaSuelo);
			
			
			// Lo dibujo
			glDrawArrays(GL_TRIANGLES, 0, 6);

			glBindTexture(GL_TEXTURE_2D, 0);
		}
	}
	
}

void dibujaPiscina(unsigned int transformLoc) {
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBlendColor(1.0f, 1.f, 1.f, 0.5f);

	glm::mat4 transform;

	float i, j, escalaSuelo = 4.0f;
	for (i = -2; i <= 2; i += (1.0f / escalaSuelo)) {
		for (j = -2; j <= 2; j += (1.0f / escalaSuelo)) {

			// Calculo la matriz
			transform = glm::mat4(); // identity matrix
			transform = glm::rotate(transform, (float)(angulo * ARADIANES), glm::vec3(1.0f, 0.0f, 0.0f)); // hago el rotate antes para girar todos los cuadrados juntos (si no giraria cada cuadrado individualmente)
			transform = glm::translate(transform, glm::vec3(i, j, 0.0f));
			transform = glm::scale(transform, glm::vec3((1 / escalaSuelo), (1 / escalaSuelo), (1 / escalaSuelo)));

			// Cargo la matriz de transformacion en el shader
			glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transform));

			glBindVertexArray(VAOCuadrado);

			// Cargo la textura
			glBindTexture(GL_TEXTURE_2D, texturaPiscina[indiceTexturaPiscina]);

			// Lo dibujo
			if (i >= 0.8f && i < 1.6f && j < 0.4f && j >= -0.4f) {
				glDrawArrays(GL_TRIANGLES, 0, 6);
			}

			glBindTexture(GL_TEXTURE_2D, 0);
		}
	}

	glDisable(GL_BLEND);

	cuentaPiscina++;
	if (cuentaPiscina == 10) {
		cuentaPiscina = 0;
		indiceTexturaPiscina++;
		if (indiceTexturaPiscina > 15) indiceTexturaPiscina = 0;
	}
}

void dibujaBrazoArticulacion(unsigned int transformLoc, glm::mat4* transformtemp, objeto obj, int id) {
	glm::mat4 transform;
	transform = glm::mat4(); // cargo la identidad
	transform = *transformtemp;
	transform = glm::translate(transform, glm::vec3(obj.px, obj.py, obj.pz));
	if (obj.listaRender == VAOEsfera) { // si es una articulacion, roto para poder mover el brazo
		transform = glm::rotate(transform, (float)(obj.angulo_trans * ARADIANES), glm::vec3(1.0f, 0.0f, 0.0f));
		transform = glm::rotate(transform, (float)(obj.angulo_trans_2 * ARADIANES), glm::vec3(0.0f, 1.0f, 0.0f));
	}

	if (id == 4) {
		float* pSource = (float*)glm::value_ptr(transform);
		float dArray[16] = { 0.0 };
		for (int i = 0; i < 16; i++) {
			dArray[i] = pSource[i];
		}

		pluz.px = (float)dArray[12];
		pluz.py = (float)dArray[13];
		pluz.pz = (float)dArray[14];
	}

	*transformtemp = transform;
	transform = glm::scale(transform, glm::vec3(obj.sx, obj.sy, obj.sz));
	glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transform));

	iluminacion(id);
	glBindTexture(GL_TEXTURE_2D, texturaGrua);
	glBindVertexArray(obj.listaRender);
	glDrawArrays(GL_TRIANGLES, 0, obj.nVertices);
	glBindTexture(GL_TEXTURE_2D, 0);

	
}


/* FUNCIONES DE LA CAMARA*/
void myPrimeraPersona(float posicionx, float posiciony, float posicionz, float angulo) {
	glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);

	glm::mat4 view;	// donde tengo que colocar la camara
	view = glm::mat4();
	view = glm::lookAt(glm::vec3(posicionx, posiciony, posicionz + 0.2),
		glm::vec3(posicionx + (1.5 * cos(angulo * ARADIANES)), posiciony + (1.5 * sin(angulo * ARADIANES)), posicionz + 0.2),
		glm::vec3(0.0f, 0.0f, 1.0f));
	unsigned int viewLoc = glGetUniformLocation(shaderProgram, "view");
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

	glm::mat4 projection;	// matriz de perspectiva
	projection = glm::mat4();
	projection = glm::perspective(glm::radians(60.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 5.0f);
	unsigned int proyectionLoc = glGetUniformLocation(shaderProgram, "projection");
	glUniformMatrix4fv(proyectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
}

void myTerceraPersona(float posicionx, float posiciony, float posicionz, float angulo) {
	glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);

	glm::mat4 view; //Donde tengo que colocar la camara
	view = glm::mat4();
	view = glm::lookAt(
		glm::vec3(posicionx - 0.5 * cos(angulo * ARADIANES), posiciony - 0.5 * sin(angulo * ARADIANES), posicionz + 0.4),
		glm::vec3(posicionx + 10 * cos(angulo * ARADIANES), posiciony + 10 * sin(angulo * ARADIANES), posicionz),
		glm::vec3(0.0f, 0.0f, 1.0f)); //Esta dibujado al reves en el eje z
	unsigned int viewLoc = glGetUniformLocation(shaderProgram, "view");
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

	glm::mat4 projection; // matriz de perspectiva
	projection = glm::mat4();
	projection = glm::perspective(glm::radians(60.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.01f, 5.0f);
	unsigned int proyectionLoc = glGetUniformLocation(shaderProgram, "projection");
	glUniformMatrix4fv(proyectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
}

void myCamaraExterior() {
	glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);

	glm::mat4 view;	//Donde tengo que colocar la camara
	view = glm::mat4();
	view = glm::lookAt(glm::vec3(.0f, .0f, 3.0), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	unsigned int viewLoc = glGetUniformLocation(shaderProgram, "view");
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

	glm::mat4 projection;	// matriz de perspectiva
	projection = glm::mat4();
	projection = glm::perspective(glm::radians(60.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 5.0f);
	unsigned int proyectionLoc = glGetUniformLocation(shaderProgram, "projection");
	glUniformMatrix4fv(proyectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
}

/* FUNCION DE ILUMINACION */
void iluminacion(int id) {
	// Cargamos las textura de iluminacion
	glActiveTexture(GL_TEXTURE0 + id);
	unsigned int textureLoc = glGetUniformLocation(shaderProgram, "texture1");
	glUniform1i(textureLoc, id);

	// El color del objeto
	unsigned int colorLoc = glGetUniformLocation(shaderProgram, "objectColor");
	glUniform3f(colorLoc, 1.0f, 1.0f, 1.0f);

	// El color de la luz ambiente
	unsigned int lightLoc = glGetUniformLocation(shaderProgram, "lightColor");
	glUniform3f(lightLoc, 1.0f, 1.0f, 1.0f);

	// Luz difusa
	unsigned int lightPosLoc = glGetUniformLocation(shaderProgram, "lightPos");
	glUniform3f(lightPosLoc, (float)pluz.px, (float)pluz.py, (float)(pluz.pz + 4));

	unsigned int viewPosLoc = glGetUniformLocation(shaderProgram, "viewPos");
	glUniform3f(viewPosLoc, (float)pview.px, (float)pview.py, (float)(pview.pz+4));
}


/* FUNCION DE TEXTURAS */
void cargarTextura(const char* nombreText, unsigned int* texture) {
	glGenTextures(1, texture);
	glBindTexture(GL_TEXTURE_2D, *texture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);


	int width, height, nrChannels;
	unsigned char* data = stbi_load(nombreText, &width, &height, &nrChannels, 0);
	int format;
	if (data) {
		if (nrChannels == 3)
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		}
		else
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		}
		glGenerateMipmap(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
	else{
		std::cout << "Failed to load texture" << std::endl;
	}
	stbi_image_free(data);
}


/* FUNCIONES DE OPENGL Y CONFIGURACIONES */
void openGlInit() {
	//Habilito aqui el depth en vez de arriba para los usuarios de linux y mac mejor arriba
	//Incializaciones varias
	glClearDepth(1.0f); //Valor z-buffer
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);  // valor limpieza buffer color
	glEnable(GL_DEPTH_TEST); // z-buffer
	glEnable(GL_CULL_FACE); //ocultacion caras back
	glCullFace(GL_BACK);
}

void restriccionesMovimiento() {
	// No puede salirse del terreno

	// Definimos los límites de la piscina
	float map_left = -2.0f;
	float map_right = 2.0f;
	float map_bottom = -2.0f;
	float map_top = 2.0f;
	// Definimos el margen de colisión
	float collision_margin = 0.02;

	if (base.px >= map_right) base.px -= collision_margin;
	else if (base.px <= map_left) base.px += collision_margin;

	if (base.py >= map_top) base.py -= collision_margin;
	else if (base.py <= map_bottom) base.py += collision_margin;

	// No puede pasar por encima de la piscina
	
	// Definimos los límites de la piscina
	float pool_left = 0.6;
	float pool_right = 1.6;
	float pool_bottom = -0.4;
	float pool_top = 0.4;

	// Si la posición del objeto base se encuentra dentro del perímetro de la piscina, lo retrocedemos en la dirección en la que se estaba pasando del perímetro
	if (base.px > pool_left && base.px < pool_right && base.py > pool_bottom && base.py < pool_top) {
		float dx = std::min(std::abs(base.px - pool_left), std::abs(base.px - pool_right)); // Distancia a los lados de la piscina
		float dy = std::min(std::abs(base.py - pool_bottom), std::abs(base.py - pool_top)); // Distancia a los bordes de la piscina

		// Comprobamos en qué dirección está el objeto base en relación a la piscina
		if (dx < dy) { // Está en el eje X
			if (base.px < pool_left + collision_margin) { // Colisión en el lado izquierdo
				base.px -= collision_margin;
			}
			else if (base.px > pool_right - collision_margin) { // Colisión en el lado derecho
				base.px += collision_margin;
			}
		}
		else { // Está en el eje Y
			if (base.py < pool_bottom + collision_margin) { // Colisión en el borde inferior
				base.py -= collision_margin;
			}
			else if (base.py > pool_top - collision_margin) { // Colisión en el borde superior
				base.py += collision_margin;
			}
		}
	}
}

void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	std::cout << key << std::endl;

	// Primer brazo
	if (key == 70) baseA1.angulo_trans--;				// F => hacia la izquierda
	if (key == 71) baseA1.angulo_trans_2--;				// G => hacia atras 
	if (key == 84) baseA1.angulo_trans_2++;				// T => hacia delante
	if (key == 72) baseA1.angulo_trans++;				// H => hacia la derecha

	// Segundo brazo
	if (key == GLFW_KEY_UP)	   baseA2.angulo_trans_2++;	// Flecha hacia arriba => Hacia arriba
	if (key == GLFW_KEY_DOWN)  baseA2.angulo_trans_2--;	// Flecha hacia abajo = > Hacia abajo
	if (key == GLFW_KEY_LEFT)  baseA2.angulo_trans--;	// Flecha hacia la izquierda = > Hacia la izquierda
	if (key == GLFW_KEY_RIGHT) baseA2.angulo_trans++;	// Flecha hacia la derecha = > Hacia la derecha

	if (key == 75) angulo++;							// K => En sentido de las agujas del reloj
	if (key == 76) angulo--;							// L => En sentido contrario

	// Base
	if (key == 65) base.angulo_trans++;					// A => giro a la izquierda
	if (key == 68) base.angulo_trans--;					// D => giro a la derecha
	if (key == 87) velocidad += 0.001;					// W => acelerar
	if (key == 88) velocidad -= 0.001;					// X => frenar

	// Camara
	if (key == 49) {									// 1 => Primera persona
		camara = 1;
		angulo = 0.0f;
	}
	if (key == 50) {									// 2 => Camara exterior
		camara = 2;
		angulo = 0.0f;
	}
	if (key == 51) {									// 3 => Tercera persona
		camara = 3;
		angulo = 0.0f;
	}

}

void frameBufferSizeCallback(GLFWwindow* window, int width, int height) {
	SCR_WIDTH = width;
	SCR_HEIGHT = height;
}