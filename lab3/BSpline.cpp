#include "BSpline.h"

BSpline::BSpline() {
}

void BSpline::init(int p1, const std::string& filename) {
    p = p1;

    loadBSplinePoints(filename);
    calculateBSpline();
}

void BSpline::loadBSplinePoints(const std::string& filename) {
    std::string filePath(filename);

    std::ifstream file(filePath.c_str());
    std::string line;

    bspline_control_dots.clear();

    if (file.is_open()) {
        while (std::getline(file, line, '\n')) {
            std::stringstream ss(line);
            std::string value;

            glm::vec3 dot;

            std::getline(ss, value, ',');
            dot.x = std::stof(value);
            std::getline(ss, value, ',');
            dot.y = std::stof(value);

            bspline_control_dots.push_back(dot);
        }

        file.close();
    }
    else {
        std::cerr << "Error opening the file!" << std::endl;
        exit(1);
    }

    if (bspline_control_dots.size() < 4) {
        exit(1);
    }
}

void BSpline::calculateBSpline() {

    glm::vec3 spline_dot;

    int dot_num = (bspline_control_dots.size() - 3) * p;

    dots.reserve(dot_num);
    dots.clear();

    for (unsigned int i = 0; i < bspline_control_dots.size() - 3; i++) {
        for (unsigned int j = 0; j < p; ++j) {

            float t = (float)j / p;

            float v1 = (-1 * t * t * t + 3 * t * t - 3 * t + 1) / 6.0f;
            float v2 = (3 * t * t * t - 6 * t * t + 4) / 6.0f;
            float v3 = (-3 * t * t * t + 3 * t * t + 3 * t + 1) / 6.0f;
            float v4 = (t * t * t) / 6.0f;

            spline_dot.x = v1 * bspline_control_dots.at(i).x +
                v2 * bspline_control_dots.at(i + 1).x +
                v3 * bspline_control_dots.at(i + 2).x +
                v4 * bspline_control_dots.at(i + 3).x;
            spline_dot.y = v1 * bspline_control_dots.at(i).y +
                v2 * bspline_control_dots.at(i + 1).y +
                v3 * bspline_control_dots.at(i + 2).y +
                v4 * bspline_control_dots.at(i + 3).y;

            dots.push_back(spline_dot);
        }
    }
}

void BSpline::calculateSpacedDots(float d) {
    if (dots.size() <= 0) {
        return;
    }

    spacedDots.clear();

    glm::vec2 lastDot = dots[0];
    spacedDots.push_back(lastDot);

    int br = 1;
    for (size_t i = 1; i < dots.size(); ++i) {
        float distance = glm::distance(lastDot, dots[i]);

        if (distance >= d) {
            spacedDots.push_back(dots[i]);
            lastDot = dots[i];
        }
    }
}

glm::vec2 BSpline::getSpacedDotsAt(int i) {
    return spacedDots[i];
}

int BSpline::getSpacedDotsSize() {
    return spacedDots.size();
}

void BSpline::drawSpline() {
    glLineWidth(1.0f);
    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_LINE_STRIP);
    for (const auto& dot : dots) {
        glVertex2f(dot.x, dot.y);
    }
    glEnd();
}

std::vector<glm::vec2>& BSpline::getDots() {
    return dots;
}

BSpline::~BSpline() {
}