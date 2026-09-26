// Práctica 06 - Carga de Modelos 3D y Fondo
// Maldonado Jr. Montoya Gustavo
// Fecha de entrega: 25/09/2026

#include <string>
#include <iostream>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "Shader.h"
#include "Camera.h"
#include "Model.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "SOIL2/SOIL2.h"
#include "stb_image.h"

// Properties
const GLuint WIDTH = 800, HEIGHT = 600;
int SCREEN_WIDTH, SCREEN_HEIGHT;

// Function prototypes
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode);
void MouseCallback(GLFWwindow* window, double xPos, double yPos);
void DoMovement();

// Camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
bool keys[1024];
GLfloat lastX = 400, lastY = 300;
bool firstMouse = true;

GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;

int main()
{
    // Init GLFW
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Practica 6 - Gustavo Maldonado Jr. Montoya", nullptr, nullptr);

    if (nullptr == window)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return EXIT_FAILURE;
    }

    glfwMakeContextCurrent(window);
    glfwGetFramebufferSize(window, &SCREEN_WIDTH, &SCREEN_HEIGHT);

    // Callbacks
    glfwSetKeyCallback(window, KeyCallback);
    glfwSetCursorPosCallback(window, MouseCallback);

    // Mostrar el cursor libremente
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

    glewExperimental = GL_TRUE;
    if (GLEW_OK != glewInit())
    {
        std::cout << "Failed to initialize GLEW" << std::endl;
        return EXIT_FAILURE;
    }

    glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    glEnable(GL_DEPTH_TEST);

    // Carga de Shader único de la plantilla
    Shader shader("Shader/modelLoading.vs", "Shader/modelLoading.frag");

    // Configuración y carga de la Textura de Fondo
    GLuint backgroundTexture;
    glGenTextures(1, &backgroundTexture);
    glBindTexture(GL_TEXTURE_2D, backgroundTexture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int bgWidth, bgHeight, bgChannels;

    // Carga a color detectando canales automáticamente
    unsigned char* bgData = SOIL_load_image("fondo/perritoTomandoSol.jpg", &bgWidth, &bgHeight, &bgChannels, SOIL_LOAD_AUTO);

    if (bgData)
    {
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        GLenum format = (bgChannels == 4) ? GL_RGBA : GL_RGB;

        glTexImage2D(GL_TEXTURE_2D, 0, format, bgWidth, bgHeight, 0, format, GL_UNSIGNED_BYTE, bgData);
        glGenerateMipmap(GL_TEXTURE_2D);
        SOIL_free_image_data(bgData);
        std::cout << "Imagen de fondo cargada exitosamente a color." << std::endl;
    }
    else
    {
        std::cout << "Error al cargar la imagen de fondo: fondo/perritoTomandoSol.jpg" << std::endl;
    }

    // Plano para el fondo
    GLfloat quadVertices[] = {
        // Posiciones 3D       // Normales          // Coordenadas UV
        -1.0f,  1.0f, 0.0f,    0.0f, 0.0f, 1.0f,    0.0f, 0.0f,
        -1.0f, -1.0f, 0.0f,    0.0f, 0.0f, 1.0f,    0.0f, 1.0f,
         1.0f, -1.0f, 0.0f,    0.0f, 0.0f, 1.0f,    1.0f, 1.0f,

        -1.0f,  1.0f, 0.0f,    0.0f, 0.0f, 1.0f,    0.0f, 0.0f,
         1.0f, -1.0f, 0.0f,    0.0f, 0.0f, 1.0f,    1.0f, 1.0f,
         1.0f,  1.0f, 0.0f,    0.0f, 0.0f, 1.0f,    1.0f, 0.0f
    };

    GLuint quadVAO, quadVBO;
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);

    // Atributo 0: Posición
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);

    // Atributo 1: Normales
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));

    // Atributo 2: Coordenadas UV
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat)));
    glBindVertexArray(0);

    // Modelos
    Model perroSalchicha((char*)"Models/perroSalchicha/Dachshund-bl.obj");
    Model dog((char*)"Models/redDog/RedDog.obj");
    Model Helicopter((char*)"Models/helicoptero/Helicopter-bl.obj");
    Model rata((char*)"Models/rata/Rat-bl.obj");
    Model gallina((char*)"Models/gallina/Chicken-bl.obj");

    glm::mat4 projection = glm::perspective(camera.GetZoom(), (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 100.0f);

    // Game loop
    while (!glfwWindowShouldClose(window))
    {
        GLfloat currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        glfwPollEvents();
        DoMovement();

        glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader.Use();

        // -------------------------------------------------------------------------
        // RENDERIZADO DEL PLANO DE FONDO (ENTORNO)
        // -------------------------------------------------------------------------
        glUniform1i(glGetUniformLocation(shader.Program, "useTexture"), true);
        glUniform3f(glGetUniformLocation(shader.Program, "objectColor"), 1.0f, 1.0f, 1.0f);

        glDepthMask(GL_FALSE);

        glm::mat4 identity = glm::mat4(1.0f);
        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(identity));
        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "view"), 1, GL_FALSE, glm::value_ptr(identity));
        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(identity));

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, backgroundTexture);
        glUniform1i(glGetUniformLocation(shader.Program, "texture_diffuse1"), 0);

        glBindVertexArray(quadVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);

        glDepthMask(GL_TRUE);

        // -------------------------------------------------------------------------
        // RENDERIZADO DE LOS MODELOS 3D
        // -------------------------------------------------------------------------
        glm::mat4 view = camera.GetViewMatrix();
        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));

        // 1. HELICÓPTERO
        glm::mat4 modelHeli = glm::mat4(1.0f);
        modelHeli = glm::translate(modelHeli, glm::vec3(-1.0f, 0.5f, 1.0f));
        modelHeli = glm::scale(modelHeli, glm::vec3(0.1f, 0.1f, 0.1f));
        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(modelHeli));
        Helicopter.Draw(shader);

        // 2. PERRO
        glm::mat4 modelDog = glm::mat4(1.0f);
        modelDog = glm::translate(modelDog, glm::vec3(-1.0f, -0.5f, 1.0f));
        modelDog = glm::scale(modelDog, glm::vec3(0.8f, 0.8f, 0.8f));
        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(modelDog));
        dog.Draw(shader);

        // 3. PERRO SALCHICHA
        glm::mat4 modelChicha = glm::mat4(1.0f);
        modelChicha = glm::translate(modelChicha, glm::vec3(-0.5f, -0.5f, 1.0f));
        modelChicha = glm::scale(modelChicha, glm::vec3(0.1f, 0.1f, 0.1f));
        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(modelChicha));
        perroSalchicha.Draw(shader);

        // 4. RATA
        glm::mat4 modelRat = glm::mat4(1.0f);
        modelRat = glm::translate(modelRat, glm::vec3(0.0f, -0.5f, 1.5f));
        modelRat = glm::scale(modelRat, glm::vec3(0.1f, 0.1f, 0.1f));
        modelRat = glm::rotate(modelRat, glm::radians(250.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(modelRat));
        rata.Draw(shader);

        // 5. GALLINA
        glm::mat4 modelGallina = glm::mat4(1.0f);
        modelGallina = glm::translate(modelGallina, glm::vec3(0.5f, -0.3f, 1.7f));
        modelGallina = glm::scale(modelGallina, glm::vec3(0.05f, 0.05f, 0.05f));
        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(modelGallina));
        gallina.Draw(shader);


        glfwSwapBuffers(window);
    }

    glDeleteVertexArrays(1, &quadVAO);
    glDeleteBuffers(1, &quadVBO);

    glfwTerminate();
    return 0;
}

