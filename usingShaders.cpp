#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <../../Shader.h>
#include <iostream>

#define RUN_SPEED  0.08
#define MAX_PARTICLES 100
#define INITIAL_GRAVITY 0.001
#define EMMITTER_SIZE 1
#define SNOW 1
#define SNOW_SIZE 2
#define RAIN 2
#define RAIN_SIZE 0.7

#define KEYS 9
#define W 0
#define A 1
#define S 2
#define D 3
#define UP 4
#define DOWN 5
#define LEFT 6
#define RIGHT 7
#define R 8

GLdouble width = 1400;
GLdouble height = 1000;
GLdouble yaw = 90;
GLdouble pitch = 0;              
GLdouble lastX = (double)width / 2;
GLdouble lastY = (double)height/2;

int particles = 0;
int current_Max_Particles = 100;

GLfloat initHeight = 4;
GLfloat snowVel = 0.01;
GLfloat gravity = INITIAL_GRAVITY;
GLfloat  lookX, lookY = 0;

bool snowing = true;
bool raining = false;
bool falling = false;
bool keyStates[KEYS];
bool firstMouse = true;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window, int key, int scancode, int action, int mods);
void calculate_lookpoint(void);
void mouse_motion(GLFWwindow* window, double x, double y);

//A general particle structure
struct particle {
    glm::vec3 position;
    glm::vec2 velocity;
    float acceleration;
    float timeToLive;
    int size;
    int type;
};

struct particle pSystem[MAX_PARTICLES];

/*
Create a snowflake that falls at a constant speed that is relative to its size.
On reaching the floor it begins to melt in time set by the user
*/
void initSnow(int i) {
    pSystem[i].timeToLive = 1;
    pSystem[i].type = SNOW;
    pSystem[i].position.y = initHeight + (4 * (float)rand() / RAND_MAX);
    pSystem[i].position.x = EMMITTER_SIZE * ((float)rand() / RAND_MAX - 2 * ((float)rand() / RAND_MAX));
    pSystem[i].position.z = EMMITTER_SIZE * ((float)rand() / RAND_MAX - 2 * ((float)rand() / RAND_MAX));
    pSystem[i].size = SNOW_SIZE * (1 + (float)rand() / RAND_MAX);
    pSystem[i].velocity.y = snowVel;
    pSystem[i].velocity.x = 0;
    pSystem[i].acceleration = gravity / 100;
}

//Camera
glm::vec3 cameraPos = glm::vec3(0.0f, 1.7f, 0.0f);
glm::vec3 front;
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, 1.0f);
glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
glm::vec3 cameraUp = up;
glm::vec3 cameraRight;

// timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;



