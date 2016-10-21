#include "window.h"


const char* window_title = "GLFW Starter Project";
Cube * cube;
GLint shaderProgram;


// Initialize objects
OBJObject * bunny;
OBJObject * dragon;
OBJObject * bear;

// Variables indicating which model to load
bool showBunny = false;
bool showBear = false;
bool showDragon = false;

// Settings, keep values positive
const float X_POS_MODIFIER = 1.0f;			// How much to move right on x-axis
const float Y_POS_MODIFIER = 1.0f;			// How much to move up on y-axis
const float Z_POS_MODIFIER = 1.0f;			// How much to move away from screen on z-axis
const float SCALE_UP_MODIFIER = 1.1f;		// Factor for how much to scale object up by
const float SCALE_DOWN_MODIFIER = 0.9f;		// Factor for how much to scale object down by
const float ORBIT_MODIFIER = 20.0f;			// How much to orbit the object by

// Callback variables
float cursor_x = 0;
float cursor_y = 0;
glm::vec3 old_location;
bool lmb = false;
bool rmb = false;
bool button_down = false;

// Lights
Light light;

// Light settings
bool LIGHT_MODE = false;
bool DIRECTIONAL = true;
bool POINT = false;
bool SPOT = false;

// On some systems you need to change this to the absolute path
#define VERTEX_SHADER_PATH "../shader.vert"
#define FRAGMENT_SHADER_PATH "../shader.frag"

// Default camera parameters
glm::vec3 cam_pos(0.0f, 0.0f, 20.0f);		// e  | Position of camera
glm::vec3 cam_look_at(0.0f, 0.0f, 0.0f);	// d  | This is where the camera looks at
glm::vec3 cam_up(0.0f, 1.0f, 0.0f);			// up | What orientation "up" is

int Window::width;
int Window::height;

glm::mat4 Window::P;
glm::mat4 Window::V;

void Window::initialize_objects()
{
	// Bunny
	bunny = new OBJObject("bunny.obj");
	bunny->setAmbient(0.92f, 0.2f, 0.2f);
	bunny->setDiffuse(0.3f, 0.2f, 0.2f);
	bunny->setSpecular(0.9f, 0.9f, 0.9f);
	bunny->setShininess(127);

	// Dragon
	dragon = new OBJObject("dragon.obj");
	dragon->setAmbient(0.1f, 0.9f, 0.1f);
	dragon->setDiffuse(0.6f, 0.6f, 0.3f);
	dragon->setSpecular(0.7f, 0.8f, 0.6f);
	dragon->setShininess(50);

	// Warren Bear
	bear = new OBJObject("bear.obj");
	bear->setAmbient(0.3f, 0.1f, 1.0f);
	bear->setDiffuse(0.6f, 0.6f, 0.6f);
	bear->setSpecular(0.2f, 0.2f, 0.2f);
	bear->setShininess(1);

	light = Light();

	// Load the shader program. Make sure you have the correct filepath up top
	shaderProgram = LoadShaders(VERTEX_SHADER_PATH, FRAGMENT_SHADER_PATH);
}

// Treat this as a destructor function. Delete dynamically allocated memory here.
void Window::clean_up()
{
	delete(cube);
	delete(bunny);
	delete(dragon);
	delete(bear);
	glDeleteProgram(shaderProgram);
}

GLFWwindow* Window::create_window(int width, int height)
{
	// Initialize GLFW
	if (!glfwInit())
	{
		fprintf(stderr, "Failed to initialize GLFW\n");
		return NULL;
	}

	// 4x antialiasing
	glfwWindowHint(GLFW_SAMPLES, 4);

#ifdef __APPLE__ // Because Apple hates comforming to standards
	// Ensure that minimum OpenGL version is 3.3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	// Enable forward compatibility and allow a modern OpenGL context
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	// Create the GLFW window
	GLFWwindow* window = glfwCreateWindow(width, height, window_title, NULL, NULL);

	// Check if the window could not be created
	if (!window)
	{
		fprintf(stderr, "Failed to open GLFW window.\n");
		fprintf(stderr, "Either GLFW is not installed or your graphics card does not support modern OpenGL.\n");
		glfwTerminate();
		return NULL;
	}

	// Make the context of the window
	glfwMakeContextCurrent(window);

	// Set swap interval to 1
	glfwSwapInterval(1);

	// Get the width and height of the framebuffer to properly resize the window
	glfwGetFramebufferSize(window, &width, &height);
	// Call the resize callback to make sure things get drawn immediately
	Window::resize_callback(window, width, height);

	return window;
}

