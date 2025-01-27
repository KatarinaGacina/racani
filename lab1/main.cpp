//gcc -Iinclude -Llib -o lab.exe lab1.cpp -lfreeglut -lopengl32 -lglu32 -lassimp-vc143-mt

#include <Windows.h> 

#include <stdio.h>
#include <stdlib.h>
#include <GL/glut.h>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <iostream>
#include <fstream>
#include <sstream>

#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp> 

int POM = 20;

GLuint window;
GLuint width = 500, height = 500;

void myDisplay();

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    //glm::vec2 texcoord;
};
std::vector<Vertex> vertices;

std::vector<int> faces;

glm::vec3 centar = { 0, 0, 0 };

std::vector<glm::vec3> bspline_dots;

std::vector<glm::vec3> dots;
std::vector<glm::vec3> dots_tangents;

glm::vec3 init_axis = { 0, 0, 1 };
glm::vec3 goal_axis = { 0, 0, 0 };
glm::vec3 os = { 0, 0, 0 };


int position = 0;


void loadBSpline(char* argv[], const std::string& filename) {
    std::string path(argv[0]);
    std::size_t lastSlashPos = path.find_last_of("\\/");
    std::size_t secondLastSlashPos = path.find_last_of("\\/", lastSlashPos - 1);
    std::size_t thirdLastSlashPos = path.find_last_of("\\/", secondLastSlashPos - 1);
    std::string dirPath(path, 0, thirdLastSlashPos);
    std::string resPath(dirPath);
    resPath.append("\\bspline\\");
    std::string filePath(resPath);
    filePath.append(filename);

    std::ifstream file(filePath.c_str());
    std::string line;

    if (file.is_open()) {
        while (std::getline(file, line, '\n')) {
            std::stringstream ss(line);
            std::string value;

            glm::vec3 dot;

            std::getline(ss, value, ',');
            dot.x = std::stof(value);
            std::getline(ss, value, ',');
            dot.y = std::stof(value);
            std::getline(ss, value);
            dot.z = std::stof(value);

            bspline_dots.push_back(dot);
        }

        file.close();
    }
    else {
        std::cerr << "Error opening the file!" << std::endl;
        exit(1);
    }

    if (bspline_dots.size() < 4) {
        exit(1);
    }

    return;
}


void calculate_spline() {

    glm::vec3 spline_dot;
    glm::vec3 spline_tangent;
    glm::vec3 spline_tangent_unit;

    float l;

    int dot_num = (bspline_dots.size() - 3) * POM;

    dots.reserve(dot_num);
    dots.clear();

    dots_tangents.reserve(dot_num);
    dots_tangents.clear();

    for (unsigned int i = 0; i < bspline_dots.size() - 3; i++) {
        for (unsigned int j = 0; j < POM; ++j) {

            float t = (float)j / POM;

            float v1 = (-1 * t * t * t + 3 * t * t - 3 * t + 1) / 6.0f;
            float v2 = (3 * t * t * t - 6 * t * t + 4) / 6.0f;
            float v3 = (-3 * t * t * t + 3 * t * t + 3 * t + 1) / 6.0f;
            float v4 = (t * t * t) / 6.0f;

            spline_dot.x = v1 * bspline_dots.at(i).x +
                v2 * bspline_dots.at(i + 1).x +
                v3 * bspline_dots.at(i + 2).x +
                v4 * bspline_dots.at(i + 3).x;
            spline_dot.y = v1 * bspline_dots.at(i).y +
                v2 * bspline_dots.at(i + 1).y +
                v3 * bspline_dots.at(i + 2).y +
                v4 * bspline_dots.at(i + 3).y;
            spline_dot.z = v1 * bspline_dots.at(i).z +
                v2 * bspline_dots.at(i + 1).z +
                v3 * bspline_dots.at(i + 2).z +
                v4 * bspline_dots.at(i + 3).z;

            dots.push_back(spline_dot);

            float t1 = (-1 * t * t + 2 * t - 1) / 2.0f;
            float t2 = (3 * t * t - 4 * t) / 2.0f;
            float t3 = (-3 * t * t + 2 * t + 1) / 2.0f;
            float t4 = (t * t) / 2.0f;

            spline_tangent.x = t1 * bspline_dots.at(i).x +
                t2 * bspline_dots.at(i + 1).x +
                t3 * bspline_dots.at(i + 2).x +
                t4 * bspline_dots.at(i + 3).x;
            spline_tangent.y = t1 * bspline_dots.at(i).y +
                t2 * bspline_dots.at(i + 1).y +
                t3 * bspline_dots.at(i + 2).y +
                t4 * bspline_dots.at(i + 3).y;
            spline_tangent.z = t1 * bspline_dots.at(i).z +
                t2 * bspline_dots.at(i + 1).z +
                t3 * bspline_dots.at(i + 2).z +
                t4 * bspline_dots.at(i + 3).z;

            l = sqrt((spline_tangent.x * spline_tangent.x) +
                (spline_tangent.y * spline_tangent.y) +
                (spline_tangent.z * spline_tangent.z));

            spline_tangent_unit.x = spline_dot.x + (spline_tangent.x / l) * 2.0;
            spline_tangent_unit.y = spline_dot.y + (spline_tangent.y / l) * 2.0;
            spline_tangent_unit.z = spline_dot.z + (spline_tangent.z / l) * 2.0;

            dots_tangents.push_back(spline_tangent_unit);

            //std::cout << spline_dot.x << " " << spline_dot.y << " " << spline_dot.z << std::endl;
            //std::cout << spline_tangent.x << " " << spline_tangent.y << " " << spline_tangent.z << std::endl;
        }
    }
}


