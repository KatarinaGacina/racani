#pragma once

#include <Windows.h> 

#include <stdio.h>
#include <stdlib.h>

#include <math.h>
#include <vector>

#include <cstdlib>
#include <ctime>

#include <iostream>
#include <fstream>
#include <sstream>

#include <GL/glew.h>
#include <GL/glut.h>

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp> 
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <random>

#include "BSpline.h"
#include "Sustav.h"
#include "Prepreka.h"

#ifndef PI
#define PI 3.14159265358979323846
#endif


const char* vertexShaderSourceSphere = R"(
#version 460 core

layout(location = 0) in vec2 aPosition;
layout(location = 1) in vec3 aColor;
layout(location = 2) in mat4 aModel;

uniform mat4 view;
uniform mat4 projection;

out vec3 vertexColor;

void main() {
    gl_Position = projection * view * aModel * vec4(aPosition, 0.0f, 1.0f);
    
    vertexColor = aColor;
}
)";

const char* vertexShaderSourceSpline = R"(
#version 460 core

layout(location = 0) in vec2 inPosition;

uniform mat4 modelViewProjection;
uniform vec3 color;

out vec3 vertexColor;

void main() {
    gl_Position = modelViewProjection * vec4(inPosition, 0.0, 1.0);

    vertexColor = color;
}
)";

const char* fragmentShaderSource = R"(
#version 460 core

in vec3 vertexColor;

out vec4 FragColor;

void main() {
    FragColor = vec4(vertexColor, 1.0f);
}
)";


const char* vertexShaderPrepreke = R"(
#version 460 core

layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 center;
layout(location = 2) in float r;

out vec2 TexCoords;

uniform mat4 projection;
uniform mat4 view;

void main() {
    vec3 worldPos = center + vec3(aPos * r, 0.0);

    TexCoords = aPos * 0.5 + 0.5;

    gl_Position = projection * view * vec4(worldPos, 1.0);
}
)";

const char* fragmentShaderPrepreke = R"(
#version 460 core

out vec4 FragColor;
in vec2 TexCoords;

void main() {
    vec2 p = TexCoords * 2.0 - 1.0;

    float distSq = dot(p, p);

    if (distSq > 1.0) discard;

    FragColor = vec4(1.0, 1.0, 1.0, 1.0);

)";



GLuint VAO1, EBO, modelMatrixBuffer, vertexBuffer, colorBuffer;
GLuint VAO2, sphereColorBuffer, sphereMatrixBuffer;
GLuint VAO3, pointsBuffer;
GLuint VAO4, lineBuffer;
GLuint VAO5, EBO2, vertexBuffer2, krajColorBuffer, krajMatrixBuffer;
GLuint VAO6, EBO3, vertexBuffer3, preprekaColorBuffer, preprekaMatrixBuffer;

std::vector<glm::vec2> line;

std::vector<glm::mat4> modelMatrices;

std::vector<GLuint> indices;
std::vector<glm::vec2> vertices;

std::vector<GLuint> indices2;
std::vector<glm::vec2> vertices2;

std::vector<GLuint> indices3;
std::vector<glm::vec2> vertices3;

glm::mat4 projection;
glm::mat4 view;

GLuint shaderProgram;
GLuint shaderProgram2;