void Window::resize_callback(GLFWwindow* window, int width, int height)
{
	Window::width = width;
	Window::height = height;
	// Set the viewport size. This is the only matrix that OpenGL maintains for us in modern OpenGL!
	glViewport(0, 0, width, height);

	if (height > 0)
	{
		P = glm::perspective(45.0f, (float)width / (float)height, 0.1f, 1000.0f);
		V = glm::lookAt(cam_pos, cam_look_at, cam_up);
	}
}

void Window::idle_callback()
{
	// Call the update function
	if (showBunny) {
		bunny->update();
	}
	
	else if (showDragon) {
		dragon->update();
	}

	else if (showBear) {
		bear->update();
	}
}

void Window::display_callback(GLFWwindow* window)
{
	// Clear the color and depth buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Use the shader of programID
	glUseProgram(shaderProgram);


	light.update(shaderProgram);

	if (showBunny)
		bunny->draw(shaderProgram);
	else if (showBear)
		bear->draw(shaderProgram);
	else if (showDragon)
		dragon->draw(shaderProgram);

	// Gets events, including input such as keyboard and mouse or window resizing
	glfwPollEvents();
	// Swap buffers
	glfwSwapBuffers(window);
}

glm::vec3 Window::trackball(float x, float y)    // Use separate x and y values for the mouse location
{
	glm::vec3 v;    // Vector v is the synthesized 3D position of the mouse location on the trackball
	float d;     // this is the depth of the mouse location: the delta between the plane through the center of the trackball and the z position of the mouse
	v.x = (2.0f*x - Window::width) / Window::width;   // this calculates the mouse X position in trackball coordinates, which range from -1 to +1
	v.y = (Window::height - 2.0f*y) / Window::height;   // this does the equivalent to the above for the mouse Y position
	v.z = 0.0;   // initially the mouse z position is set to zero, but this will change below
	d = (float)v.length();    // this is the distance from the trackball's origin to the mouse location, without considering depth (=in the plane of the trackball's origin)
	d = (d<1.0) ? d : 1.0f;   // this limits d to values of 1.0 or less to avoid square roots of negative values in the following line
	v.z = (float)sqrtf(1.001 - d*d);  // this calculates the Z coordinate of the mouse position on the trackball, based on Pythagoras: v.z*v.z + d*d = 1*1
	v = normalize(v); // Still need to normalize, since we only capped d, not v.
	return v;  // return the mouse location on the surface of the trackball
}

glm::vec3 Window::trackball_translate(float x, float y)    // Use separate x and y values for the mouse location
{
	glm::vec3 v;    // Vector v is the synthesized 3D position of the mouse location on the trackball
	float d;     // this is the depth of the mouse location: the delta between the plane through the center of the trackball and the z position of the mouse
	v.x = (2.0f*x - Window::width) / Window::width;   // this calculates the mouse X position in trackball coordinates, which range from -1 to +1
	v.y = (Window::height - 2.0f*y) / Window::height;   // this does the equivalent to the above for the mouse Y position
	v.z = 0.0f;   // initially the mouse z position is set to zero, but this will change below
	d = (float)v.length();    // this is the distance from the trackball's origin to the mouse location, without considering depth (=in the plane of the trackball's origin)
	d = (d<1.0) ? d : 1.0f;   // this limits d to values of 1.0 or less to avoid square roots of negative values in the following line
	v.z = sqrtf(1.001 - d*d);  // this calculates the Z coordinate of the mouse position on the trackball, based on Pythagoras: v.z*v.z + d*d = 1*1
	v.x = v.x * 19.5;
	v.y = v.y * 12.5;
	return v;  // return the mouse location on the surface of the trackball
}