void loadModel(char* argv[], const std::string& filename) {
    Assimp::Importer importer;

    std::string path(argv[0]);
    std::size_t lastSlashPos = path.find_last_of("\\/");
    std::size_t secondLastSlashPos = path.find_last_of("\\/", lastSlashPos - 1);
    std::size_t thirdLastSlashPos = path.find_last_of("\\/", secondLastSlashPos - 1);
    std::string dirPath(path, 0, thirdLastSlashPos);
    std::string resPath(dirPath);
    resPath.append("\\resources\\");
    std::string objPath(resPath);
    objPath.append(filename);

    const aiScene* scene = importer.ReadFile(objPath.c_str(),
        aiProcess_CalcTangentSpace |
        aiProcess_Triangulate |
        aiProcess_JoinIdenticalVertices |
        aiProcess_SortByPType |
        aiProcess_FlipUVs |
        aiProcess_GenNormals
    );

    if (!scene) {
        std::cerr << "Error loading model: " << importer.GetErrorString() << std::endl;
        exit(1);
    }

    const aiMesh* mesh = scene->mMeshes[0];

    vertices.reserve(mesh->mNumVertices);
    vertices.clear();

    Vertex vertex;
    for (unsigned int j = 0; j < mesh->mNumVertices; j++) {
        vertex.position.x = mesh->mVertices[j].x;
        centar.x += vertex.position.x;
        vertex.position.y = mesh->mVertices[j].y;
        centar.y += vertex.position.y;
        vertex.position.z = mesh->mVertices[j].z;
        centar.z += vertex.position.z;

        if (mesh->HasNormals()) {
            vertex.normal.x = mesh->mNormals[j].x;
            vertex.normal.y = mesh->mNormals[j].y;
            vertex.normal.z = mesh->mNormals[j].z;
        }

        /*if (mesh->HasTextureCoords(0)) {
            vertex.texcoord[0] = mesh->mTextureCoords[0][j].x;
            vertex.texcoord[1] = mesh->mTextureCoords[0][j].y;
        }*/

        vertices.push_back(vertex);
    }

    centar.x /= mesh->mNumVertices;
    centar.y /= mesh->mNumVertices;
    centar.z /= mesh->mNumVertices;

    faces.reserve(mesh->mNumFaces * 3);
    faces.clear();

    for (unsigned int j = 0; j < mesh->mNumFaces; j++) {
        aiFace& face = mesh->mFaces[j];

        for (unsigned int k = 0; k < face.mNumIndices; k++) {
            faces.push_back(face.mIndices[k]);
        }
    }

    return;
}


