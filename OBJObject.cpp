#define _CRT_SECURE_NO_DEPRECATE
#include "OBJObject.h"
#include "Window.h"

OBJObject::OBJObject()
{}

OBJObject::OBJObject(const char* filepath)
{
	parse(filepath);
	initialize();
}

OBJObject::OBJObject(bool cube) {
	vertices = {
		// "Front" vertices
		{ -2.0, -2.0,  2.0 },{ 2.0, -2.0,  2.0 },{ 2.0,  2.0,  2.0 },{ -2.0,  2.0,  2.0 },
		// "Back" vertices
		{ -2.0, -2.0, -2.0 },{ 2.0, -2.0, -2.0 },{ 2.0,  2.0, -2.0 },{ -2.0,  2.0, -2.0 }
	};
	unsigned int index_array[6][6] = {
		// Front face
		{ 0, 1, 2, 2, 3, 0 },
		// Top face
		{ 1, 5, 6, 6, 2, 1 },
		// Back face
		{ 7, 6, 5, 5, 4, 7 },
		// Bottom face
		{ 4, 0, 3, 3, 7, 4 },
		// Left face
		{ 4, 5, 1, 1, 0, 4 },
		// Right face
		{ 3, 2, 6, 6, 7, 3 } };
	for (int i = 0; i < 6; i++) {
		for (int j = 0; j < 6; j++) {
			indices.push_back(index_array[i][j]);
		}
	}
	initialize();
}

OBJObject::~OBJObject()
{
	// Delete previously generated buffers. Note that forgetting to do this can waste GPU memory in a 
	// large project! This could crash the graphics driver due to memory leaks, or slow down application performance!
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);
}

void OBJObject::initialize()
{
	toWorld = glm::mat4(1.0f);

	// Create array object and buffers. Remember to delete your buffers when the object is destroyed!
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO); // Stores large number of vertices
	glGenBuffers(1, &NBO);
	glGenBuffers(1, &EBO);

	// Bind the Vertex Array Object (VAO) first, then bind the associated buffers to it.
	// Consider the VAO as a container for all your buffers.
	glBindVertexArray(VAO);

	// Now bind a VBO to it as a GL_ARRAY_BUFFER. The GL_ARRAY_BUFFER is an array containing relevant data to what
	// you want to draw, such as vertices, normals, colors, etc.
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	// glBufferData populates the most recently bound buffer with data starting at the 3rd argument and ending after
	// the 2nd argument number of indices. How does OpenGL know how long an index spans? Go to glVertexAttribPointer.
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), vertices.data(), GL_STATIC_DRAW);

	// Enable the usage of layout location 0 (check the vertex shader to see what this is)
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0,// This first parameter x should be the same as the number passed into the line "layout (location = x)" in the vertex shader. In this case, it's 0. Valid values are 0 to GL_MAX_UNIFORM_LOCATIONS.
		3, // This second line tells us how any components there are per vertex. In this case, it's 3 (we have an x, y, and z component)
		GL_FLOAT, // What type these components are
		GL_FALSE, // GL_TRUE means the values should be normalized. GL_FALSE means they shouldn't
		3 * sizeof(GLfloat), // Offset between consecutive indices. Since each of our vertices have 3 floats, they should have the size of 3 floats in between
		(GLvoid*)0); // Offset of the first vertex's component. In our case it's 0 since we don't pad the vertices array with anything.

					 // We've sent the vertex data over to OpenGL, but there's still something missing.
					 // In what order should it draw those vertices? That's why we'll need a GL_ELEMENT_ARRAY_BUFFER for this.

	/* NBO */
	
	glBindBuffer(GL_ARRAY_BUFFER, NBO);
	glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), normals.data(), GL_STATIC_DRAW);
	
	// Enable the usage of layout location 1 (check the vertex shader to see what this is)
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(1,// This first parameter x should be the same as the number passed into the line "layout (location = x)" in the vertex shader.
		3, // This second line tells us how any components there are per vertex. In this case, it's 3 (we have an x, y, and z component)
		GL_FLOAT, // What type these components are
		GL_FALSE, // GL_TRUE means the values should be normalized. GL_FALSE means they shouldn't
		3 * sizeof(GLfloat), // Offset between consecutive indices. Since each of our vertices have 3 floats, they should have the size of 3 floats in between
		(GLvoid*)0); // Offset of the first vertex's component. In our case it's 0 since we don't pad the vertices array with anything.

					 // We've sent the vertex data over to OpenGL, but there's still something missing.
					 // In what order should it draw those vertices? That's why we'll need a GL_ELEMENT_ARRAY_BUFFER for this.
	
	/*END*/


	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);

	// Unbind the currently bound buffer so that we don't accidentally make unwanted changes to it.
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Unbind the VAO now so we don't accidentally tamper with it.
	// NOTE: You must NEVER unbind the element array buffer associated with a VAO!
	glBindVertexArray(0);
}