void generateSphere(std::vector<glm::vec2>& vertices, std::vector<unsigned int>& indices, float radius, unsigned int sectorCount, unsigned int stackCount) {

    vertices.clear();
    indices.clear();
    
    for (unsigned int i = 0; i <= stackCount; ++i) {
        float stackAngle = PI / 2 - i * PI / stackCount;
        float xy = radius * cosf(stackAngle);
        float z = radius * sinf(stackAngle);

        for (unsigned int j = 0; j <= sectorCount; ++j) {
            float sectorAngle = j * 2 * PI / sectorCount;
            float x = xy * cosf(sectorAngle);
            float y = xy * sinf(sectorAngle);

            glm::vec2 vertex;
            vertex.x = x;
            vertex.y = y;

            vertices.push_back(vertex);
        }
    }

    for (unsigned int i = 0; i < stackCount; ++i) {
        unsigned int k1 = i * (sectorCount + 1);
        unsigned int k2 = k1 + sectorCount + 1;

        for (unsigned int j = 0; j < sectorCount; ++j, ++k1, ++k2) {
            if (i != 0) {
                indices.push_back(k1);
                indices.push_back(k2);
                indices.push_back(k1 + 1);
            }
            if (i != (stackCount - 1)) {
                indices.push_back(k1 + 1);
                indices.push_back(k2);
                indices.push_back(k2 + 1);
            }
        }
    }
}


double MAX_X = 10.0;

GLuint window;
GLuint width = 500, height = 500;

int level;

std::vector<glm::vec3> colors;
int sphere_max_num;
int sphere_init_num;

struct Sphere {
    glm::vec2 init_pos = { 0.0, 0.0 };
    glm::vec2 pos = { 0.0, 0.0 };
    int intersection;
    int collision = -1;
    glm::vec3 color;
    glm::mat4 sphereMatrix;
    float r;
};
Sphere sphere;

BSpline bspline;
Sustav sustav;
Prepreka prepreka;

bool promjena = false;
bool hitac = false;

bool waitToRemove = false;
std::pair<int, int> removeIndexes;

glm::vec2 smjer = { 0.0, 0.0 };
glm::vec2 mouse_coordinates = { 0.0, 0.0 };

int t = 0;
int n = 0;


std::vector<int> candidates;

bool lineIntersects(glm::vec2 r, glm::vec2 p1, glm::vec2 p2) {
    glm::vec2 s = p2 - p1; //segment direction

    float d = r.x * s.y - r.y * s.x; //ray direction x segment direction
    if (d == 0.0f) {
        return false; //parallel lines
    }

    float d2 = p1.x * s.y - p1.y * s.x;

    float u = ((p1.x * r.y) - (p1.y * r.x)) / d;
    float t = d2 / d;

    return (u >= 0.0f && u <= 1.0f && t >= 0.0f);

}

int findClosestPoint() {
    candidates.clear();

    if (sustav.getSize() == 0) {
        return -1;
    }

    for (int i = 0; i < bspline.getSpacedDotsSize() - 1; i++) {
        if (lineIntersects(smjer, bspline.getSpacedDotsAt(i), bspline.getSpacedDotsAt(i + 1))) {
            if ((i + 1) >= sustav.getFirstT() - 1 && (i + 1) <= sustav.getLastT()) {
                candidates.push_back(i + 1);
            }
        }
    }

    if (candidates.size() <= 0) {
        return -1;
    }

    int best_candidate = candidates[0];
    float shortestLength = glm::distance(sphere.init_pos, bspline.getSpacedDotsAt(best_candidate));

    float pom;
    for (int& c : candidates) {
        pom = glm::distance(sphere.init_pos, bspline.getSpacedDotsAt(c));

        if (pom < shortestLength) {
            shortestLength = pom;
            best_candidate = c;
        }
    }

    return best_candidate;
}


