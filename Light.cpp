#define _CRT_SECURE_NO_DEPRECATE
#include "Light.h"

Light::Light()
{
}

Light::~Light()
{
}

void Light::update(GLuint shaderProgram)
{
	// Light color
	uLight_on = glGetUniformLocation(shaderProgram, "on");

	// Above
	glUniform1i(uLight_on, lights_on);

	// Directional light
	glUniform3f(glGetUniformLocation(shaderProgram, "dirLight.direction"), d_direction.x, d_direction.y, d_direction.z);
	glUniform3f(glGetUniformLocation(shaderProgram, "dirLight.ambient"), 0.3f, 0.24f, 0.14f);
	glUniform3f(glGetUniformLocation(shaderProgram, "dirLight.diffuse"), 0.7f, 0.42f, 0.26f);
	glUniform3f(glGetUniformLocation(shaderProgram, "dirLight.specular"), 0.5f, 0.5f, 0.5f);

	// Point light
	glUniform3f(glGetUniformLocation(shaderProgram, "pointLight.position"), p_position.x, p_position.y, p_position.z);
	glUniform3f(glGetUniformLocation(shaderProgram, "pointLight.ambient"), light_color.r * 0.1, light_color.g * 0.1, light_color.b * 0.1);
	glUniform3f(glGetUniformLocation(shaderProgram, "pointLight.diffuse"), light_color.r, light_color.g, light_color.b);
	glUniform3f(glGetUniformLocation(shaderProgram, "pointLight.specular"), light_color.r, light_color.g, light_color.b);
	glUniform1f(glGetUniformLocation(shaderProgram, "pointLight.quadratic"), attenuation);

	// Spot light
	glUniform3f(glGetUniformLocation(shaderProgram, "spotLight.direction"), 0.0f, 0.0f, -1.0f);
	glUniform3f(glGetUniformLocation(shaderProgram, "spotLight.position"), s_position.x, s_position.y, s_position.z);
	glUniform3f(glGetUniformLocation(shaderProgram, "spotLight.ambient"), 0.0f, 0.0f, 0.0f);
	glUniform3f(glGetUniformLocation(shaderProgram, "spotLight.diffuse"), 0.8f, 0.8f, 0.0f);
	glUniform3f(glGetUniformLocation(shaderProgram, "spotLight.specular"), 0.8f, 0.8f, 0.8f);
	glUniform1f(glGetUniformLocation(shaderProgram, "spotLight.quadratic"), attenuation);
	glUniform1f(glGetUniformLocation(shaderProgram, "spotLight.cutOff"), glm::cos(glm::radians(cutOff)));
	glUniform1f(glGetUniformLocation(shaderProgram, "spotLight.outerCutOff"), glm::cos(glm::radians(outerCutOff)));
}

void Light::setPos(float x, float y, float z)
{
	p_position = { x, y, z };
}

void Light::setSPos(float x, float y, float z)
{
	s_position = { x, y, z };
}