void OBJObject::parse(const char* filepath)
{
	//std::cout << std::endl << "Parsing: " << filepath << std::endl;
	FILE* fp;     // file pointer
	GLfloat x, y, z;  // vertex coordinates
	GLfloat r, g, b;  // vertex color
	GLuint f1_1, f1_2, f2_1, f2_2, f3_1, f3_2; // pairs of faces

	// Double parsing centering
	GLfloat min_x = FLT_MAX;
	GLfloat max_x = -FLT_MAX;
	GLfloat min_y = FLT_MAX; 
	GLfloat max_y = -FLT_MAX;
	GLfloat min_z = FLT_MAX; 
	GLfloat max_z = -FLT_MAX;

	int c1 = 0, c2 = 0;    // characters read from file

	glm::vec3 parser_vec3; // holds float values from parsing

	fp = fopen(filepath, "rb");  // make the file name configurable so you can load other files
	if (fp == NULL) {
		std::cerr << "error loading file" << std::endl;
		exit(-1);
	}  // just in case the file can't be found or is corrupt

	   // Repeat until end of file
	while (c1 != EOF || c2 != EOF) {
		c1 = fgetc(fp);
		c2 = fgetc(fp);

		// Check if line is a vertex
		if ((c1 == 'v') && (c2 == ' '))
		{
			fscanf(fp, "%f %f %f %f %f %f", &x, &y, &z, &r, &g, &b);
			parser_vec3 = glm::vec3(x, y, z);
			vertices.push_back(parser_vec3);
			if (x > max_x) {
				max_x = x;
			}
			else if (x < min_x) {
				min_x = x;
			}

			if (y > max_y) {
				max_y = y;
			}
			else if (y < min_y) {
				min_y = y;
			}

			if (z > max_z) {
				max_z = z;
			}
			else if (z < min_z) {
				min_z = z;
			}
		}

		// read normal data accordingly
		else if ((c1 == 'v') && (c2 == 'n'))
		{
			fscanf(fp, "%f %f %f", &x, &y, &z);
			parser_vec3 = glm::vec3(x, y, z);
			normals.push_back(parser_vec3);
		}

		else if ((c1 == 'f') && (c2 == ' ')) {
			fscanf(fp, "%u//%u %u//%u %u//%u", &f1_1, &f1_2, &f2_1, &f2_2, &f3_1, &f3_2);
			indices.push_back(f1_1 - 1);
			indices.push_back(f2_1 - 1);
			indices.push_back(f3_1 - 1);
		}
		else {
			ungetc(c2, fp);
		}
	}
	fclose(fp);   // make sure you don't forget to close the file when done

	GLfloat avg_x = (max_x - min_x) / 2;
	GLfloat avg_y = (max_y - min_y) / 2;
	GLfloat avg_z = (max_z - min_z) / 2;
	GLfloat size;

	if (max_x - min_x > max_y - min_y)
		size = max_x - min_x;
	else {
		size = max_y - min_y;
	}
	if (max_z - min_z > size) {
		size = max_z - min_z;
	}

	for (int i = 0; i < vertices.size(); i++) {
		vertices[i][0] = (vertices[i][0] - avg_x) / size;
		vertices[i][1] = (vertices[i][1] - avg_y) / size;
		vertices[i][2] = (vertices[i][2] - avg_z) / size;
	}

	//std::cout << "Parsing of " << filepath << " complete!" << std::endl;
}