void myDisplay() {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (!sustav.getKraj() && (sustav.getSize() != 0 || n < sphere_init_num)) {

        glUseProgram(shaderProgram);

        glBindVertexArray(VAO1);

        std::vector<glm::vec3> combinedData;
        for (int i = 0; i < (sustav.getFirstT() - 1); i++) {
            combinedData.push_back({ 0.0, 0.0, 0.0 });
        }
        combinedData.insert(combinedData.end(), sustav.getColors().begin(), sustav.getColors().end());

        glBindBuffer(GL_ARRAY_BUFFER, colorBuffer);
        glBufferData(GL_ARRAY_BUFFER, combinedData.size() * sizeof(glm::vec3), combinedData.data(), GL_DYNAMIC_DRAW);

        glDrawElementsInstancedBaseInstance(
            GL_TRIANGLES,
            indices.size(),
            GL_UNSIGNED_INT,
            (void*)0,
            sustav.getSize(),
            (sustav.getFirstT() - 1)
        );
        
        glBindVertexArray(0);

        glBindVertexArray(VAO2);

        glBindBuffer(GL_ARRAY_BUFFER, sphereColorBuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3), &sphere.color, GL_DYNAMIC_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER, sphereMatrixBuffer);
        sphere.sphereMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(sphere.pos, 0.0f));
        glBufferData(GL_ARRAY_BUFFER, sizeof(glm::mat4), &sphere.sphereMatrix, GL_DYNAMIC_DRAW);

        glDrawElementsInstancedBaseInstance(
            GL_TRIANGLES, 
            indices.size(),
            GL_UNSIGNED_INT, 
            (void*)0, 
            1, 
            0
        );

        glBindVertexArray(0);

        glBindVertexArray(VAO5);

        glDrawElementsInstancedBaseInstance(
            GL_TRIANGLES,
            indices2.size(),
            GL_UNSIGNED_INT,
            (void*)0,
            1,
            0
        );

        glBindVertexArray(0);

        glBindVertexArray(VAO6);

        glDrawElementsInstancedBaseInstance(
            GL_TRIANGLES,
            indices3.size(),
            GL_UNSIGNED_INT,
            (void*)0,
            1,
            0
        );

        glBindVertexArray(0);

        glUseProgram(shaderProgram2);

        glBindVertexArray(VAO3);

        glDrawArrays(GL_LINE_STRIP, 0, bspline.getDots().size());

        glBindVertexArray(0);

        if (!hitac) {
            glBindVertexArray(VAO4);

            glBindBuffer(GL_ARRAY_BUFFER, lineBuffer);
            line.pop_back();
            line.push_back(sphere.pos);
            glBufferData(GL_ARRAY_BUFFER, 2 * sizeof(glm::vec2), line.data(), GL_DYNAMIC_DRAW);

            glDrawArrays(GL_LINES, 0, 2);

            glBindVertexArray(0);
        }
    }

    glutSwapBuffers();
}


void timer(int value) {

    if (hitac) {
        sphere.pos.x += smjer.x * 2.0;
        sphere.pos.y += smjer.y * 2.0;


        if (sphere.collision >= 0) {
            hitac = false;

            sphere.pos = sphere.init_pos;
            sphere.color = colors[(std::rand() % 3)];
        }
        else if (sphere.intersection >= 0) {
            if (glm::distance(sphere.pos, bspline.getSpacedDotsAt(sphere.intersection)) < (sphere.r + 2.0)) {
                
                int insertion_index = sustav.insertSpheresTime(sphere.color, (sphere.intersection + 1));

                removeIndexes = sustav.findIndexes(insertion_index);
                if (removeIndexes.second - removeIndexes.first + 1 >= 3) {
                    waitToRemove = true;
                }

                hitac = false;

                sphere.pos = sphere.init_pos;
                sphere.color = colors[(std::rand() % colors.size())];
            }
        }
        else {
            if (sphere.pos.x <= -MAX_X or sphere.pos.y <= -MAX_X or sphere.pos.x >= MAX_X or sphere.pos.y >= MAX_X) {
                hitac = false;

                sphere.pos = sphere.init_pos;
                sphere.color = colors[(std::rand() % colors.size())];
            }
        }
    } 
    else if (waitToRemove) {
        sustav.removeSpheresTime(removeIndexes.first, removeIndexes.second);

        if (sustav.getSize() > 0) {
            if (removeIndexes.first <= sustav.getSize() - 1) {
                removeIndexes = sustav.findIndexes(removeIndexes.first);

                if (removeIndexes.second - removeIndexes.first + 1 < 3) {
                    waitToRemove = false;
                }
            }
            else {
                waitToRemove = false;
            }
        }
        else {
            waitToRemove = false;
        }

    }
    else if (!sustav.getKraj()){
        t++;

        if (t % 10 == 0 || n == 0 || n < sphere_init_num) {
            if (n <= sphere_max_num) {
                sustav.insertSpheresTime(colors[(std::rand() % colors.size())], 1);

                n++;
            }
            else {
                sustav.updateSustav();
            }

            t = 0;
        }
    }

    glutPostRedisplay();

    glutTimerFunc(100, timer, 0);
}


