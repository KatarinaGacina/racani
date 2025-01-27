#pragma once

#include <vector>

#include "SphereTime.h"
#include "BSpline.h"

class Sustav {
    private:
        std::vector<glm::vec3> colorsTime;

        bool kraj;

        int n;
        float r;

        int t;
        int t_n;

        int max_t;

        std::vector<glm::vec2> pozicije;

    public:
        Sustav();

        void init(int n1, float r1, BSpline& bspline);

        int getSize();

        std::vector<glm::vec2>& getPositionReference();
        std::vector<glm::vec3>& getColors();

        int getFirstT();
        int getLastT();
        
        bool getKraj();

        int insertSpheresTime(glm::vec3 color, int t1);

        std::pair<int, int> findIndexes(int i);
        void removeSpheresTime(int start, int end);

        bool updateSustav();

        void drawSustav();

        ~Sustav();
};