void DoMovement()
{
    if (keys[GLFW_KEY_W] || keys[GLFW_KEY_UP]) camera.ProcessKeyboard(FORWARD, deltaTime);
    if (keys[GLFW_KEY_S] || keys[GLFW_KEY_DOWN]) camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (keys[GLFW_KEY_A] || keys[GLFW_KEY_LEFT]) camera.ProcessKeyboard(LEFT, deltaTime);
    if (keys[GLFW_KEY_D] || keys[GLFW_KEY_RIGHT]) camera.ProcessKeyboard(RIGHT, deltaTime);
}

void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    if (GLFW_KEY_ESCAPE == key && GLFW_PRESS == action) glfwSetWindowShouldClose(window, GL_TRUE);
    if (key >= 0 && key < 1024)
    {
        if (action == GLFW_PRESS) keys[key] = true;
        else if (action == GLFW_RELEASE) keys[key] = false;
    }
}

void MouseCallback(GLFWwindow* window, double xPos, double yPos)
{
    if (firstMouse)
    {
        lastX = xPos;
        lastY = yPos;
        firstMouse = false;
    }
    GLfloat xOffset = xPos - lastX;
    GLfloat yOffset = lastY - yPos;
    lastX = xPos;
    lastY = yPos;
    camera.ProcessMouseMovement(xOffset, yOffset);
}