void reshape(int w, int h) {
    glViewport(0, 0, w, h);

    double aspect = (double)w / (double)h;

    if (width >= height) {
        projection = glm::ortho(-MAX_X * aspect, MAX_X * aspect, -MAX_X, MAX_X, 1.0, 100.0);
    }
    else {
        projection = glm::ortho(-MAX_X, MAX_X, -MAX_X / aspect, MAX_X / aspect, 1.0, 100.0);
    }

    glUseProgram(shaderProgram);
    GLuint projectionLoc = glGetUniformLocation(shaderProgram, "projection");
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

    glUseProgram(shaderProgram2);
    glm::mat4 mvp = projection * view * glm::mat4(1.0f);
    GLuint mvpLoc = glGetUniformLocation(shaderProgram2, "modelViewProjection");
    glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, glm::value_ptr(mvp));

    width = w;
    height = h;

    glEnable(GL_DEPTH_TEST);
}


void setup1() {
    level = 1;

    bspline.init(100, ".\\lab3\\bspline\\bspline_0.txt");
    bspline.calculateSpacedDots(1.0);
    sustav.init(bspline.getSpacedDotsSize(), 0.5, bspline);

    colors.clear();
    colors.push_back(glm::vec3(1.0, 1.0, 0.0));
    colors.push_back(glm::vec3(1.0, 0.0, 1.0));
    colors.push_back(glm::vec3(0.0, 1.0, 1.0));

    sphere.pos = sphere.init_pos;
    sphere.r = 0.5;
    sphere.color = colors[(std::rand() % colors.size())];

    sphere_max_num = bspline.getSpacedDotsSize();
    sphere_init_num = sphere_max_num / 2;

    t = 0;
    n = 0;

    promjena = false;
    hitac = false;
    waitToRemove = false;

    prepreka.init(glm::vec2{2.3, 2.3}, 0.6);

}

void setup2() {
    level = 2;

    bspline.init(100, ".\\lab3\\bspline\\bspline_1.txt");
    bspline.calculateSpacedDots(1.0);
    sustav.init(bspline.getSpacedDotsSize(), 0.5, bspline);

    colors.clear();
    colors.push_back(glm::vec3(1.0, 1.0, 0.0));
    colors.push_back(glm::vec3(1.0, 0.0, 1.0));
    colors.push_back(glm::vec3(0.0, 1.0, 1.0));
    colors.push_back(glm::vec3(0.0, 1.0, 0.0));

    sphere.pos = sphere.init_pos;
    sphere.r = 0.5;
    sphere.color = colors[(std::rand() % colors.size())];

    sphere_max_num = bspline.getSpacedDotsSize();
    sphere_init_num = sphere_max_num / 2;

    t = 0;
    n = 0;

    promjena = false;
    hitac = false;
    waitToRemove = false;

    prepreka.init(glm::vec2{ 2.5, 2.5 }, 0.6);

}


