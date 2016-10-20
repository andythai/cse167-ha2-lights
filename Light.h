#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>

class Light 
{
public:
	// Constructors
	Light();
	~Light();

	// Methods
	void setPos(float x, float y, float z);
	void setSPos(float x, float y, float z);
	void update(GLuint shaderProgram);

	// Settings
	glm::vec3 d_direction = { -0.2f, -1.0f, -0.3f }; 
	glm::vec3 p_position = { 0.0f, 0.0f, 5.2f }; // Point light position
	glm::vec3 light_color = { 0.7f, 0.7f, 0.7f }; // Point light color
	glm::vec3 s_position = { -0.911678, 0.026234, 3.89464 }; // Spot light
	int cos_exp = 1;
	float attenuation = 0.032f;
	int lights_on = 1;
	float cutOff = 12.5f;
	float outerCutOff = 13.0f;
	float cos_cutOff = pow(glm::cos(glm::radians(cutOff)), cos_exp);
	float cos_outerCutOff = pow(glm::cos(glm::radians(outerCutOff)), cos_exp);
	// Sent to GPU
	GLuint uLight_on;
};