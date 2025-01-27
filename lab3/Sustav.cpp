
#include "Sustav.h"

Sustav::Sustav() {
}

void Sustav::init(int n1, float r1, BSpline& bspline) {
    n = n1;
    r = r1;

    t = 0;
    t_n = 0;

    max_t = bspline.getSpacedDotsSize();

    colorsTime.reserve(n);
    colorsTime.clear();

    pozicije.clear();
    pozicije.reserve(bspline.getSpacedDotsSize());

    for (int i = 0; i < bspline.getSpacedDotsSize(); i++) {
        pozicije.push_back(bspline.getSpacedDotsAt(i));
    }

    kraj = false;
}

std::vector<glm::vec2>& Sustav::getPositionReference() {
    return pozicije;
}

std::vector<glm::vec3>& Sustav::getColors() {
    return colorsTime;
}

int Sustav::getFirstT() {
    return t;
}

int Sustav::getLastT() {
    return t + t_n - 1;
}

int Sustav::getSize() {
    return t_n;
}

bool Sustav::getKraj() {
    return kraj;
}

int Sustav::insertSpheresTime(glm::vec3 color, int t1) {
    int insertion_index;

    if (colorsTime.empty() || t1 < t) {
        insertion_index = 0;
        t = t1;
        colorsTime.insert(colorsTime.begin(), color);
    }
    else if (t1 > (t + t_n - 1)) {
        insertion_index = t_n;
        colorsTime.push_back(color);
    }
    else {
        insertion_index = t1 - t;
        colorsTime.insert(colorsTime.begin() + insertion_index, color);
    }

    t_n++;
    if (t + t_n >= n) {
        kraj = true;
    }

    return insertion_index;
}

std::pair<int, int> Sustav::findIndexes(int i) {
    int left = i;
    int right = i;

    if (i < 0 || i >= colorsTime.size()) {
        return { left, right };
    }

    while (left > 0 && colorsTime[left - 1] == colorsTime[i]) {
        left--;
    }

    while (right < colorsTime.size() - 1 && colorsTime[right + 1] == colorsTime[i]) {
        right++;
    }

    return { left, right };;
}

void Sustav::removeSpheresTime(int start, int end) {
    int pom = t;

    for (int i = start; i <= end; i++) {
        colorsTime.erase(colorsTime.begin() + start);
    }

    t_n -= (end - start + 1);
}

bool Sustav::updateSustav() {
    t++;
    if (t + t_n - 1 >= max_t) {
        kraj = true;
    }

    return kraj;
}

void Sustav::drawSustav() {
   for (int i = 0; i < t_n; i++) {
        glPushMatrix();
        glTranslatef(pozicije[t + i - 1].x, pozicije[t + i - 1].y, 0.0);
        glColor3f(colorsTime[i].x, colorsTime[i].y, colorsTime[i].z);
        GLUquadric* quadric = gluNewQuadric();
        gluSphere(quadric, 0.5, 32, 32);
        gluDeleteQuadric(quadric);
        glPopMatrix();
    }
}

Sustav::~Sustav() {
}