void setShaderData() {

    glm::vec3 whiteColor(1.0f, 1.0f, 1.0f);

    vertices.clear();
    indices.clear();
    generateSphere(vertices, indices, sphere.r, 36, 18);

    view = glm::lookAt(
        glm::vec3{ 0.0, 0.0, 20.0 },
        glm::vec3{ 0.0, 0.0, 0.0 },
        glm::vec3{ 0.0, 1.0, 0.0 });

    double aspect = (double)width / (double)height;
    if (width >= height) {
        projection = glm::ortho(-MAX_X * aspect, MAX_X * aspect, -MAX_X, MAX_X, 1.0, 100.0);
    }
    else {
        projection = glm::ortho(-MAX_X, MAX_X, -MAX_X / aspect, MAX_X / aspect, 1.0, 100.0);
    }

    glUseProgram(shaderProgram);

    glGenVertexArrays(1, &VAO1);
    glBindVertexArray(VAO1);

    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);

    glGenBuffers(1, &vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec2), vertices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void*)0);
    glEnableVertexAttribArray(0);

    modelMatrices.clear();
    modelMatrices.reserve(bspline.getSpacedDotsSize());
    modelMatrices.resize(bspline.getSpacedDotsSize());
    for (int i = 0; i < bspline.getSpacedDotsSize(); i++) {
        modelMatrices[i] = glm::translate(glm::mat4(1.0f), glm::vec3(bspline.getSpacedDotsAt(i), 0.0f));
    }

    glGenBuffers(1, &modelMatrixBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, modelMatrixBuffer);
    glBufferData(GL_ARRAY_BUFFER, modelMatrices.size() * sizeof(glm::mat4), modelMatrices.data(), GL_STATIC_DRAW);

    for (GLuint i = 0; i < 4; ++i) {
        glVertexAttribPointer(2 + i, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(i * sizeof(glm::vec4)));
        glEnableVertexAttribArray(2 + i);
        glVertexAttribDivisor(2 + i, 1);
    }

    glGenBuffers(1, &colorBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, colorBuffer);
    glBufferData(GL_ARRAY_BUFFER, sustav.getColors().size() * sizeof(glm::vec3), sustav.getColors().data(), GL_DYNAMIC_DRAW);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribDivisor(1, 1);

    glBindVertexArray(0);

    
    glGenVertexArrays(1, &VAO2);
    glBindVertexArray(VAO2);

    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);

    glGenBuffers(1, &vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec2), vertices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void*)0);
    glEnableVertexAttribArray(0);

    sphere.sphereMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(sphere.pos, 0.0f));

    glGenBuffers(1, &sphereMatrixBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, sphereMatrixBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::mat4), &sphere.sphereMatrix, GL_DYNAMIC_DRAW);

    for (GLuint i = 0; i < 4; ++i) {
        glVertexAttribPointer(2 + i, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(i * sizeof(glm::vec4)));
        glEnableVertexAttribArray(2 + i);
        glVertexAttribDivisor(2 + i, 1);
    }

    glGenBuffers(1, &sphereColorBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, sphereColorBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3), &sphere.color, GL_DYNAMIC_DRAW);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribDivisor(1, 1);
    
    glBindVertexArray(0);


    vertices2.clear();
    indices2.clear();
    generateSphere(vertices2, indices2, 0.8, 36, 18);

    glGenVertexArrays(1, &VAO5);
    glBindVertexArray(VAO5);

    glGenBuffers(1, &EBO2);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO2);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices2.size() * sizeof(GLuint), indices2.data(), GL_STATIC_DRAW);

    glGenBuffers(1, &vertexBuffer2);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer2);
    glBufferData(GL_ARRAY_BUFFER, vertices2.size() * sizeof(glm::vec2), vertices2.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void*)0);
    glEnableVertexAttribArray(0);

    glm::mat4 krajMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(bspline.getSpacedDotsAt(bspline.getSpacedDotsSize() - 1), 0.0f));

    glGenBuffers(1, &krajMatrixBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, krajMatrixBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::mat4), &krajMatrix, GL_STATIC_DRAW);

    for (GLuint i = 0; i < 4; ++i) {
        glVertexAttribPointer(2 + i, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(i * sizeof(glm::vec4)));
        glEnableVertexAttribArray(2 + i);
        glVertexAttribDivisor(2 + i, 1);
    }

    glGenBuffers(1, &krajColorBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, krajColorBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3), &whiteColor, GL_STATIC_DRAW);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribDivisor(1, 1);

    glBindVertexArray(0);


    vertices3.clear();
    indices3.clear();
    generateSphere(vertices3, indices3, prepreka.r, 36, 18);

    glGenVertexArrays(1, &VAO6);
    glBindVertexArray(VAO6);

    glGenBuffers(1, &EBO3);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO3);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices3.size() * sizeof(GLuint), indices3.data(), GL_STATIC_DRAW);

    glGenBuffers(1, &vertexBuffer3);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer3);
    glBufferData(GL_ARRAY_BUFFER, vertices3.size() * sizeof(glm::vec2), vertices3.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void*)0);
    glEnableVertexAttribArray(0);

    glm::mat4 preprekaMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(prepreka.position, 0.0f));

    glGenBuffers(1, &preprekaMatrixBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, preprekaMatrixBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::mat4), &preprekaMatrix, GL_STATIC_DRAW);

    for (GLuint i = 0; i < 4; ++i) {
        glVertexAttribPointer(2 + i, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(i * sizeof(glm::vec4)));
        glEnableVertexAttribArray(2 + i);
        glVertexAttribDivisor(2 + i, 1);
    }

    glGenBuffers(1, &preprekaColorBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, preprekaColorBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3), &whiteColor, GL_STATIC_DRAW);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribDivisor(1, 1);

    glBindVertexArray(0);


    GLuint viewLoc = glGetUniformLocation(shaderProgram, "view");
    GLuint projectionLoc = glGetUniformLocation(shaderProgram, "projection");

    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));


    glUseProgram(shaderProgram2);

    glGenVertexArrays(1, &VAO3);
    glBindVertexArray(VAO3);

    glGenBuffers(1, &pointsBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, pointsBuffer);
    glBufferData(GL_ARRAY_BUFFER, bspline.getDots().size() * sizeof(glm::vec2), bspline.getDots().data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void*)0);
    glEnableVertexAttribArray(0);

    glm::mat4 mvp = projection * view * glm::mat4(1.0f);
    GLuint mvpLoc = glGetUniformLocation(shaderProgram2, "modelViewProjection");
    glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, glm::value_ptr(mvp));

    GLuint col = glGetUniformLocation(shaderProgram2, "color");
    glUniform3fv(col, 1, glm::value_ptr(whiteColor));

    glBindVertexArray(0);


    glGenVertexArrays(1, &VAO4);
    glBindVertexArray(VAO4);

    glGenBuffers(1, &lineBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, lineBuffer);
    line.clear();
    line.push_back(sphere.init_pos);
    line.push_back(sphere.pos);
    glBufferData(GL_ARRAY_BUFFER, 2 * sizeof(glm::vec2), line.data(), GL_DYNAMIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);

}

