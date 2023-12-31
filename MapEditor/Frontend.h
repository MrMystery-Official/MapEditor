#pragma once

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "imgui_internal.h"
#include "imgui_stdlib.h"
#include <stdio.h>
#include <vector>
#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <linmath.h>
#include "Shader.h"
#include "Camera.h"
#include "VBO.h"
#include "VAO.h"
#include "EBO.h"
#include "Texture.h"
#include <sstream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/vector_angle.hpp>
#include "Bfres.h"
#include "Actor.h"
#include "Exporter.h"
#include "ImGuiPopUp.h"
#include "HashMgr.h"
#include "EditorConfig.h"
#include "MapLoader.h"

namespace Frontend {
	void Initialize(bool LoadedEditorConfig, std::vector<Actor>& LocalActors);
	void CleanUp();
	bool ShouldWindowClose();
	void Render();
};