void myDisplay() {
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glLineWidth(3.0f);
    glColor3f(0.0f, 0.0f, 1.0f);
    glBegin(GL_LINE_STRIP);
    for (const auto& dot : dots) {
        glVertex3f(dot.x, dot.y, dot.z);
    }
    glEnd();


    glColor3f(0.0f, 1.0f, 0.0f);
    glBegin(GL_LINES);
    for (unsigned int i = 0; i < dots.size(); i += (int)(dots.size() / 10)) {
        glVertex3f(dots.at(i).x, dots.at(i).y, dots.at(i).z);
        glVertex3f(dots_tangents.at(i).x, dots_tangents.at(i).y, dots_tangents.at(i).z);
    }
    glEnd();


    /*glPointSize(5.0f);
    glColor3f(0.0f, 1.0f, 0.0f);
    glBegin(GL_POINTS);
    for (const auto& dot : bspline_dots) {
        glVertex3f(dot.x, dot.y, dot.z);
    }*/
    //glVertex3f(-1, -1, -1);
    //glEnd();


    glPushMatrix(); //shaders!

    glTranslatef(dots.at(position).x, dots.at(position).y, dots.at(position).z);

    goal_axis = dots_tangents.at(position) - dots.at(position);
    goal_axis = glm::normalize(goal_axis);

    os.x = init_axis.y * goal_axis.z - goal_axis.y * init_axis.z;
    os.y = -init_axis.x * goal_axis.z + goal_axis.x * init_axis.z;
    os.z = init_axis.x * goal_axis.y - init_axis.y * goal_axis.x;
    os = glm::normalize(os);

    float kutDeg = glm::degrees(std::acos(glm::dot(init_axis, goal_axis)));

    glRotatef(kutDeg, os.x, os.y, os.z);

    glTranslatef(-centar.x, -centar.y, -centar.z);

    glColor3f(1.0f, 0.0f, 0.0f);
    /*glBegin(GL_TRIANGLES);
    for (const auto& vertex : vertices) {
        glVertex3f(vertex.position[0], vertex.position[1], vertex.position[2]);
    }
    glEnd();*/
    glBegin(GL_TRIANGLES);
    for (unsigned int j = 0; j < faces.size(); j += 3) {
        int index_1 = faces.at(j);
        int index_2 = faces.at(j + 1);
        int index_3 = faces.at(j + 2);

        glVertex3f(vertices.at(index_1).position.x, vertices.at(index_1).position.y, vertices.at(index_1).position.z);
        glVertex3f(vertices.at(index_2).position.x, vertices.at(index_2).position.y, vertices.at(index_2).position.z);
        glVertex3f(vertices.at(index_3).position.x, vertices.at(index_3).position.y, vertices.at(index_3).position.z);

    }
    glEnd();

    glPopMatrix();


    glutSwapBuffers();
}


void reshape(int w, int h) {
    width = w;
    height = h;

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-10, 10, -10, 10, -10, 55);

    glViewport(0, 0, w, h);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    //gluPerspective(60.0f, (GLfloat)w / (GLfloat)h, 1.0f, 200.0f);

    gluLookAt(
        -0.5, -0.5, -1.0,
        5.0, 5.0, 27.5,
        0.0, 0.0, 1.0
    );

    glEnable(GL_DEPTH_TEST);
}


void timer(int value) {
    if (position >= dots.size() - 1) {
        position = 0;
    }

    glutPostRedisplay();

    position++;

    glutTimerFunc(100, timer, 0);
}


int main(int argc, char** argv) {
    glutInit(&argc, argv);

    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
    glutInitWindowSize(width, height);
    glutInitWindowPosition(100, 100);

    glutCreateWindow("OBJ viewer");
    glutReshapeFunc(reshape);

    loadBSpline(argv, "bspline_0.txt");
    calculate_spline();

    //loadModel(argv, "tetrahedron.obj");
    loadModel(argv, "bird.obj");

    glutDisplayFunc(myDisplay);

    glutTimerFunc(100, timer, 0);

    glutMainLoop();

    return 0;
}