void clearBuffers() {
    glDeleteBuffers(1, &EBO);
    glDeleteBuffers(1, &vertexBuffer);
    glDeleteBuffers(1, &modelMatrixBuffer);
    glDeleteBuffers(1, &colorBuffer);
    glDeleteVertexArrays(1, &VAO1);

    glDeleteBuffers(1, &sphereMatrixBuffer);
    glDeleteBuffers(1, &sphereColorBuffer);
    glDeleteVertexArrays(1, &VAO2);

    glDeleteBuffers(1, &EBO2);
    glDeleteBuffers(1, &vertexBuffer2);
    glDeleteBuffers(1, &krajMatrixBuffer);
    glDeleteBuffers(1, &krajColorBuffer);
    glDeleteVertexArrays(1, &VAO5);

    glDeleteBuffers(1, &EBO3);
    glDeleteBuffers(1, &vertexBuffer3);
    glDeleteBuffers(1, &preprekaMatrixBuffer);
    glDeleteBuffers(1, &preprekaColorBuffer);
    glDeleteVertexArrays(1, &VAO6);

    glDeleteBuffers(1, &pointsBuffer);
    glDeleteVertexArrays(1, &VAO3);

    glDeleteBuffers(1, &lineBuffer);
    glDeleteVertexArrays(1, &VAO4);
}



