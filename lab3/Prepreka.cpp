#include "Prepreka.h"

Prepreka::Prepreka() {
}

void Prepreka::init(glm::vec2 pos, float radius) {
	position = pos;
	r = radius;
}

bool Prepreka::checkCollision(glm::vec2 zraka) {
	zraka = glm::normalize(zraka);

	float a = 1.0;
	float b = 2.0 * glm::dot(zraka, position);
	float c = glm::dot(position, position) - r * r;

	float d = (b * b - 4.0f * a * c);

	if (d < 0.0) {
		return false;
	}

	float sqrtd = sqrt(d);
	float t1 = (-b - sqrtd) / (2.0f * a);
	float t2 = (-b + sqrtd) / (2.0f * a);

	//std::cout << t1 << std::endl;
	//std::cout << t2 << std::endl;

	if (t1 <= 0.0f) {
		return true;
	}
	else if (t2 <= 0.0f) {
		return true;
	}
	else {
		return false;
	}

}

void Prepreka::drawPrepreka() {
	glPushMatrix();
	glTranslatef(position.x, position.y, 0.0);
	glColor3f(1.0, 1.0, 1.0);
	GLUquadric* quadric = gluNewQuadric();
	gluSphere(quadric, r, 32, 32);
	gluDeleteQuadric(quadric);
	glPopMatrix();
}

Prepreka::~Prepreka() {
}