#pragma once

#include <iostream>

#include <cstdlib>
#include <ctime>

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp> 

class Particle {
private:
    float max_age;
    float current_age;

    float size;

    glm::vec3 position;

    glm::vec3 os;
    float angle;

    glm::vec3 orientation; //pom

    void create_new(glm::vec3 ociste);

public:
    static const glm::vec3 s; //pocetna orijentacija

    Particle(glm::vec3 ociste);
    ~Particle();

    void update(glm::vec3 ociste, glm::vec3 wind);

    float getSize();
    glm::vec3 getOs();
    float getKut();
    glm::vec3 getPosition();
};