int main()
{
    //Uints to be allocated as vertex array and buffer objects
    unsigned int VBO, EBO, VAO, particleVBuffer, particleVArray;

    //Initialise projection and view matrices
    glm::mat4 projection = glm::perspective(glm::radians(50.0f), (float)width / (float)height, 0.1f, 80.0f);
    glm::mat4 view = glm::mat4(1.0f);

    //Initialise GLFW
    glfwInit();

    //Create a window and check if successful
    GLFWwindow* window = glfwCreateWindow(width, height, "Particle System", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    
    //load all OpenGL function pointers adn enable depth test
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
    glEnable(GL_DEPTH_TEST);

    //Set callback functions
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetKeyCallback(window, processInput);
    glfwSetCursorPosCallback(window, mouse_motion);

    //Compile and link  our shaders
    Shader floorShader("FloorVShader.txt", "FloorFShader.txt"); // you can name your shader files however you like
    Shader particleShader("ParticleVertexShader.txt", "ParticleFragmentShader.txt");
   
    //Vertices of the floor, and indices to draw it 
    float floor_vertices[] = {
         20.0f,  0.0f, 20.0f,  
        -20.0f, 0.0f, -20.0f,
         20.0f, 0.0f, -20.0f, 
        -20.0f,  0.0f, 20.0f    
    };
    unsigned int floor_indices[] = {  
        0, 2, 3,
        2, 1, 3   
    };

    //Point to use for template for particles
    float particlePoint[] = {
         0.0f,  0.0f, 0.0f,  // top right
    };

    //Generate the required vertex arrays  
    glGenVertexArrays(1, &VAO);
    glGenVertexArrays(1, &particleVArray);

    //Generate the required vertex buffers
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &particleVBuffer);
    glGenBuffers(1, &EBO);
    
    //Bind the floor rendering information into VAO  
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(floor_vertices), floor_vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(floor_indices), floor_indices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    //Bind the particle rendering information into particleVArray
    glBindVertexArray(particleVArray);
    glBindBuffer(GL_ARRAY_BUFFER, particleVBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(particlePoint), particlePoint, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    //Hide the cursor
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    
    while (!glfwWindowShouldClose(window))
    {
        //Clear colour and depth buffer
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        //Update the view matrix
        view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        
        //Draw the floor
        floorShader.use();
        floorShader.setMat4("view", view);
        floorShader.setMat4("projection", projection);
        floorShader.setInt("mode", 0);
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
          
        //Draw the grid lines
        floorShader.setInt("mode", 1);
        glLineWidth(2);
        for (float x = -20; x <= 20; x++) {
            floorShader.setFloat("x", x);

            glDrawArrays(GL_LINES, 0, 6);
        }
        floorShader.setInt("mode", 2);
        for (float z = -20; z <= 20; z++) {
            floorShader.setFloat("z", z);
            glDrawArrays(GL_LINES, 0, 6);
        }
        
        //Draw the particles
        particleShader.use();
        particleShader.setMat4("view", view);
        particleShader.setMat4("projection", projection);
        glBindVertexArray(particleVArray);            
        if (particles < current_Max_Particles && falling)
            particles += 1;
        for (int i = 0; i < particles; i++) {
            if (falling) {
                if (pSystem[i].timeToLive > 0 && !keyStates[R]) {
                    if (pSystem[i].position.y > 0.02) {
                        pSystem[i].position.y -= pSystem[i].velocity.y;
                        pSystem[i].position.x -= pSystem[i].velocity.x;
                        pSystem[i].velocity.y += pSystem[i].acceleration;
                    }
                    else if (pSystem[i].type == SNOW) {
                        pSystem[i].timeToLive -= 0.01;
                        pSystem[i].size *= pSystem[i].timeToLive;
                    }
                }
                else if (snowing)
                    initSnow(i);
                else if (raining);
            }
            particleShader.setVec3("position", pSystem[i].position);
            glPointSize(pSystem[i].size);
            glDrawArrays(GL_POINTS, 0, 3);
        }
        
        if (keyStates[W]) {
            cameraPos.x = cameraPos.x + (cos(glm::radians(yaw)) * RUN_SPEED);
            cameraPos.z = cameraPos.z + (sin(glm::radians(yaw))* RUN_SPEED);
        }
        if (keyStates[A]) {
            cameraPos.x = cameraPos.x + (sin(glm::radians(yaw)) * RUN_SPEED);
            cameraPos.z = cameraPos.z - (cos(glm::radians(yaw))* RUN_SPEED);
        }
        if (keyStates[S]) {
            cameraPos.x = cameraPos.x - (cos(glm::radians(yaw)) * RUN_SPEED);
            cameraPos.z = cameraPos.z - (sin(glm::radians(yaw)) * RUN_SPEED);
        }
        if (keyStates[D]) {
            cameraPos.x = cameraPos.x - (sin(glm::radians(yaw)) * RUN_SPEED);
            cameraPos.z = cameraPos.z + (cos(glm::radians(yaw)) * RUN_SPEED);
        }
        if (keyStates[R]) {
            particles = 0;
            keyStates[R] = false;
            falling = false;
        }

        //Check and call events adn swap buffers
        glfwPollEvents();
        glfwSwapBuffers(window);
        
    }

    //De-allocate resources that have outlived thier purpose
    glDeleteVertexArrays(1, &VAO);
    glDeleteVertexArrays(1, &particleVArray);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &particleVBuffer);
    glDeleteBuffers(1, &EBO);

    glfwTerminate();
    return 0;
}

//Fuction called whenever the window is resized
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

//Function called when a key is pressed
void processInput(GLFWwindow* window, int key, int scancode, int action, int mods)
{

    switch (key) {
    
        //Press escape to exit 
        case GLFW_KEY_ESCAPE:
            glfwSetWindowShouldClose(window, true);
            break;

        //Press Space to pause
        case GLFW_KEY_SPACE:
            if (action == GLFW_PRESS)
                falling = !falling;
            break;

        //Press 1 to make it snow
        case GLFW_KEY_1:
            if (action == GLFW_PRESS) {
                snowing = true;
                raining = false;
            }
            break;

        //Press 2 to make it rain
        case GLFW_KEY_2:
            if (action == GLFW_PRESS) {
                snowing = false;
                raining = true;
            }
            break;

        //Press P to increase max number of particles
        case GLFW_KEY_P:
            if(action == GLFW_PRESS) {
                if (current_Max_Particles < MAX_PARTICLES)
                    current_Max_Particles += 10;
            }
            break;

        //Press R to reset the simulation
        case GLFW_KEY_R:
            if (action == GLFW_PRESS) {
                gravity = INITIAL_GRAVITY;
                keyStates[R] = true;
                falling = true;
            }
            break;

        //Use WASD to move
        case GLFW_KEY_W:
            if (action == GLFW_PRESS) { keyStates[W] = true; }
            else if (action == GLFW_RELEASE) { keyStates[W] = false; }
            break;
        case GLFW_KEY_A:
            if (action == GLFW_PRESS) { keyStates[A] = true; }
            else if (action == GLFW_RELEASE) { keyStates[A] = false; }
            break;
        case GLFW_KEY_S:
            if (action == GLFW_PRESS) { keyStates[S] = true; }
            else if (action == GLFW_RELEASE) { keyStates[S] = false; }
            break;
        case GLFW_KEY_D:
            if (action == GLFW_PRESS) { keyStates[D] = true; }
            else if (action == GLFW_RELEASE) { keyStates[D] = false; }
    }  
}

//Function called when the mouse is moved
void mouse_motion(GLFWwindow* window, double x, double y) {
    
    if (firstMouse)
    {
        lastX = x;
        lastY = y;
        firstMouse = false;
    }

    float xoffset = x - lastX;
    float yoffset = lastY - y;
    lastX = x;
    lastY = y;

    float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;

    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(front);
    cameraRight = glm::normalize(glm::cross(cameraFront, up));
    cameraUp = glm::normalize(glm::cross(cameraRight, cameraFront));    
}
