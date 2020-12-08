#include<GL/glut.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h> 

/* Set to 0 or 1 for normal or reversed mouse Y direction */
#define INVERT_MOUSE 0

#define RUN_SPEED  0.08
#define TURN_ANGLE 4.0
#define DEG_TO_RAD 0.017453293
#define MAX_PARTICLES 500000
#define EMMITTER_SIZE 10
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

GLdouble lat, lon;              /* View angles (degrees)    */
GLdouble mlat, mlon;             /* Mouse look offset angles */
GLfloat  eyex, eyey, eyez;    /* Eye point                */
GLfloat  centerx, centery, centerz; /* Look point               */
GLfloat  upx, upy, upz;     /* View up vector           */
GLfloat  lookX, lookY = 0;
bool keystates[KEYS];
int particles = 0;

GLint width = 1400, height = 1000;      /* size of window           */
GLint falling = true;		    //play and pause simulation
GLint WALKING = 0;	    	    /* Representing the walking state */
GLint state = 0;
bool snowing = true;
bool raining = false;
GLfloat initHeight = 4;
GLfloat snowVel = 0.01;
GLfloat rainVel = 0.01;
GLfloat initialGravity = 0.001;
GLfloat gravity = 0.001;
GLfloat wind = 0;
int current_Max_Particles = 800;

//////////////////////////////////////////////

struct particle {
    GLfloat timeToLive;
    GLint type;
    GLfloat x;
    GLfloat y;
    GLfloat z;
    GLdouble size;
    GLfloat yVelocity;
    GLfloat xVelocity;
    //TODO add zVelocity in small random range
    GLfloat acc;
};

struct particle pSystem[MAX_PARTICLES];

/* 
Create a snowflake that falls at a constant speed that is relative to its size.
On reaching the floor it begins to melt in time set by the user
*/
void initSnow(int i) {
    pSystem[i].timeToLive = 1;
    pSystem[i].type = SNOW;
    pSystem[i].y = initHeight + (4 * (float)rand() / RAND_MAX);
    pSystem[i].x = EMMITTER_SIZE * ((float)rand() / RAND_MAX - 2 * ((float)rand() / RAND_MAX));
    pSystem[i].z = EMMITTER_SIZE * ((float)rand() / RAND_MAX - 2 * ((float)rand() / RAND_MAX));
    pSystem[i].size = SNOW_SIZE * (1 + (float)rand()/RAND_MAX);
    pSystem[i].yVelocity = snowVel;
    pSystem[i].xVelocity = 0;
    pSystem[i].acc = gravity/100;
}

//Draw a snow particle
void drawSnow(particle snow) {
    glDisable(GL_LIGHTING);
    glEnable(GL_POINT_SMOOTH);
    glPointSize(snow.size);
    glColor3f(1, 1, 1);
    glBegin(GL_POINTS);
    glVertex3f(snow.x, snow.y, snow.z);
    glEnd();
    glEnable(GL_LIGHTING);
}

/*
Create a raindrop that falls with constant acceleration due to gravity
And has a random starting velocity based on its size
*/
void initRain(int i) {
    pSystem[i].timeToLive = 0.09;
    pSystem[i].type = RAIN;
    pSystem[i].y = initHeight + (4 * (float)rand() / RAND_MAX);
    pSystem[i].x = EMMITTER_SIZE * ((float)rand() / RAND_MAX - 2 * ((float)rand() / RAND_MAX));
    pSystem[i].z = EMMITTER_SIZE * ((float)rand() / RAND_MAX - 2 * ((float)rand() / RAND_MAX));
    pSystem[i].size = RAIN_SIZE * (1 + (float)rand() / RAND_MAX);
    pSystem[i].yVelocity = rainVel;
    pSystem[i].xVelocity = wind;
    pSystem[i].acc = gravity;
}
GLfloat white_light[] = { 1.0, 1.0, 1.0, 1.0 };

void drawRain(particle rain) {
    glDisable(GL_LIGHTING);
    //glEnable(GL_POINT_SMOOTH);
    glPointSize(rain.size);
    glColor3f(0, 0, 1);
    if (rain.y > 0.02) {
        glBegin(GL_LINES);
        glVertex3f(rain.x + (rain.xVelocity - wind), rain.y + (rain.yVelocity - rain.acc), rain.z);
        glVertex3f(rain.x, rain.y, rain.z);
        glEnd();
    }
    else {
        glBegin(GL_POINTS);
        glVertex3f(rain.x, rain.y, rain.z);
        glEnd();
    }
    
    glEnable(GL_LIGHTING);
}

