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
#define MAX_PARTICLES 10000
#define EMMITTER_SIZE 10
#define SNOW_SIZE 1

#define KEYS 4
#define W 0
#define A 1
#define S 2
#define D 3

GLdouble lat, lon;              /* View angles (degrees)    */
GLdouble mlat, mlon;             /* Mouse look offset angles */
GLfloat  eyex, eyey, eyez;    /* Eye point                */
GLfloat  centerx, centery, centerz; /* Look point               */
GLfloat  upx, upy, upz;     /* View up vector           */
GLfloat  lookX, lookY = 0;
bool keystates[KEYS];

GLint width = 1400, height = 1000;      /* size of window           */
GLint falling = false;		    //play and pause simulation
GLint WALKING = 0;	    	    /* Representing the walking state */
GLint state = 0;
GLfloat initHeight = 4;
GLfloat snowVel = 0.01;

//////////////////////////////////////////////
struct snowflake {

    GLfloat height = initHeight + (4 * (float)rand()/RAND_MAX);
    GLfloat xCoord = EMMITTER_SIZE * ( (float)rand()/RAND_MAX - 2 * ((float)rand()/RAND_MAX));
    GLfloat zCoord = EMMITTER_SIZE * ((float)rand() / RAND_MAX - 2 * ((float)rand() / RAND_MAX));
    GLdouble size = SNOW_SIZE;
    GLfloat velocity = snowVel;
};

struct raindrop {
    //GLfloat life = ;
};

struct snowflake pSystem[MAX_PARTICLES];

GLfloat white_light[] = { 1.0, 1.0, 1.0, 1.0 };
GLfloat light_position0[] = { -5.0, 7.0, -5.0, 0.0 };
GLfloat matSpecular[] = { 1.0, 1.0, 1.0, 1.0 };
GLfloat matShininess[] = { 50.0 };
GLfloat matSurface[] = { 0.8, 0.5, 0.2, 0.1 };
GLfloat matEmissive[] = { 0.0, 1.0, 0.0, 0.1 };

//////////////////////////////////////////////

void drawSnow(snowflake snow) {
    //draw a stationary snow particle
    glDisable(GL_LIGHTING);
    glEnable(GL_POINT_SMOOTH);
    glPointSize(snow.size);
    glColor3f(1, 1, 1);
    glBegin(GL_POINTS);
    glVertex3f(snow.xCoord, snow.height, snow.zCoord);
    glEnd();
    glEnable(GL_LIGHTING);
}

void draw_scene(void) {
    // Draws all the elements in the scene
    int x, z;
    int L = 25;

    /* Draw ground */
    glDisable(GL_LIGHTING);
    // The ground quad and the grid lines are co-planar, which would lead to horrible Z-fighting,
    // so we resort to 2 hacks. First, fiddle with the Z-buffer depth range, using glDepthRange(),
    // and second, draw the lines 0.01 higher in Y than the ground plane
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

    //Draw a snow particle
    for (int i = 0; i < MAX_PARTICLES; i++) {
        drawSnow(pSystem[i]);
    }
} // draw_scene()

//////////////////////////////////////////////

void calculate_lookpoint(void) { /* Given an eyepoint and latitude and longitude angles, will
     compute a look point one unit away */

     /* To be completed */
    centerx = eyex + (cos((mlat + lat) * DEG_TO_RAD) * sin((mlon + lon) * DEG_TO_RAD));
    centery = eyey + sin((mlat + lat) * DEG_TO_RAD);
    centerz = eyez + (cos((mlat + lat) * DEG_TO_RAD) * cos((mlon + lon) * DEG_TO_RAD));

} // calculate_lookpoint()

//Called by OpenGl to update the display
void display(void) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //Flush the frame and depth buffer to update window
    glLoadIdentity(); //reset the matrix to defualt
    calculate_lookpoint(); /* Compute the centre of interest   */
    gluLookAt(eyex, eyey, eyez, centerx, centery, centerz, upx, upy, upz);
    //glLightfv(GL_LIGHT0, GL_POSITION, light_position0);
    draw_scene();
    glutSwapBuffers();
} // display()

//Called by OpenGl when the window is moved/reshaped
void reshape(int w, int h) {
    glViewport(0, 0, (GLsizei)w, (GLsizei)h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(50, (GLfloat)w / (GLfloat)h, 0.1, 80.0);
    glMatrixMode(GL_MODELVIEW);
    width = w;   /* Record the new width and height */
    height = h;
} //reshape()

//////////////////////////////////////////////

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

//////////////////////////////////////////////

void keyboardDown(unsigned char key, int x, int y) {
    switch (key) {
    case 27:  /* Escape key */
        exit(0);
        break;
    case 32: /* space bar */
        falling = !falling;
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
    }
} // keyboard()

//////////////////////////////////////////////

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
} // keyboard()

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
    for (int i = 0; i < MAX_PARTICLES; i++) {
        struct snowflake snow;
        pSystem[i] = snow;
    }
    
    //set the keystates to false
    for (int i = 0; i < KEYS; i++) {
        keystates[i] = false;
    }
} // init()

//Function called bu OpenGl called on each glutMainLoop
void idle(void) {
    //Have logic calculating falling speed of snow flake
    for (int i = 0; i < MAX_PARTICLES; i++) {
        if (falling) {
            if (pSystem[i].height <= 0.02)
                pSystem[i].height = 0.02;//initHeight;
            pSystem[i].height -= 0.01;  
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
    glutPassiveMotionFunc(mouse_motion);
    glutSetCursor(GLUT_CURSOR_NONE);
    glutMainLoop();     //Main loop of OpenGL
    return 0;
}