void OBJObject::draw(GLuint shaderProgram)
{ 
	// Calculate the combination of the model and view (camera inverse) matrices
	glm::mat4 modelview = Window::V * toWorld;
	// We need to calculate this because modern OpenGL does not keep track of any matrix other than the viewport (D)
	// Consequently, we need to forward the projection, view, and model matrices to the shader programs

	// Get the location of the uniform variables "projection" and "modelview"
	uProjection = glGetUniformLocation(shaderProgram, "projection");
	uModelview = glGetUniformLocation(shaderProgram, "modelview");
	uModel = glGetUniformLocation(shaderProgram, "model");
	uView = glGetUniformLocation(shaderProgram, "view");
	uColor = glGetUniformLocation(shaderProgram, "material.ambient");
	uDiffuse = glGetUniformLocation(shaderProgram, "material.diffuse");
	uSpecular = glGetUniformLocation(shaderProgram, "material.specular");
	uShininess = glGetUniformLocation(shaderProgram, "material.shininess");

	// Now send these values to the shader program
	glUniformMatrix4fv(uProjection, 1, GL_FALSE, &Window::P[0][0]);
	glUniformMatrix4fv(uModelview, 1, GL_FALSE, &modelview[0][0]);
	glUniformMatrix4fv(uModel, 1, GL_FALSE, &toWorld[0][0]);
	glUniformMatrix4fv(uView, 1, GL_FALSE, &Window::V[0][0]);
	

	// Materials
	glUniform3f(uColor, object_color.r, object_color.g, object_color.b);
	glUniform3f(uDiffuse, diffuse.r, diffuse.g, diffuse.b);
	glUniform3f(uSpecular, specular.r, specular.g, specular.b);
	glUniform1f(uShininess, shininess);

	// Now draw the object. We simply need to bind the VAO associated with it.
	glBindVertexArray(VAO);

	// Tell OpenGL to draw with triangles, using indices, the type of the indices, and the offset to start from
	glDrawElements(GL_TRIANGLES, (GLsizei)indices.size(), GL_UNSIGNED_INT, 0);

	// Unbind the VAO when we're done so we don't accidentally draw extra stuff or tamper with its bound buffers
	glBindVertexArray(0);
}

void OBJObject::update()
{
	//spin(1.0f);
}

void OBJObject::spin(float deg)
{
	// If you haven't figured it out from the last project, this is how you fix spin's behavior
	toWorld = toWorld * glm::rotate(glm::mat4(1.0f), 1.0f / 180.0f * glm::pi<float>(), glm::vec3(0.0f, 1.0f, 0.0f));
}

void OBJObject::translate(float x, float y, float z)
{
	glm::mat4 matrix =
	{ 1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		x, y, z, 1 };

	this->toWorld = matrix * this->toWorld;
}

void OBJObject::origin()
{
	glm::mat4 matrix =
	{ 1, 0, 0, 0,
	0, 1, 0, 0,
	0, 0, 1, 0,
	0, 0, 0, 1 };

	matrix[3][0] = -this->toWorld[3][0];
	matrix[3][1] = -this->toWorld[3][1];
	matrix[3][2] = -this->toWorld[3][2];

	this->toWorld = matrix * this->toWorld;
}

void OBJObject::origin_preserve_z()
{
	glm::mat4 matrix =
	{ 1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1 };

	matrix[3][0] = -this->toWorld[3][0];
	matrix[3][1] = -this->toWorld[3][1];

	this->toWorld = matrix * this->toWorld;
}

void OBJObject::reset()
{
	this->toWorld = glm::mat4(1.0f);
}

void OBJObject::scale(float mult)
{
	glm::mat4 scale_matrix =
	{ mult, 0, 0, 0,
		0, mult, 0, 0,
		0, 0, mult, 0,
		0, 0, 0, 1 };

	float matX = this->toWorld[3][0];
	float matY = this->toWorld[3][1];
	float matZ = this->toWorld[3][2];

	glm::mat4 to_origin_matrix =
	{ 1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		-matX, -matY, -matZ, 1 };

	glm::mat4 reset_matrix =
	{ 1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		matX, matY, matZ, 1 };

	this->toWorld = to_origin_matrix * this->toWorld;
	this->toWorld = scale_matrix * this->toWorld;
	this->toWorld = reset_matrix * this->toWorld;
}

void OBJObject::orbit(float deg)
{

	float radians = deg / 180.0f * glm::pi<float>();
	glm::mat4 matrix =
	{ cos(radians), sin(radians), 0, 0,
		-sin(radians), cos(radians), 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1 };

	this->toWorld = matrix * this->toWorld;
}

void OBJObject::rotate(float angle, glm::vec3 axis)
{
	this->toWorld = glm::rotate(glm::mat4(1.0f), angle / 180.0f * glm::pi<float>(), axis) * this->toWorld;
}

void OBJObject::setAmbient(float r, float g, float b)
{
	object_color = { r, g, b }; 
}

void OBJObject::setDiffuse(float r, float g, float b)
{
	diffuse = { r, g, b };
}

void OBJObject::setSpecular(float r, float g, float b)
{
	specular = { r, g, b };
}

void OBJObject::setShininess(int number)
{
	if (number > 128) {
		shininess = 128;
	}
	else if (number < 0) {
		shininess = 0;
	}
	else {
		shininess = number;
	}
}