void Window::cursor_callback(GLFWwindow* window, double xpos, double ypos)
{
	cursor_x = xpos;
	cursor_y = ypos;

	// Rotate object
	if (lmb && button_down && !LIGHT_MODE) {
		glm::vec3 location = trackball(cursor_x, cursor_y);
		glm::vec3 direction = location - old_location;
		float velocity = direction.length();

		if (velocity > 0.0001) {
			glm::vec3 rot_axis;
			rot_axis = cross(old_location, location);
			velocity = velocity / (1.25);
			rot_axis[2] = 0.0f;
			if (showBunny)
				bunny->rotate(velocity, rot_axis);
			else if (showBear)
				bear->rotate(velocity, rot_axis);
			else if (showDragon)
				dragon->rotate(velocity, rot_axis);
		}
	}

	// Rotate directional light
	else if (lmb && button_down && LIGHT_MODE && DIRECTIONAL) {
		glm::vec3 location = trackball(cursor_x, cursor_y);
		glm::vec3 direction = location - old_location;
		float velocity = direction.length();

		if (velocity > 0.0001) {
			light.d_direction = location;
		}
	}

	// Rotate point light
	if (lmb && button_down && LIGHT_MODE && POINT) {
		glm::vec3 location = trackball(cursor_x, cursor_y);
		glm::vec3 direction = location - old_location;
		float velocity = direction.length();

		if (velocity > 0.0001) {
			glm::vec3 rot_axis;
			rot_axis = cross(old_location, location);
			velocity = velocity / (2.25);
			rot_axis[2] = 0.0f;
			glm::vec4 new_pos = glm::rotate(glm::mat4(1.0f), velocity / 180.0f * glm::pi<float>(), rot_axis) * glm::vec4(light.p_position, 1.0f);
			light.p_position.x = new_pos.x;
			light.p_position.y = new_pos.y;
			light.p_position.z = new_pos.z;
		}
	}

	// Rotate spot light
	if (lmb && button_down && LIGHT_MODE && SPOT) {
		glm::vec3 location = trackball(cursor_x, cursor_y);
		glm::vec3 direction = location - old_location;
		float velocity = direction.length();

		if (velocity > 0.0001) {
			glm::vec3 rot_axis;
			rot_axis = cross(old_location, location);
			velocity = velocity / (3.0f);
			rot_axis[2] = 0.0f;
			glm::vec4 new_pos = glm::rotate(glm::mat4(1.0f), velocity / 180.0f * glm::pi<float>(), rot_axis) * glm::vec4(light.s_position, 1.0f);
			light.s_position.x = new_pos.x;
			light.s_position.y = new_pos.y;
			light.s_position.z = new_pos.z;
		}
		//std::cout << light.s_position.x << ", " << light.s_position.y << ", " << light.s_position.z << std::endl;
	}

	// Change spot light size
	else if (button_down && rmb && LIGHT_MODE && SPOT) {
		glm::vec3 location = trackball(cursor_x, cursor_y);
		glm::vec3 direction = location - old_location;
		float velocity = direction.length();
		if (velocity > 0.0001) {
			velocity = velocity / 10.0f;
			if (direction.y < 0 && ((light.cutOff - velocity) > 0.0f) && ((light.outerCutOff - velocity) > 0.0f)) {
				light.cutOff = light.cutOff - velocity;
				light.outerCutOff = light.outerCutOff - velocity;
				light.cos_cutOff = pow(glm::cos(glm::radians(light.cutOff)), light.cos_exp);
				light.cos_outerCutOff = pow(glm::cos(glm::radians(light.outerCutOff)), light.cos_exp);
			}
			else if (direction.y > 0) {
				light.cutOff = light.cutOff + velocity;
				light.outerCutOff = light.outerCutOff + velocity;
				light.cos_cutOff = pow(glm::cos(glm::radians(light.cutOff)), light.cos_exp);
				light.cos_outerCutOff = pow(glm::cos(glm::radians(light.outerCutOff)), light.cos_exp);
			}
		}
	}

	// Translate object
	else if (rmb && button_down && !LIGHT_MODE) {
		glm::vec3 location = trackball_translate(cursor_x, cursor_y);
		if (showBunny) {
			bunny->origin_preserve_z();
			bunny->translate(location.x, location.y, 0.0);
		}
		else if (showBear) {
			bear->origin_preserve_z();
			bear->translate(location.x, location.y, 0.0);
		}
		else if (showDragon) {
			dragon->origin_preserve_z();
			dragon->translate(location.x, location.y, 0.0);
		}
	}
}