void mouseMotion(int x, int y) {
    if (promjena) {
        mouse_coordinates.x = ((float)(x) / width * (2.0 * MAX_X) - MAX_X);
        mouse_coordinates.y = -((float)(y) / height * (2.0 * MAX_X) - MAX_X);

        mouse_coordinates = glm::normalize(mouse_coordinates);

        smjer.x = sphere.init_pos.x - mouse_coordinates.x;
        smjer.y = sphere.init_pos.y - mouse_coordinates.y;

        sphere.pos.x = mouse_coordinates.x * 2.0;
        sphere.pos.y = mouse_coordinates.y * 2.0;

        glutPostRedisplay();
    }
}

void mouseButton(int button, int state, int x, int y) {

    if (button == GLUT_LEFT_BUTTON) {
        float x_real = ((float)x / width * (2.0f * MAX_X) - MAX_X);
        float y_real = -((float)y / height * (2.0f * MAX_X) - MAX_X);

        float distanceSquared = (x_real - sphere.pos.x) * (x_real - sphere.pos.x) +
            (y_real - sphere.pos.y) * (y_real - sphere.pos.y);

        if (distanceSquared <= sphere.r * sphere.r) {
            if (state == GLUT_DOWN && !waitToRemove && !hitac && n > sphere_init_num) {
                promjena = true;
            }
        }

        if (state == GLUT_UP && promjena && !sustav.getKraj()) {
            promjena = false;
            hitac = true;

            sphere.intersection = findClosestPoint();

            sphere.collision = -1;
            if (prepreka.checkCollision(smjer)) {
                sphere.collision = 0;

                if (sphere.intersection != -1 &&
                    (glm::distance(sphere.init_pos, bspline.getSpacedDotsAt(sphere.intersection)) < glm::distance(sphere.init_pos, prepreka.position))) {
                    sphere.collision = -1;
                }
            }
            

        }
    }
}


void myKeyboard(unsigned char theKey, int mouseX, int mouseY)
{
    switch (theKey)
    {
    case 'x':
        if ((sustav.getSize() == 0 && n != 0)) {
            clearBuffers();
            setup2();
            setShaderData();
        }
        else if (sustav.getKraj()) {
            if (level == 1) {
                clearBuffers();
                setup1();
                setShaderData();
            }
            else if (level == 2) {
                clearBuffers();
                setup2();
                setShaderData();
            }

        }
    default:
        return;
    }
}


int main(int argc, char** argv) {

    srand(static_cast<unsigned int>(time(0)));

    glutInit(&argc, argv);

    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
    glutInitWindowSize(width, height);
    glutInitWindowPosition(100, 100);

    glutCreateWindow("Game");

    glewInit();

    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;

    glutReshapeFunc(reshape);

    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSourceSphere, NULL);
    glCompileShader(vertexShader);

    GLuint vertexShader2 = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader2, 1, &vertexShaderSourceSpline, NULL);
    glCompileShader(vertexShader2);

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    shaderProgram2 = glCreateProgram();
    glAttachShader(shaderProgram2, vertexShader2);
    glAttachShader(shaderProgram2, fragmentShader);
    glLinkProgram(shaderProgram2);

    glDeleteShader(vertexShader);
    glDeleteShader(vertexShader2);
    glDeleteShader(fragmentShader);

    std::cout << "Shader Program ID: " << shaderProgram << std::endl;

    setup2();
    setShaderData();

    glutDisplayFunc(myDisplay);
    glutMotionFunc(mouseMotion);
    glutMouseFunc(mouseButton);
    glutKeyboardFunc(myKeyboard);

    glutTimerFunc(100, timer, 0);


    glutMainLoop();

    return 0;
}