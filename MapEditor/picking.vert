#version 330 core

// Positions/Coordinates
layout (location = 0) in vec3 aPos;

// Imports the camera matrix from the main function
uniform mat4 camMatrix;

uniform mat4 modelMatrix;

void main()
{
	// Outputs the positions/coordinates of all vertices
	gl_Position = camMatrix * modelMatrix * vec4(aPos, 1.0);
}