void Window::mouse_callback(GLFWwindow* window, int button, int action, int mods)
{
	if (action == GLFW_PRESS) {
		button_down = true;
		old_location = trackball(cursor_x, cursor_y);

		if (button == GLFW_MOUSE_BUTTON_1) {
			lmb = true;
			rmb = false;
		}

		if (button == GLFW_MOUSE_BUTTON_2) {
			lmb = false;
			rmb = true;
		}
	}
	else {
		button_down = false;
		lmb = false;
		rmb = false;
	}
}

void Window::scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	if (yoffset != 0 && !LIGHT_MODE) {
		if (showBunny) {
			bunny->translate(0.0, 0.0, (float)-yoffset);
		}
		else if (showDragon) {
			dragon->translate(0.0, 0.0, (float)-yoffset);
		}
		else if (showBear) {
			bear->translate(0.0, 0.0, (float)-yoffset);
		}
	}

	else if (LIGHT_MODE && POINT) {
		if (yoffset > 0)
			light.setPos(light.p_position.x, light.p_position.y, light.p_position.z - 1);
		else if (yoffset < 0)
			light.setPos(light.p_position.x, light.p_position.y, light.p_position.z + 1);
	}

	else if (LIGHT_MODE && SPOT) {
			if (yoffset > 0)
				light.setSPos(light.s_position.x, light.s_position.y, light.s_position.z - 1);
			else if (yoffset < 0)
				light.setSPos(light.s_position.x, light.s_position.y, light.s_position.z + 1);
	}
}


