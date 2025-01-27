#include "Particle.h"

const glm::vec3 Particle::s = { 0.0, 0.0, 1.0 };

Particle::Particle(glm::vec3 ociste) {
	create_new(ociste);

	position = { (std::rand() % 16), (std::rand() % 16), (std::rand() % 16) };
}

void Particle::create_new(glm::vec3 ociste) {
	current_age = 0;
	max_age = 50 + std::rand() % 201;

	position = { (std::rand() % 16), 15.0, (std::rand() % 16) };

	orientation = ociste - position;
	orientation = glm::normalize(orientation);

	os = glm::normalize(glm::cross(s, orientation));
	angle = glm::degrees(std::acos(glm::dot(glm::normalize(s), orientation)));
}

void Particle::update(glm::vec3 ociste, glm::vec3 wind) {
	current_age += 1;

	size = 0.3f * (1.0 - (current_age / max_age));

	position.x += wind.x * sin((position.x + position.y + position.z) / 45.0);
	position.y -= 0.1f; //gravitacija

	if (position.x < 0.0) {
		position.x += 15.0;
	}
	else if (position.x > 15.0) {
		position.x -= 15.0;
	}

	orientation = ociste - position;
	orientation = glm::normalize(orientation);

	os = glm::normalize(glm::cross(s, orientation));
	angle = glm::degrees(std::acos(glm::dot(glm::normalize(s), orientation)));


	if (current_age >= max_age or position.y <= 0) {
		create_new(ociste);
	}
}

float Particle::getSize() {
	return size;
}

glm::vec3 Particle::getOs() {
	return os;
}

float Particle::getKut() {
	return angle;
}

glm::vec3 Particle::getPosition() {
	return position;
}

Particle::~Particle() {
}