void draw_scene(void) {
    // Draws all the elements in the scene
    int x, z;
    int L = 25;

    /* Draw ground */
    glDisable(GL_LIGHTING);
    glDepthRange(0.1, 1.0);
    glColor3f(0.4, 0.4, 0.4);
    glBegin(GL_QUADS);
    glVertex3f(-L, 0, -L);
    glVertex3f(L, 0, -L);
    glVertex3f(L, 0, L);
    glVertex3f(-L, 0, L);
    glEnd();

    glDepthRange(0.0, 0.9);
    glColor3f(0.2, 0.2, 0.2);
    glLineWidth(1.0);
    glBegin(GL_LINES);
    for (x = -L; x <= L; x++) {
        glVertex3f((GLfloat)x, 0.01, -L);
        glVertex3f((GLfloat)x, 0.01, L);
    }
    for (z = -L; z <= L; z++) {
        glVertex3f(-L, 0.01, (GLfloat)z);
        glVertex3f(L, 0.01, (GLfloat)z);
    }
    glEnd();

    glEnable(GL_LIGHTING);

    for (int i = 0; i < particles; i++) {
        if (pSystem[i].type == SNOW)
            drawSnow(pSystem[i]);
        else
            drawRain(pSystem[i]);
    }
} // draw_scene()

void calculate_lookpoint(void) { 

    centerx = eyex + (cos((mlat + lat) * DEG_TO_RAD) * sin((mlon + lon) * DEG_TO_RAD));
    centery = eyey + sin((mlat + lat) * DEG_TO_RAD);
    centerz = eyez + (cos((mlat + lat) * DEG_TO_RAD) * cos((mlon + lon) * DEG_TO_RAD));

} // calculate_lookpoint()

//Display functoin 
void display(void) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //Flush the frame and depth buffer to update window
    glLoadIdentity(); //reset the matrix to defualt
    calculate_lookpoint(); 
    gluLookAt(eyex, eyey, eyez, centerx, centery, centerz, upx, upy, upz);
    draw_scene();
    glutSwapBuffers();
} // display()

//Reshape function 
void reshape(int w, int h) {
    glViewport(0, 0, (GLsizei)w, (GLsizei)h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(50, (GLfloat)w / (GLfloat)h, 0.1, 80.0);
    glMatrixMode(GL_MODELVIEW);
    width = w; 
    height = h;
}

void mouse_motion(int x, int y) {
    lookX += (x - width/2);
    lookY += (y - height/2);
    //lookY = height;
    
    mlat = 50 * (1 - ((2 * lookY) / (float)height));
    if (mlat > 89)
        mlat = 89;
    if (mlat < -89)
        mlat = -89;
    mlon = 50 * (1 - ((2 * lookX) / (float)width));
    
  
} // mouse_motion()

void keyboardDown(unsigned char key, int x, int y) {
    switch (key) {
    case 27:  /* Escape key */
        exit(0);
        break;
    case 32: /* space bar */
        falling = !falling;
        break;
    case 49: /* 1 key */
        snowing = true;
        raining = false;
        break;
    case 50: /* 2 key */
        raining = true;
        snowing = false;
        break;
    case 119: /*w key*/
        keystates[W] = true;
        break;
    case 97: /*a key*/
        keystates[A] = true;
        break;
    case 115: /*s key*/
        keystates[S] = true;
        break;
    case 100: /*d key*/
        keystates[D] = true;
        break;
    case 112: /*p key*/
        if (current_Max_Particles < MAX_PARTICLES)
            current_Max_Particles += 200;
        break;
    case 114: /*r key*/
        gravity = initialGravity;
        wind = 0;
        keystates[R] = true;
        break;
    }
} // keyboardDown()

void keyboardUp(unsigned char key, int x, int y) {
    switch (key) {
    case 119: /*w key*/
        keystates[W] = false;
        break;
    case 97: /*a key*/
        keystates[A] = false;
        break;
    case 115: /*s key*/
        keystates[S] = false;
        break;
    case 100: /*d key*/
        keystates[D] = false;
        break;
    }
} // keyboardUp()

void specialDown(int key, int x, int y) {
    switch (key) {
    case GLUT_KEY_UP:
        keystates[UP] = true;
        break;
    case GLUT_KEY_DOWN:    
        keystates[DOWN] = true;
        break;
    case GLUT_KEY_LEFT:
        keystates[LEFT] = true;
        break;
    case GLUT_KEY_RIGHT:
        keystates[RIGHT] = true;
        break;
    }
}


void specialUp(int key, int x, int y) {
    switch (key) {
    case GLUT_KEY_UP:
        keystates[UP] = false;
        break;
    case GLUT_KEY_DOWN:
        keystates[DOWN] = false;
        break;
    case GLUT_KEY_LEFT:
        keystates[LEFT] = false;
        break;
    case GLUT_KEY_RIGHT:
        keystates[RIGHT] = false;
        break;
    }
}
//////////////////////////////////////////////

void init(void) {
    /* Set initial view parameters */
    eyex = 0.0; /* Set eyepoint at eye height within the scene */
    eyey = 1.7;
    eyez = -10.0;

    upx = 0.0;   /* Set up direction to the +Y axis  */
    upy = 1.0;
    upz = 0.0;

    lat = 0.0;   /* Look horizontally ...  */
    lon = 0.0;   /* ... along the +Z axis  */

    mlat = 0.0;  /* Zero mouse look angles */
    mlon = 0.0;

    /* set up lighting */
    glLightfv(GL_LIGHT0, GL_DIFFUSE, white_light);
    glLightfv(GL_LIGHT0, GL_SPECULAR, white_light);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_DEPTH_TEST);
    glShadeModel(GL_SMOOTH);
    glEnable(GL_NORMALIZE);

    //Create a snowflake 
    //for (int i = 0; i < MAX_PARTICLES; i++) {
        //initSnow(i);
    //}

    //set the keystates to false
    for (int i = 0; i < KEYS; i++) {
        keystates[i] = false;
    }
} // init()

