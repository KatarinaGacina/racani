#include <Windows.h> 

#include <stdio.h>
#include <stdlib.h>

#include <math.h>
#include <vector>

#include <iostream>
#include <cstdlib>
#include <ctime>

#include <GL/glut.h>

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp> 

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "Particle.h"

#define particles_num 100

GLuint window;
GLuint width = 500, height = 500;

glm::vec3 ociste = { 20.0, 7.5, 20.0 };
glm::vec3 wind = { 0.0, 0.0, 0.0 };
int t = 0;

std::vector<Particle> particles;
GLuint texture_id;

glm::vec3 position;
glm::vec3 os;
float angle;
float size;

GLuint loadTexture(char* argv[], const std::string& filename) {
    std::string path(argv[0]);
    std::size_t lastSlashPos = path.find_last_of("\\/");
    std::size_t secondLastSlashPos = path.find_last_of("\\/", lastSlashPos - 1);
    std::size_t thirdLastSlashPos = path.find_last_of("\\/", secondLastSlashPos - 1);
    std::string dirPath(path, 0, thirdLastSlashPos);
    std::string resPath(dirPath);
    resPath.append("\\texture\\");
    std::string filePath(resPath);
    filePath.append(filename);

    int width, height, channels;
    unsigned char* data = stbi_load(filePath.c_str(), &width, &height, &channels, 0);

    if (!data) {
        std::cerr << "Failed to load image with stb_image.\n" << std::endl;
        exit(1);
    }

    GLuint textureID;

    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data); - gpu
    
    //glGenerateMipmap(GL_TEXTURE_2D);
    gluBuild2DMipmaps(GL_TEXTURE_2D, 3, width, height, GL_RGB, GL_UNSIGNED_BYTE, data);

    stbi_image_free(data);

    return textureID;
}


void myDisplay() {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    //glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    /*glPointSize(5.0f);
    glBegin(GL_POINTS);
    glColor3f(1.0f, 0.0f, 0.0f);
    glVertex3f(15.0f, 15.0f, 15.0f);
    glEnd();*/

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texture_id);

    glColor3f(1.0f, 1.0f, 1.0f);
    for (Particle& p : particles) {
        p.update(ociste, wind);

        position = p.getPosition();
        os = p.getOs();
        angle = p.getKut();
        size = p.getSize();
        
        glPushMatrix();

        glTranslatef(position.x, position.y, position.z);
        glRotatef(angle, os.x, os.y, os.z);

        glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 0.0f);
        glVertex3f(-size, -size, 0.0f);

        glTexCoord2f(1.0f, 0.0f);
        glVertex3f(size, -size, 0.0f);

        glTexCoord2f(1.0f, 1.0f);
        glVertex3f(size, size, 0.0f);

        glTexCoord2f(0.0f, 1.0f);
        glVertex3f(-size, size, 0.0f);
        glEnd();

        glPopMatrix();
    }

    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_TEXTURE_2D);
    
    glutSwapBuffers();
}


void reshape(int w, int h) {
    width = w;
    height = h;

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    gluPerspective(45.0f, (float)w / (float)h, 1.0f, 100.0f);

    glViewport(0, 0, w, h);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    gluLookAt(
        ociste.x, ociste.y, ociste.z,
        7.5, 7.5, 7.5,
        0.0, 1.0, 0.0
    );

    glEnable(GL_DEPTH_TEST);
}


void timer(int value) {
    if (t < 90) {
        t++;
        wind = { 0.0, 0.0, 0.0 };
    }
    else {
        wind = { -0.3, 0.0, 0.0 };
    }

    glutPostRedisplay();

    glutTimerFunc(100, timer, 0); // t = 1s
}


void keyboardFunction(unsigned char theKey, int mouseX, int mouseY) {
    
    switch (theKey) {
        case 'l': 
            ociste.x = ociste.x + 0.1;
            break;

        case 'k': 
            ociste.x = ociste.x - 0.1;
            break;

        case 'a': 
            ociste.y = ociste.y + 0.1;
            break;

        case 'd': 
            ociste.y = ociste.y - 0.1;
            break;

        case 27:  exit(0);
            break;
    }

    reshape(width, height);
    glutPostRedisplay();
}


int main(int argc, char** argv) {
    srand(static_cast<unsigned int>(time(0)));

    particles.reserve(particles_num); 
    for (int i = 0; i < particles_num; ++i) {
        particles.push_back(Particle(ociste));
    }

    glutInit(&argc, argv);

    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
    glutInitWindowSize(width, height);
    glutInitWindowPosition(100, 100);

    glutCreateWindow("Particle system viewer");

    glutReshapeFunc(reshape);

    texture_id = loadTexture(argv, "snow.bmp");

    glutDisplayFunc(myDisplay);
    glutKeyboardFunc(keyboardFunction);

    glutTimerFunc(100, timer, 0);

    glutMainLoop();

    return 0;
}
