#pragma once

#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp> 

#include <GL/glut.h>

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp> 

#include <cstdlib>
#include <ctime>

#include <iostream>
#include <fstream>
#include <sstream>

#include <vector>

#include <map>

class BSpline {
private:
    float p;

    std::vector<glm::vec2> bspline_control_dots;
    std::vector<glm::vec2> dots;
    std::vector<glm::vec2> spacedDots;

    void loadBSplinePoints(const std::string& filename);

public:

    BSpline();

    void init(int p1, const std::string& filename);

    void calculateBSpline();
    void calculateSpacedDots(float d);

    glm::vec2 getSpacedDotsAt(int i);
    int getSpacedDotsSize();

    std::vector<glm::vec2>& getDots();

    void drawSpline();

    ~BSpline();
};