//Function called bu OpenGl called on each glutMainLoop
void idle(void) {
    //Have logic calculating falling speed of snow flake
    if (particles < current_Max_Particles)
        particles += 50;
    for (int i = 0; i < particles; i++) {
        if (falling) {
            if (pSystem[i].timeToLive > 0 && !keystates[R]) {
                if (pSystem[i].y > 0.02) {
                    pSystem[i].y -= pSystem[i].yVelocity;
                    pSystem[i].x -= pSystem[i].xVelocity;
                    pSystem[i].yVelocity += pSystem[i].acc;
                    pSystem[i].xVelocity += wind;
                }
                else if (pSystem[i].type == SNOW) {
                    pSystem[i].timeToLive -= 0.01;
                    pSystem[i].size *= pSystem[i].timeToLive;
                }
                else if (pSystem[i].type == RAIN) {
                    pSystem[i].timeToLive -= 0.01;
                    pSystem[i].size += 0.7;
                }
            }
            else {
                if (snowing)
                    initSnow(i);
                if (raining)
                    initRain(i);
            }
        }
    }
    if (keystates[W]) {
        eyex = eyex + (sin(mlon * DEG_TO_RAD) * RUN_SPEED);
        eyez = eyez + (cos(mlon * DEG_TO_RAD) * RUN_SPEED);
    }
    if (keystates[A]) {
        eyex = eyex + (cos(mlon * DEG_TO_RAD) * RUN_SPEED);
        eyez = eyez - (sin(mlon * DEG_TO_RAD) * RUN_SPEED);
    }
    if (keystates[S]) {
        eyex = eyex - (sin(mlon * DEG_TO_RAD) * RUN_SPEED);
        eyez = eyez - (cos(mlon * DEG_TO_RAD) * RUN_SPEED);
    }
    if (keystates[D]) {
        eyex = eyex - (cos(mlon * DEG_TO_RAD) * RUN_SPEED);
        eyez = eyez + (sin(mlon * DEG_TO_RAD) * RUN_SPEED);
    }
    if (keystates[UP]) {
        gravity -= 0.001;
    }
    if (keystates[DOWN]) {
        gravity += 0.001;
    }
    if (keystates[LEFT]) {
        wind -= 0.00001;
    }
    if (keystates[RIGHT]) {
        wind += 0.00001;
    }
    if (keystates[R]) {
        particles = 0;
        keystates[R] = false;
    }

    glutWarpPointer(width/2, height/2);
    glutPostRedisplay(); //Tells open GL that the scene need redrawing
}

//////////////////////////////////////////////

int main(int argc, char** argv) {
    glutInit(&argc, argv); //initialise open GL
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(width, height);
    glutCreateWindow("Particle System");
    init();
    glutDisplayFunc(display); //Register teh display function 
    glutIdleFunc(idle);       //Register the idle function 
    glutReshapeFunc(reshape); //Register the reshape function 
    glutKeyboardFunc(keyboardDown); //Function when keys pressed
    glutKeyboardUpFunc(keyboardUp); //Function when keys released
    glutSpecialFunc(specialDown);
    glutSpecialUpFunc(specialUp);
    glutPassiveMotionFunc(mouse_motion);
    glutSetCursor(GLUT_CURSOR_NONE);
    glutMainLoop();     //Main loop of OpenGL
    return 0;
}