void Window::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	// Check for a key press
	if (action == GLFW_PRESS)
	{
		// Check if escape was pressed
		if (key == GLFW_KEY_ESCAPE)
		{
			// Close the window. This causes the program to also terminate.
			glfwSetWindowShouldClose(window, GL_TRUE);
		}

		// F1 BUNNY
		else if (key == GLFW_KEY_F1)
		{
			bunny->reset();
			// Show the bunny
			if (!showBunny)
			{
				showBunny = true;
				showBear = false;
				showDragon = false;
			}
		}

		// F2 DRAGON
		else if (key == GLFW_KEY_F2)
		{
			dragon->reset();
			// Show the dragon
			if (!showDragon)
			{
				showBunny = false;
				showBear = false;
				showDragon = true;
			}
		}

		// F3 BEAR
		else if (key == GLFW_KEY_F3)
		{
			bear->reset();
			// Show the bear
			if (!showBear)
			{
				showBunny = false;
				showBear = true;
				showDragon = false;
			}
		}
		
		// X MOVE X
		else if (key == GLFW_KEY_X)
		{
			// X
			if (mods == GLFW_MOD_SHIFT)
			{
				if (showBunny)
				{
					bunny->translate(X_POS_MODIFIER, 0, 0);
				}
				else if (showDragon)
				{
					dragon->translate(X_POS_MODIFIER, 0, 0);
				}
				else if (showBear)
				{
					bear->translate(X_POS_MODIFIER, 0, 0);
				}

			}
			// x
			else
			{
				if (showBunny)
				{
					bunny->translate(-X_POS_MODIFIER, 0, 0);
				}
				else if (showDragon)
				{
					dragon->translate(-X_POS_MODIFIER, 0, 0);
				}
				else if (showBear)
				{
					bear->translate(-X_POS_MODIFIER, 0, 0);
				}
			}

		}
		
		// Y MOVE Y
		else if (key == GLFW_KEY_Y)
		{
			if (!showBunny && !showDragon && !showBear)
			{
				return;
			}

			// Y
			if (mods == GLFW_MOD_SHIFT)
			{

				if (showBunny)
				{
					bunny->translate(0, Y_POS_MODIFIER, 0);
				}
				else if (showDragon)
				{
					dragon->translate(0, Y_POS_MODIFIER, 0);
				}
				else if (showBear)
				{
					bear->translate(0, Y_POS_MODIFIER, 0);
				}

			}

			// y
			else
			{

				if (showBunny)
				{
					bunny->translate(0, -Y_POS_MODIFIER, 0);
				}
				else if (showDragon)
				{
					dragon->translate(0, -Y_POS_MODIFIER, 0);
				}
				else if (showBear)
				{
					bear->translate(0, -Y_POS_MODIFIER, 0);
				}
			}

		}

		// Z MOVE Z
		else if (key == GLFW_KEY_Z)
		{
			// Z
			if (mods == GLFW_MOD_SHIFT)
			{
				if (showBunny)
				{
					bunny->translate(0, 0, Z_POS_MODIFIER);
				}
				else if (showDragon)
				{
					dragon->translate(0, 0, Z_POS_MODIFIER);
				}
				else if (showBear)
				{
					bear->translate(0, 0, Z_POS_MODIFIER);
				}
			}

			// z
			else
			{
				if (showBunny)
				{
					bunny->translate(0, 0, -Z_POS_MODIFIER);
				}
				else if (showDragon)
				{
					dragon->translate(0, 0, -Z_POS_MODIFIER);
				}
				else if (showBear)
				{
					bear->translate(0, 0, -Z_POS_MODIFIER);
				}
			}
		}

		// S SCALE MODEL
		else if (key == GLFW_KEY_S)
		{
			// S
			if (mods == GLFW_MOD_SHIFT)
			{
				if (showBunny)
				{
					bunny->scale(SCALE_UP_MODIFIER);
				}
				else if (showBear)
				{
					bear->scale(SCALE_UP_MODIFIER);
				}
				else if (showDragon)
				{
					dragon->scale(SCALE_UP_MODIFIER);
				}
			}

			// s
			else
			{
				if (showBunny)
				{
					bunny->scale(SCALE_DOWN_MODIFIER);
				}
				else if (showBear)
				{
					bear->scale(SCALE_DOWN_MODIFIER);
				}
				else if (showDragon)
				{
					dragon->scale(SCALE_DOWN_MODIFIER);
				}
			}

		}

		// O ORBIT MODEL
		else if (key == GLFW_KEY_O)
		{
			// O
			if (mods == GLFW_MOD_SHIFT)
			{
				if (showBunny)
				{
					bunny->orbit(-ORBIT_MODIFIER);
				}
				else if (showBear)
				{
					bear->orbit(-ORBIT_MODIFIER);
				}
				else if (showDragon)
				{
					dragon->orbit(-ORBIT_MODIFIER);
				}
			}

			// o
			else
			{
				if (showBunny)
				{
					bunny->orbit(ORBIT_MODIFIER);
				}
				else if (showBear)
				{
					bear->orbit(ORBIT_MODIFIER);
				}
				else if (showDragon)
				{
					dragon->orbit(ORBIT_MODIFIER);
				}
			}

		}

		// R RESET
		else if (key == GLFW_KEY_R)
		{
			if (mods == GLFW_MOD_SHIFT) {
				light = Light();
			}
			else {
				if (showBunny)
				{
					bunny->reset();
				}
				else if (showDragon)
				{
					dragon->reset();
				}
				else if (showBear)
				{
					bear->reset();
				}
			}
		}
		else if (key == GLFW_KEY_0) {
			LIGHT_MODE = false;
		}
		else if (key == GLFW_KEY_1) {
			DIRECTIONAL = true;
			POINT = false;
			SPOT = false;
			LIGHT_MODE = true;
		}
		else if (key == GLFW_KEY_2) {
			DIRECTIONAL = false;
			POINT = true;
			SPOT = false;
			LIGHT_MODE = true;
		}
		else if (key == GLFW_KEY_3) {
			DIRECTIONAL = false;
			POINT = false;
			SPOT = true;
			LIGHT_MODE = true;
		}
	}

	else if (key == GLFW_KEY_F) {
		if (light.lights_on == 1) {
			light.lights_on = 0;
		}
		else {
			light.lights_on = 1;
		}
	}

	else if (key == GLFW_KEY_E && LIGHT_MODE && SPOT) {
		if (mods == GLFW_MOD_SHIFT) {
			if (light.cos_exp >= 128)
				light.cos_exp = 128;
			else {
				light.cos_exp = light.cos_exp * 2;
				//light.cos_cutOff = pow(glm::cos(glm::radians(light.cutOff)), light.cos_exp);
				light.cos_outerCutOff = pow(glm::cos(glm::radians(light.outerCutOff)), light.cos_exp);
			}
		}
		else {
			if (light.cos_exp <= 1) {
				light.cos_exp = 1;
			}
			else {
				light.cos_exp = light.cos_exp / 2;
				//light.cos_cutOff = pow(glm::cos(glm::radians(light.cutOff)), light.cos_exp);
				light.cos_outerCutOff = pow(glm::cos(glm::radians(light.outerCutOff)), light.cos_exp);
			}
		}
	}
}