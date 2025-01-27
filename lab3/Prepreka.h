
#pragma once

#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp> 

#include <GL/glut.h>

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp> 

class Prepreka {
private:

public:
    glm::vec2 position;
    float r;

    Prepreka();

    void init(glm::vec2 pos, float radius);

    void drawPrepreka();

    bool checkCollision(glm::vec2 zraka);

    ~Prepreka();
};