#include "Frontend.h"
#include "AINBEditor.h"

static void glfw_error_callback(int error, const char* description)
{
	fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

GLFWwindow* Window;
ImVec4 ClearColor = ImVec4(0.11f, 0.15f, 0.18f, 1.00f);

Shader TextureShader;
Shader PickingShader;

GLuint FBO;
GLuint RBO;
GLuint TextureId;

const GLint WIDTH = 800;
const GLint HEIGHT = 600;

Camera camera;

std::vector<Actor>* Actors;

int PickedActorId = -1;

bool FirstFrame = true;

AINBEditor NodeEditor;

struct RenderSettingsStruct
{
	bool Invisible = true;
	bool Area = true;
	bool Visible = true;
	bool FarActors = true;
};

RenderSettingsStruct RenderSettings;

bool LoadedEConfig = false;
bool LoadedDungeon = true;

ImGuiPopUp DynamicPopUp("Add dynamic data", 400, 100, 2);
ImGuiPopUp PhivePlacementPopUp("Add Phive placement", 400, 100, 2);
ImGuiPopUp AddActorPopUp("Add Actor", 400, 124, 3);
ImGuiPopUp GenerateHashesPopUp("Generate Hashes (Phive)", 777, 220, 1);
ImGuiPopUp StackActorsPopUp("Stack actors", 600, 170, 5);
ImGuiPopUp AddLinkPopUp("Add Link", 400, 146, 4);
ImGuiPopUp AddRailPopUp("Add Rail", 400, 123, 3);

ImGuiPopUp SetPathsPopUp("Setup", 600, 181, 2);
ImGuiPopUp LoadMapPopUp("Open level", 468, 118, 1);
ImGuiPopUp ExportModPopUp("Export mod", 468, 78, 1);

namespace Frustum
{
	float m_Frustum[6][4];

	enum FrustumSide
	{
		RIGHT = 0,
		LEFT = 1,
		BOTTOM = 2,
		TOP = 3,
		BACK = 4,
		FRONT = 5
	};

	enum PlaneData
	{
		A = 0,
		B = 1,
		C = 2,
		D = 3
	};

	void NormalizePlane(float frustum[6][4], int side)
	{
		float magnitude = (float)sqrt(frustum[side][A] * frustum[side][A] +
			frustum[side][B] * frustum[side][B] +
			frustum[side][C] * frustum[side][C]);

		// Then we divide the plane's values by it's magnitude.
		// This makes it easier to work with.
		frustum[side][A] /= magnitude;
		frustum[side][B] /= magnitude;
		frustum[side][C] /= magnitude;
		frustum[side][D] /= magnitude;
	}

	void CalculateFrustum()
	{
		const float* proj = glm::value_ptr(camera.GetProjectionMatrix());
		const float* modl = glm::value_ptr(camera.GetViewMatrix());
		float clip[16];

		clip[0] = modl[0] * proj[0] + modl[1] * proj[4] + modl[2] * proj[8] + modl[3] * proj[12];
		clip[1] = modl[0] * proj[1] + modl[1] * proj[5] + modl[2] * proj[9] + modl[3] * proj[13];
		clip[2] = modl[0] * proj[2] + modl[1] * proj[6] + modl[2] * proj[10] + modl[3] * proj[14];
		clip[3] = modl[0] * proj[3] + modl[1] * proj[7] + modl[2] * proj[11] + modl[3] * proj[15];

		clip[4] = modl[4] * proj[0] + modl[5] * proj[4] + modl[6] * proj[8] + modl[7] * proj[12];
		clip[5] = modl[4] * proj[1] + modl[5] * proj[5] + modl[6] * proj[9] + modl[7] * proj[13];
		clip[6] = modl[4] * proj[2] + modl[5] * proj[6] + modl[6] * proj[10] + modl[7] * proj[14];
		clip[7] = modl[4] * proj[3] + modl[5] * proj[7] + modl[6] * proj[11] + modl[7] * proj[15];

		clip[8] = modl[8] * proj[0] + modl[9] * proj[4] + modl[10] * proj[8] + modl[11] * proj[12];
		clip[9] = modl[8] * proj[1] + modl[9] * proj[5] + modl[10] * proj[9] + modl[11] * proj[13];
		clip[10] = modl[8] * proj[2] + modl[9] * proj[6] + modl[10] * proj[10] + modl[11] * proj[14];
		clip[11] = modl[8] * proj[3] + modl[9] * proj[7] + modl[10] * proj[11] + modl[11] * proj[15];

		clip[12] = modl[12] * proj[0] + modl[13] * proj[4] + modl[14] * proj[8] + modl[15] * proj[12];
		clip[13] = modl[12] * proj[1] + modl[13] * proj[5] + modl[14] * proj[9] + modl[15] * proj[13];
		clip[14] = modl[12] * proj[2] + modl[13] * proj[6] + modl[14] * proj[10] + modl[15] * proj[14];
		clip[15] = modl[12] * proj[3] + modl[13] * proj[7] + modl[14] * proj[11] + modl[15] * proj[15];

		m_Frustum[RIGHT][A] = clip[3] - clip[0];
		m_Frustum[RIGHT][B] = clip[7] - clip[4];
		m_Frustum[RIGHT][C] = clip[11] - clip[8];
		m_Frustum[RIGHT][D] = clip[15] - clip[12];
		NormalizePlane(m_Frustum, RIGHT);

		m_Frustum[LEFT][A] = clip[3] + clip[0];
		m_Frustum[LEFT][B] = clip[7] + clip[4];
		m_Frustum[LEFT][C] = clip[11] + clip[8];
		m_Frustum[LEFT][D] = clip[15] + clip[12];
		NormalizePlane(m_Frustum, LEFT);

		m_Frustum[BOTTOM][A] = clip[3] + clip[1];
		m_Frustum[BOTTOM][B] = clip[7] + clip[5];
		m_Frustum[BOTTOM][C] = clip[11] + clip[9];
		m_Frustum[BOTTOM][D] = clip[15] + clip[13];
		NormalizePlane(m_Frustum, BOTTOM);

		m_Frustum[TOP][A] = clip[3] - clip[1];
		m_Frustum[TOP][B] = clip[7] - clip[5];
		m_Frustum[TOP][C] = clip[11] - clip[9];
		m_Frustum[TOP][D] = clip[15] - clip[13];
		NormalizePlane(m_Frustum, TOP);

		m_Frustum[BACK][A] = clip[3] - clip[2];
		m_Frustum[BACK][B] = clip[7] - clip[6];
		m_Frustum[BACK][C] = clip[11] - clip[10];
		m_Frustum[BACK][D] = clip[15] - clip[14];
		NormalizePlane(m_Frustum, BACK);

		m_Frustum[FRONT][A] = clip[3] + clip[2];
		m_Frustum[FRONT][B] = clip[7] + clip[6];
		m_Frustum[FRONT][C] = clip[11] + clip[10];
		m_Frustum[FRONT][D] = clip[15] + clip[14];
		NormalizePlane(m_Frustum, FRONT);
	}

	bool SphereInFrustum(float x, float y, float z, float radius)
	{
		for (int i = 0; i < 6; i++)
		{
			if (m_Frustum[i][A] * x + m_Frustum[i][B] * y + m_Frustum[i][C] * z + m_Frustum[i][D] <= -radius)
			{
				return false;
			}
		}

		return true;
	}
};

float Max(float a, float b) {
	return a > b ? a : b;
}

void Frontend::Initialize(bool LoadedEditorConfig, std::vector<Actor>& LocalActors) {
	Actors = &LocalActors;
	LoadedEConfig = LoadedEditorConfig;

	glfwSetErrorCallback(glfw_error_callback);
	if (!glfwInit())
		return;

	Window = glfwCreateWindow(1280, 720, "Map Editor - The Legend of Zelda: Tears of the Kingdom", nullptr, nullptr);
	if (Window == nullptr)
		return;
	glfwMakeContextCurrent(Window);

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

	gladLoadGL();

	glfwSwapInterval(1); // Enable vsync

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	io.ConfigWindowsMoveFromTitleBarOnly = true;

	ImGui::StyleColorsDark();

	ImGuiStyle& style = ImGui::GetStyle();

	style.Colors[ImGuiCol_TitleBgActive] = style.Colors[ImGuiCol_TitleBg];

	const char* glsl_version = "#version 130";

	ImGui_ImplGlfw_InitForOpenGL(Window, true);
	ImGui_ImplOpenGL3_Init(glsl_version);

	ImGuiPlatformIO& platform_io = ImGui::GetPlatformIO();
	for (ImGuiPlatformMonitor monitor : platform_io.Monitors) {
		ImFont* font = io.Fonts->AddFontFromFileTTF("FiraSans-Regular.ttf", floor(14 * (monitor.DpiScale * 1.2f)));
		Config::Fonts->emplace(monitor.DpiScale, font);
	}

	platform_io.Platform_OnChangedViewport = OnViewportChanged;

	TextureShader = Shader("default.vert", "default.frag");
	PickingShader = Shader("picking.vert", "picking.frag");

	glGenFramebuffers(1, &FBO);
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);

	glGenTextures(1, &TextureId);
	glBindTexture(GL_TEXTURE_2D, TextureId);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, WIDTH, HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, TextureId, 0);

	glGenRenderbuffers(1, &RBO);
	glBindRenderbuffer(GL_RENDERBUFFER, RBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, WIDTH, HEIGHT);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, RBO);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "ERROR::FRAMEBUFFER: Framebuffer is not complete!\n";

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	camera = Camera(WIDTH, HEIGHT, glm::vec3(0.0f, 0.0f, 2.0f));

	NodeEditor.Initialize();
}

void Frontend::OnViewportChanged(ImGuiViewport* vp) {
	if (Config::UIScale != vp->DpiScale * 1.2f) {
		ImGuiStyle& style = ImGui::GetStyle();
		Config::UIScale = vp->DpiScale * 1.2f;
		style.ScaleAllSizes(Config::UIScale);
		ImGui::SetCurrentFont(Config::Fonts->at(vp->DpiScale));

		DynamicPopUp.UpdateScale(Config::UIScale);
		PhivePlacementPopUp.UpdateScale(Config::UIScale);
		AddActorPopUp.UpdateScale(Config::UIScale);
		GenerateHashesPopUp.UpdateScale(Config::UIScale);
		StackActorsPopUp.UpdateScale(Config::UIScale);
		AddLinkPopUp.UpdateScale(Config::UIScale);
		AddRailPopUp.UpdateScale(Config::UIScale);
		SetPathsPopUp.UpdateScale(Config::UIScale);
		LoadMapPopUp.UpdateScale(Config::UIScale);
		ExportModPopUp.UpdateScale(Config::UIScale);
	}
}

void Frontend::CleanUp() {
	for (auto& [Gyml, Model] : ActorModelLibrary::GetModels())
	{
		Model.Delete();
	}
	for (auto& [TexToGo, Tex] : GLTextureLibrary::GetTextures())
	{
		Tex.Delete();
	}
	TextureShader.Delete();
	PickingShader.Delete();
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	NodeEditor.Destroy();
	ImGui::DestroyContext();

	glfwDestroyWindow(Window);
	glfwTerminate();
}

bool Frontend::ShouldWindowClose() {
	return glfwWindowShouldClose(Window);
}

bool IsObjectVisible(const glm::mat4& mvp, std::vector<glm::vec3>& BBVertices) {
	std::vector<glm::vec4> transformedVertices;

	for (const auto& vertex : BBVertices) {
		glm::vec4 transformedVertex = mvp * glm::vec4(vertex, 1.0f);
		transformedVertices.push_back(transformedVertex);
	}

	for (const auto& vertex : transformedVertices) {
		glm::vec3 normalizedDeviceCoords = glm::vec3(vertex) / vertex.w;

		if (normalizedDeviceCoords.x >= -1.0f && normalizedDeviceCoords.x <= 1.0f &&
			normalizedDeviceCoords.y >= -1.0f && normalizedDeviceCoords.y <= 1.0f &&
			normalizedDeviceCoords.z >= -1.0f && normalizedDeviceCoords.z <= 1.0f) {
			return true;
		}
	}

	return false;
}

//WARNING: EXTREMLY HACKY AND ULTRA-MEGA-OVER SLOOOOOOOOOW, REWRITE!
//Bug: Clicking is not pixel-perfect
void CheckActorSelection(ImVec2 SceneWindowSize, ImVec2 MousePos)
{

	if (glfwGetMouseButton(Window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS && glfwGetMouseButton(Window, GLFW_MOUSE_BUTTON_RIGHT) != GLFW_PRESS)
	{
		if (!(SceneWindowSize.x > MousePos.x && SceneWindowSize.y > MousePos.y && MousePos.x > 0 && MousePos.y > 0)) return;

		glBindFramebuffer(GL_FRAMEBUFFER, FBO);

		glClearColor(ClearColor.x * ClearColor.w, ClearColor.y * ClearColor.w, ClearColor.z * ClearColor.w, ClearColor.w);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		PickingShader.Activate();

		camera.Matrix(45.0f, 0.1f, 100.0f, PickingShader, "camMatrix");

		for (int ActorIndex = 0; ActorIndex < Actors->size(); ActorIndex++)
		{
			Actor& Actor = Actors->at(ActorIndex);
			if (Actor.GetModel()->IsDefaultModel() && !RenderSettings.Invisible) continue;
			if (!Actor.GetModel()->IsDefaultModel() && !RenderSettings.Visible) continue;
			if (Actor.GetGyml().ends_with("_Far") && !RenderSettings.FarActors) continue;
			if (Actor.GetGyml().find("Area") != std::string::npos && !RenderSettings.Area) continue;

			if (Actor.GetModel()->GetModels().size() == 0) continue;
			if (Actor.GetModel()->GetModels()[0].LODs.size() == 0) continue;

			int R = (ActorIndex & 0x000000FF) >> 0;
			int G = (ActorIndex & 0x0000FF00) >> 8;
			int B = (ActorIndex & 0x00FF0000) >> 16;

			glm::mat4 model = glm::mat4(1.0f);  // Identity matrix

			model = glm::translate(model, glm::vec3(Actor.GetTranslate().GetX(), Actor.GetTranslate().GetY(), Actor.GetTranslate().GetZ()));

			model = glm::rotate(model, glm::radians(Actor.GetRotate().GetZ()), glm::vec3(0.0, 0.0f, 1.0));
			model = glm::rotate(model, glm::radians(Actor.GetRotate().GetY()), glm::vec3(0.0f, 1.0, 0.0));
			model = glm::rotate(model, glm::radians(Actor.GetRotate().GetX()), glm::vec3(1.0, 0.0f, 0.0));

			model = glm::scale(model, glm::vec3(Actor.GetScale().GetX(), Actor.GetScale().GetY(), Actor.GetScale().GetZ()));

			float DistanceToCamera = fabs(pow(Actor.GetTranslate().GetX() - camera.Position.x, 2) + pow(Actor.GetTranslate().GetY() - camera.Position.y, 2) + pow(Actor.GetTranslate().GetZ() - camera.Position.z, 2));

			BfresFile::LOD* LODModel = &Actor.GetModel()->GetModels()[0].LODs[0];

			if (DistanceToCamera >= 10000) //Max distance is 100, 10000 = 100 to the power of 2
			{
				LODModel = &Actor.GetModel()->GetModels()[0].LODs[Actor.GetModel()->GetModels()[0].LODs.size() - 1];
			}

			glUniformMatrix4fv(glGetUniformLocation(PickingShader.ID, "modelMatrix"), 1, GL_FALSE, glm::value_ptr(model));
			glUniform4f(glGetUniformLocation(PickingShader.ID, "PickingColor"), R / 255.0f, G / 255.0f, B / 255.0f, 1.0f);

			for (int SubModelIndex = 0; SubModelIndex < LODModel->GL_Textures.size(); SubModelIndex++)
			{
				LODModel->GL_VAO[SubModelIndex].Bind();
				glDrawElements(GL_TRIANGLES, sizeof(int) * LODModel->Faces[SubModelIndex].size(), GL_UNSIGNED_INT, 0);
			}
		}

		glReadBuffer(GL_COLOR_ATTACHMENT0);
		unsigned char Data[3];

		glReadPixels((GLint)MousePos.x, (GLint)SceneWindowSize.y - MousePos.y, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, Data);

		PickedActorId =
			Data[0] +
			Data[1] * 256 +
			Data[2] * 256 * 256;

		if (PickedActorId > 10000 || PickedActorId < 0)
		{
			PickedActorId = -1;
		}

		glReadBuffer(GL_NONE);
	}
}

void Frontend::Render() {
	glfwPollEvents();

	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();

	glClearColor(ClearColor.x * ClearColor.w, ClearColor.y * ClearColor.w, ClearColor.z * ClearColor.w, ClearColor.w);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	ImGui::NewFrame();

	int width, height;
	glfwGetFramebufferSize(Window, &width, &height);

	ImGuiID DockSpace = ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());

	//ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove

	ImGui::Begin("Map View");

	const float window_width = ImGui::GetContentRegionAvail().x;
	const float window_height = ImGui::GetContentRegionAvail().y;

	glBindTexture(GL_TEXTURE_2D, TextureId);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, window_width, window_height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, TextureId, 0);

	glBindRenderbuffer(GL_RENDERBUFFER, RBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, window_width, window_height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, RBO);

	glViewport(0, 0, window_width, window_height);

	camera.width = window_width;
	camera.height = window_height;

	ImVec2 pos = ImGui::GetCursorScreenPos();
	camera.windowHovered = (ImGui::IsWindowHovered() && !ImGui::IsItemHovered());

	ImGui::GetWindowDrawList()->AddImage(
		(void*)TextureId,
		ImVec2(pos.x, pos.y),
		ImVec2(pos.x + window_width, pos.y + window_height),
		ImVec2(0, 1),
		ImVec2(1, 0)
	);

	ImGui::Text("FPS: %.1f (%.3f ms/frame)", ImGui::GetIO().Framerate, 1000.0 / ImGui::GetIO().Framerate);

	if (!DynamicPopUp.IsOpen() &&
		!PhivePlacementPopUp.IsOpen()
		&& !AddActorPopUp.IsOpen()
		&& !GenerateHashesPopUp.IsOpen()
		&& !StackActorsPopUp.IsOpen()
		&& !ExportModPopUp.IsOpen()
		&& !AddLinkPopUp.IsOpen()
		&& !AddRailPopUp.IsOpen()
		&& ImGui::IsWindowFocused())
	{
		CheckActorSelection(ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y), ImVec2(ImGui::GetMousePos().x - ImGui::GetWindowPos().x - ImGui::GetWindowContentRegionMin().x,
			ImGui::GetMousePos().y - ImGui::GetWindowPos().y - ImGui::GetWindowContentRegionMin().y));
	}

	ImGui::End();

	//Left window with options and actions
	ImGui::Begin("Actions");

	if (ImGui::Button("Load map"))
	{
		LoadedDungeon = false;
	}

	if (ImGui::Button("Add Actor"))
	{
		AddActorPopUp.IsOpen() = true;
		HashMgr::ArtificialHash NewHash = HashMgr::GetArtificialHash(false);
		AddActorPopUp.GetData()[1] = std::to_string(NewHash.ActorHash);
		AddActorPopUp.GetData()[2] = std::to_string(NewHash.SRTHash);
	}
	if (AddActorPopUp.IsCompleted())
	{
		Actor NewActor;
		NewActor.SetGyml(AddActorPopUp.GetData()[0]);
		NewActor.SetHash(Util::StringToNumber<uint64_t>(AddActorPopUp.GetData()[1]));
		NewActor.SetSRTHash(Util::StringToNumber<uint32_t>(AddActorPopUp.GetData()[2]));
		NewActor.SetType(NewActor.GetGyml().rfind("MapEditor_Collision_", 0) == 0 ? Actor::Type::Static : Actor::Type::Dynamic);

		if (NewActor.GetGyml().rfind("MapEditor_Collision_", 0) == 0)
		{
			std::string NewGymlId = Config::Key + AddActorPopUp.GetData()[2].substr(3, AddActorPopUp.GetData()[2].length() - 3); //Remove first 3 chars
			if (NewGymlId[0] == '0') NewGymlId[0] = '2';
			if (NewGymlId[1] == '0') NewGymlId[1] = '2';
			if (NewGymlId[2] == '0') NewGymlId[2] = '2';

			if (NewGymlId[1] == '-') NewGymlId[1] = '0';

			if (NewGymlId[0] == 'A') NewGymlId[0] = '3';
			if (NewGymlId[0] == 'B') NewGymlId[0] = '4';
			if (NewGymlId[0] == 'C') NewGymlId[0] = '5';
			if (NewGymlId[0] == 'D') NewGymlId[0] = '6';
			if (NewGymlId[0] == 'E') NewGymlId[0] = '7';
			if (NewGymlId[0] == 'F') NewGymlId[0] = '8';
			if (NewGymlId[0] == 'G') NewGymlId[0] = '9';
			if (NewGymlId[0] == 'H')
			{
				NewGymlId[0] = '3';
				NewGymlId[1] = '1';
			}
			if (NewGymlId[0] == 'I')
			{
				NewGymlId[0] = '4';
				NewGymlId[1] = '1';
			}
			if (NewGymlId[0] == 'J')
			{
				NewGymlId[0] = '5';
				NewGymlId[1] = '1';
			}
			NewActor.SetGyml(AddActorPopUp.GetData()[0] + "_" + NewGymlId);
			NewActor.SetModel(ActorModelLibrary::GetModel("Collision"));
			goto FinishedBfresLoading;
		}

		{
			SarcFile ActorPackFile(ZStdFile::Decompress(Config::GetRomFSFile("Pack/Actor/" + NewActor.GetGyml() + ".pack.zs"), ZStdFile::Dictionary::Pack).Data);
			std::string ModelInfoEntryName;
			for (SarcFile::Entry Entry : ActorPackFile.GetEntries())
			{
				if (Entry.Name.rfind("Component/ModelInfo/", 0) == 0)
				{
					ModelInfoEntryName = Entry.Name;
					break;
				}
			}
			if (ModelInfoEntryName.rfind("Component/ModelInfo/None", 0) == 0 || ModelInfoEntryName == "")
			{
				NewActor.SetModel(ActorModelLibrary::GetModel("None"));
				std::cout << "Warning: Could not find a model for actor " << NewActor.GetGyml() << "!\n";
			}
			else
			{
				BymlFile ModelInfo(ActorPackFile.GetEntry(ModelInfoEntryName).Bytes);
				if (ModelInfo.GetNodes().size() == 0)
				{
					NewActor.SetModel(ActorModelLibrary::GetModel("None"));
					goto FinishedBfresLoading;
				}
				NewActor.SetModel(ActorModelLibrary::GetModel(ModelInfo.GetNode("ModelProjectName")->GetValue<std::string>() + "." + ModelInfo.GetNode("FmdbName")->GetValue<std::string>()));
			}

		}

	FinishedBfresLoading:

		int ActorIndex = Actors->size();
		Actors->push_back(NewActor);
		PickedActorId = Actors->size() - 1;

		AddActorPopUp.Reset();
	}

	if (ImGui::Button("Save"))
	{
		Exporter::Export(Actors, Config::GetWorkingDirFile("Save"), true);
	}

	if (ImGui::Button("Export"))
	{
		ExportModPopUp.GetData()[0] = Config::ExportPath;
		ExportModPopUp.IsOpen() = true;
	}
	if (ExportModPopUp.IsCompleted())
	{
		Config::ExportPath = ExportModPopUp.GetData()[0];
		Exporter::Export(Actors, ExportModPopUp.GetData()[0], false);
		ExportModPopUp.Reset();
	}

	ImGui::NewLine();
	if (ImGui::Button("Generate Hashes (Phive)"))
	{
		GenerateHashesPopUp.IsOpen() = true;
	}
	ImGui::Text(std::string("Available Hashes: " + std::to_string(HashMgr::GetHashes().size() - (HashMgr::GetIndex() + 1))).c_str());
	if (GenerateHashesPopUp.IsCompleted())
	{
		if (GenerateHashesPopUp.IntData == 0)
		{
			Actor* MapModelActor = nullptr;
			for (Actor& Actor : *Actors)
			{
				if (Actor.GetSRTHash() == Util::StringToNumber<uint32_t>(GenerateHashesPopUp.GetData()[0]))
				{
					MapModelActor = &Actor;
					break;
				}
			}

			if (MapModelActor == nullptr)
			{
				std::cerr << "ERROR: Could not find actor with SRTHash " << GenerateHashesPopUp.GetData()[0] << "!\n";
			}
			else
			{
				if (MapModelActor->GetDynamic().DynamicString.count("BancPath"))
				{
					BymlFile MergedActorByml(ZStdFile::Decompress(Config::GetRomFSFile(MapModelActor->GetDynamic().DynamicString["BancPath"] + ".zs"), ZStdFile::Dictionary::BcettByaml).Data);
					for (BymlFile::Node& ActorNode : MergedActorByml.GetNode("Actors")->GetChildren())
					{
						uint64_t Hash = ActorNode.GetChild("Hash")->GetValue<uint64_t>();
						uint32_t SRTHash = ActorNode.GetChild("SRTHash")->GetValue<uint32_t>();
						uint64_t PhiveHash = 0;
						if (ActorNode.HasChild("Phive"))
						{
							BymlFile::Node* PhiveNode = ActorNode.GetChild("Phive");
							if (PhiveNode->HasChild("Placement"))
							{
								for (BymlFile::Node& PlacementNode : PhiveNode->GetChild("Placement")->GetChildren())
								{
									if (PlacementNode.GetKey() == "ID")
									{
										PhiveHash = PlacementNode.GetValue<uint64_t>();
										break;
									}
								}
							}
						}

						if (PhiveHash != 0)
						{
							if (Hash > HashMgr::CurrentHash) HashMgr::CurrentHash = Hash;
							if (SRTHash > HashMgr::CurrentSRTHash) HashMgr::CurrentSRTHash = SRTHash;

							HashMgr::AddHash({ Hash, PhiveHash, SRTHash });

							HashMgr::ArtificialHash NewHash = HashMgr::GetArtificialHash(false);

							ActorNode.GetChild("Hash")->SetValue<uint64_t>(NewHash.ActorHash);
							ActorNode.GetChild("SRTHash")->SetValue<uint32_t>(NewHash.SRTHash);
							ActorNode.GetChild("Phive")->GetChild("Placement")->GetChild("ID")->SetValue<uint64_t>(NewHash.PhiveHash);
						}
					}
					HashMgr::GetIndex() = -1;
					ZStdFile::Compress(MergedActorByml.ToBinary(BymlFile::TableGeneration::Auto), ZStdFile::Dictionary::BcettByaml).WriteToFile(Config::GetWorkingDirFile("Save/" + MapModelActor->GetDynamic().DynamicString["BancPath"] + ".zs"));
					Exporter::Export(Actors, Config::GetWorkingDirFile("Save"), true);
				}
				else
				{
					std::cerr << "ERROR: The input actor was not a MergedActor!\n";
				}
			}
		}
		else
		{
			std::map<uint32_t, uint32_t> SRTHashes;
			std::map<uint64_t, uint64_t> Hashes;
			std::map<uint64_t, uint64_t> PhiveHashes;

			for (Actor& PhysicsActor : *Actors)
			{
				if (PhysicsActor.GetCategory() == "MapStatic" || PhysicsActor.GetCategory() == "System")
				{
					uint64_t Hash = PhysicsActor.GetHash();
					uint32_t SRTHash = PhysicsActor.GetSRTHash();
					uint64_t PhiveHash = 0;
					if (PhysicsActor.GetPhive().Placement.count("ID"))
					{
						PhiveHash = Util::StringToNumber<uint64_t>(PhysicsActor.GetPhive().Placement["ID"]);
					}

					if (PhiveHash != 0)
					{
						if (Hash > HashMgr::CurrentHash) HashMgr::CurrentHash = Hash;
						if (SRTHash > HashMgr::CurrentSRTHash) HashMgr::CurrentSRTHash = SRTHash;

						HashMgr::AddHash({ Hash, PhiveHash, SRTHash });

						HashMgr::ArtificialHash NewHash = HashMgr::GetArtificialHash(false);
						if (!SRTHashes.count(SRTHash) && !Hashes.count(Hash) && !PhiveHashes.count(PhiveHash))
						{
							SRTHashes.insert({ SRTHash, NewHash.SRTHash });
							Hashes.insert({ Hash, NewHash.ActorHash });
							PhiveHashes.insert({ PhiveHash, NewHash.PhiveHash });
						}
					}
				}
			}
			HashMgr::GetIndex() = -1;
			Exporter::Export(Actors, Config::GetWorkingDirFile("Save"), true);
			for (BymlFile::Node& Child : Config::StaticActorsByml.GetNodes())
			{
				HashMgr::TransformHashesInNode(Child, &Hashes, &SRTHashes, &PhiveHashes);
			}
			for (BymlFile::Node& Child : Config::DynamicActorsByml.GetNodes())
			{
				HashMgr::TransformHashesInNode(Child, &Hashes, &SRTHashes, &PhiveHashes);
			}
			ZStdFile::Compress(Config::StaticActorsByml.ToBinary(BymlFile::TableGeneration::Auto), ZStdFile::Dictionary::BcettByaml).WriteToFile(Config::GetWorkingDirFile("Save/" + Config::BancPrefix + Config::Key + "_Static.bcett.byml.zs"));
			ZStdFile::Compress(Config::DynamicActorsByml.ToBinary(BymlFile::TableGeneration::Auto), ZStdFile::Dictionary::BcettByaml).WriteToFile(Config::GetWorkingDirFile("Save/" + Config::BancPrefix + Config::Key + "_Dynamic.bcett.byml.zs"));
			glfwSetWindowShouldClose(Window, 1);
		}
		GenerateHashesPopUp.Reset();
	}

	if (ImGui::Button("Stack actors"))
	{
		if (PickedActorId != -1)
			StackActorsPopUp.GetData()[0] = std::to_string(Actors->at(PickedActorId).GetSRTHash());

		StackActorsPopUp.IsOpen() = true;
	}
	if (StackActorsPopUp.IsCompleted())
	{
		uint32_t SRTHash = Util::StringToNumber<uint32_t>(StackActorsPopUp.GetData()[0]);
		uint32_t Amount = Util::StringToNumber<uint32_t>(StackActorsPopUp.GetData()[1]);
		float OffsetX = std::stof(StackActorsPopUp.GetData()[2]);
		float OffsetY = std::stof(StackActorsPopUp.GetData()[3]);
		float OffsetZ = std::stof(StackActorsPopUp.GetData()[4]);

		Actor BaseActor;

		for (Actor& Actor : *Actors)
		{
			if (Actor.GetSRTHash() == SRTHash)
			{
				BaseActor = Actor;
				break;
			}
		}

		if (BaseActor.GetHash() != 0)
		{

			bool Physics = !BaseActor.GetPhive().Placement.empty();

			Actors->resize(Actors->size() + Amount);

			for (int i = 0; i < Amount; i++)
			{
				Actor NewActor = BaseActor;
				NewActor.GetTranslate().SetX(NewActor.GetTranslate().GetX() + (OffsetX * (i + 1)));
				NewActor.GetTranslate().SetY(NewActor.GetTranslate().GetY() + (OffsetY * (i + 1)));
				NewActor.GetTranslate().SetZ(NewActor.GetTranslate().GetZ() + (OffsetZ * (i + 1)));

				HashMgr::ArtificialHash NewHash = HashMgr::GetArtificialHash(Physics);

				NewActor.SetHash(NewHash.ActorHash);
				NewActor.SetSRTHash(NewHash.SRTHash);
				if (Physics)
				{
					std::map<std::string, std::string>::iterator Iter = NewActor.GetPhive().Placement.find("ID");
					if (Iter != NewActor.GetPhive().Placement.end())
						Iter->second = std::to_string(NewHash.PhiveHash);
				}

				(*Actors)[Actors->size() - Amount + i] = NewActor;
			}
		}
		else
		{
			std::cerr << "ERROR: Can't find actor with SRTHash " << StackActorsPopUp.GetData()[0] << "!\n";
		}

		StackActorsPopUp.Reset();
	}

	if (Config::MapType == 0) //0 = SmallDungeon
	{
		if (ImGui::Button("StartPos to WarpIn Actor"))
		{
			BymlFile StartPosByml(ZStdFile::Decompress(Config::GetWorkingDirFile("Save/Banc/SmallDungeon/StartPos/SmallDungeon.startpos.byml.zs"), ZStdFile::Dictionary::Base).Data);

			Actor* WarpInActor = nullptr;
			for (Actor& Actor : *Actors)
			{
				if (Actor.GetGyml().rfind("DgnObj_Small_Warpin_B_", 0) == 0)
				{
					WarpInActor = &Actor;
					break;
				}
			}
			StartPosByml.GetNode("OnElevator")->GetChild("Dungeon" + Config::Key)->GetChild("Trans")->GetChild(0)->SetValue<float>(WarpInActor->GetTranslate().GetX());
			StartPosByml.GetNode("OnElevator")->GetChild("Dungeon" + Config::Key)->GetChild("Trans")->GetChild(1)->SetValue<float>(WarpInActor->GetTranslate().GetY());
			StartPosByml.GetNode("OnElevator")->GetChild("Dungeon" + Config::Key)->GetChild("Trans")->GetChild(2)->SetValue<float>(WarpInActor->GetTranslate().GetZ());
			ZStdFile::Compress(StartPosByml.ToBinary(BymlFile::TableGeneration::Auto), ZStdFile::Dictionary::Base).WriteToFile(Config::GetWorkingDirFile("Save/Banc/SmallDungeon/StartPos/SmallDungeon.startpos.byml.zs"));
		}
	}

	ImGui::NewLine();
	ImGui::Text("Render Settings");
	ImGui::Checkbox("Visible actors", &RenderSettings.Visible);
	ImGui::Checkbox("Invisible actors", &RenderSettings.Invisible);
	ImGui::Checkbox("Areas", &RenderSettings.Area);
	ImGui::Checkbox("Far actors", &RenderSettings.FarActors);

	ImGui::End();

	//Actor properties window
	ImGui::Begin("Properties");

	if (PickedActorId != -1)
	{
		if (ImGui::Button("Delete"))
		{
			Actors->erase(Actors->begin() + PickedActorId);

			PickedActorId = -1;

			goto PopupRendering;
		}

		if (ImGui::Button("Duplicate"))
		{
			Actor NewActor = Actors->at(PickedActorId);

			bool Physics = !NewActor.GetPhive().Placement.empty();

			HashMgr::ArtificialHash NewHash = HashMgr::GetArtificialHash(Physics);

			NewActor.SetHash(NewHash.ActorHash);
			NewActor.SetSRTHash(NewHash.SRTHash);
			if (Physics)
			{
				std::map<std::string, std::string>::iterator Iter = NewActor.GetPhive().Placement.find("ID");
				if (Iter != NewActor.GetPhive().Placement.end())
					Iter->second = std::to_string(NewHash.PhiveHash);
			}

			Actors->resize(Actors->size() + 1);

			(*Actors)[Actors->size() - 1] = NewActor;

			PickedActorId = Actors->size() - 1;
		}

		Actor& SelectedActor = Actors->at(PickedActorId);

		ImGui::NewLine();
		ImGui::Text("Debug");
		ImGui::Text(SelectedActor.GetCategory().c_str());
		ImGui::NewLine();
		/*Actor identification*/
		ImGui::Text("Identification");
		ImGui::InputText("Gyaml", &SelectedActor.GetGyml());
		ImGui::InputScalar("Hash", ImGuiDataType_::ImGuiDataType_U64, &SelectedActor.GetHash());
		ImGui::InputScalar("SRTHash", ImGuiDataType_::ImGuiDataType_U32, &SelectedActor.GetSRTHash());
		ImGui::InputText("Name", &SelectedActor.GetName());
		const char* TypeDropdownItems[] = { "Static", "Dynamic" };
		ImGui::Combo("Type", reinterpret_cast<int*>(&SelectedActor.GetType()), TypeDropdownItems, IM_ARRAYSIZE(TypeDropdownItems));
		if (HashMgr::GetHashes().size() > 0)
		{
			if (ImGui::Button("Enable physics"))
			{
				HashMgr::ArtificialHash NewHash = HashMgr::GetArtificialHash(true);
				SelectedActor.SetHash(NewHash.ActorHash);
				SelectedActor.SetSRTHash(NewHash.SRTHash);
				std::map<std::string, std::string>::iterator Iter = SelectedActor.GetPhive().Placement.find("ID");
				if (Iter != SelectedActor.GetPhive().Placement.end())
				{
					Iter->second = std::to_string(NewHash.PhiveHash);
				}
				else
				{
					SelectedActor.GetPhive().Placement.insert({ "ID", std::to_string(NewHash.PhiveHash) });
				}
			}
		}
		if (SelectedActor.GetGyml().rfind("MapEditor_Collision_", 0) != 0)
		{
			if (ImGui::Button("Add collision"))
			{
				using recursive_directory_iterator = std::filesystem::recursive_directory_iterator;
				bool Found = false;
				for (const auto& DirEntry : recursive_directory_iterator(Config::GetRomFSFile("Phive/Shape/Dcc")))
				{
					std::string CollisionFileName = DirEntry.path().string();
					Util::ReplaceString(CollisionFileName, Config::GetRomFSFile("Phive/Shape/Dcc") + "\\", "");
					if (CollisionFileName.rfind(SelectedActor.GetGyml() + "__Physical", 0) == 0)
					{
						Found = true;
						Util::ReplaceString(CollisionFileName, "Nin_NX_NVN.bphsh.zs", "phsh");
						std::cout << "Collision File: " << CollisionFileName << std::endl;

						Actor NewActor;

						HashMgr::ArtificialHash NewHash = HashMgr::GetArtificialHash(false);

						NewActor.SetGyml("MapEditor_Collision_File");
						NewActor.SetHash(NewHash.ActorHash);
						NewActor.SetSRTHash(NewHash.SRTHash);

						NewActor.SetTranslate(SelectedActor.GetTranslate());
						NewActor.SetRotate(SelectedActor.GetRotate());
						NewActor.SetScale(SelectedActor.GetScale());

						NewActor.SetCollisionFile(CollisionFileName);

						NewActor.SetModel(ActorModelLibrary::GetModel("Collision"));

						NewActor.SetType(Actor::Type::Static);

						Actors->resize(Actors->size() + 1);
						(*Actors)[Actors->size() - 1] = NewActor;

						PickedActorId = Actors->size() - 1;
					}
				}
				if (!Found)
				{
					std::cerr << "ERROR: Editor could not find a valid Binary Phive Shape File!\n";
				}
			}
		}

		/*
		std::map<std::string, std::string> m_Presence; //Not required
		std::vector<Rail> m_Rails; //Not required
		std::vector<Link> m_Links; //Not rquired
		*/

		ImGui::NewLine();
		ImGui::Text("Transform");
		ImGui::InputFloat3("Translate", SelectedActor.GetTranslate().GetRawData());
		ImGui::InputFloat3("Rotate", SelectedActor.GetRotate().GetRawData());
		ImGui::InputFloat3("Scale", SelectedActor.GetScale().GetRawData());
		ImGui::Checkbox("Bakeable", &SelectedActor.IsBakeable());
		ImGui::Checkbox("Physics Stable", &SelectedActor.IsPhysicsStable());
		ImGui::Checkbox("Force Active", &SelectedActor.IsForceActive());
		ImGui::Checkbox("In Water", &SelectedActor.IsInWater());
		ImGui::Checkbox("Turn Actor Near Enemy", &SelectedActor.IsTurnActorNearEnemy());

		ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x * 0.5f);
		ImGui::InputFloat("Move Radius", &SelectedActor.GetMoveRadius());
		ImGui::InputFloat("Extra Create Radius", &SelectedActor.GetExtraCreateRadius());
		//ImGui::PopItemWidth();

		ImGui::NewLine();
		ImGui::Text("Dynamic");
		ImGui::SameLine();
		int Identifier = 0;

		if (ImGui::Button("Add"))
		{
			DynamicPopUp.IsOpen() = true;
		}
		if (DynamicPopUp.IsCompleted())
		{
			SelectedActor.AddDynamic(DynamicPopUp.GetData()[0], DynamicPopUp.GetData()[1]);
			DynamicPopUp.Reset();
		}

		for (auto Iter = SelectedActor.GetDynamic().DynamicString.begin(); Iter != SelectedActor.GetDynamic().DynamicString.end(); ) {
			auto& [Key, Value] = *Iter;

			ImGui::InputText(Key.c_str(), &Value);
			ImGui::Text(" ");
			ImGui::SameLine();

			if (ImGui::Button(std::string("Del##" + std::to_string(Identifier)).c_str()))
			{
				Iter = SelectedActor.GetDynamic().DynamicString.erase(Iter);
			}
			else
			{
				Iter++;
			}
			Identifier++;
		}

		ImGui::PopItemWidth();

		ImGui::NewLine();
		ImGui::Text("Phive");
		if (SelectedActor.GetGyml().rfind("MapEditor_Collision_Cube_", 0) == 0)
		{
			ImGui::Checkbox("Climbable", &SelectedActor.IsCollisionClimbable());
		}
		if (SelectedActor.GetGyml().rfind("MapEditor_Collision_File", 0) == 0)
		{
			ImGui::InputText("Phive Shape", &SelectedActor.GetCollisionFile());
		}
		if (SelectedActor.GetGyml().rfind("MapEditor_Collision_Custom", 0) == 0)
		{
			ImGui::InputScalar("Collision Actor Link", ImGuiDataType_::ImGuiDataType_U32, &SelectedActor.GetCollisionSRTHash());
		}
		ImGui::Text("Phive - Placement");
		ImGui::SameLine();
		if (ImGui::Button("Add##1"))
		{
			PhivePlacementPopUp.IsOpen() = true;
		}
		if (PhivePlacementPopUp.IsCompleted())
		{
			SelectedActor.GetPhive().Placement.insert({ PhivePlacementPopUp.GetData()[0], PhivePlacementPopUp.GetData()[1] });
			PhivePlacementPopUp.Reset();
		}
		for (auto Iter = SelectedActor.GetPhive().Placement.begin(); Iter != SelectedActor.GetPhive().Placement.end(); )
		{
			auto& [Key, Value] = *Iter;

			ImGui::InputText(Key.c_str(), &Value);
			ImGui::Text(" ");
			ImGui::SameLine();
			if (ImGui::Button(std::string("Del##" + std::to_string(Identifier)).c_str())) {
				Iter = SelectedActor.GetPhive().Placement.erase(Iter);
			}
			else {
				Iter++;
			}
			Identifier++;
		}

		ImGui::NewLine();
		ImGui::Text("Links");
		ImGui::SameLine();
		if (ImGui::Button("Add##2"))
		{
			AddLinkPopUp.IsOpen() = true;
		}
		if (AddLinkPopUp.IsCompleted())
		{
			SelectedActor.GetLinks().push_back({ stoull(AddLinkPopUp.GetData()[0]), AddLinkPopUp.GetData()[1], AddLinkPopUp.GetData()[2], stoull(AddLinkPopUp.GetData()[3]) });
			AddLinkPopUp.Reset();
		}
		for (auto it = SelectedActor.GetLinks().begin(); it != SelectedActor.GetLinks().end();)
		{
			Actor::Link& Link = *it;
			ImGui::InputScalar(std::string("Dst##" + std::to_string(Identifier)).c_str(), ImGuiDataType_::ImGuiDataType_U64, &Link.Dst);
			ImGui::InputText(std::string("Gyaml##" + std::to_string(Identifier + 1)).c_str(), &Link.Gyaml);
			ImGui::InputText(std::string("Name##" + std::to_string(Identifier + 2)).c_str(), &Link.Name);
			ImGui::InputScalar(std::string("Src##" + std::to_string(Identifier + 3)).c_str(), ImGuiDataType_::ImGuiDataType_U64, &Link.Src);
			ImGui::Text(" ");
			ImGui::SameLine();
			if (ImGui::Button(std::string("Del##" + std::to_string(Identifier + 4)).c_str())) {
				it = SelectedActor.GetLinks().erase(it);
			}
			else {
				it++;
			}
			Identifier += 5;
		}

		ImGui::NewLine();
		ImGui::Text("Rails");
		ImGui::SameLine();
		if (ImGui::Button("Add##3"))
		{
			AddRailPopUp.IsOpen() = true;
		}
		if (AddRailPopUp.IsCompleted())
		{
			SelectedActor.GetRails().push_back({ stoull(AddRailPopUp.GetData()[0]), AddRailPopUp.GetData()[1], AddRailPopUp.GetData()[2] });
			AddRailPopUp.Reset();
		}
		for (auto it = SelectedActor.GetRails().begin(); it != SelectedActor.GetRails().end();)
		{
			Actor::Rail& Rail = *it;
			ImGui::InputScalar(std::string("Dst##" + std::to_string(Identifier)).c_str(), ImGuiDataType_::ImGuiDataType_U64, &Rail.Dst);
			ImGui::InputText(std::string("Gyaml##" + std::to_string(Identifier + 1)).c_str(), &Rail.Gyaml);
			ImGui::InputText(std::string("Name##" + std::to_string(Identifier + 2)).c_str(), &Rail.Name);
			ImGui::Text(" ");
			ImGui::SameLine();
			if (ImGui::Button(std::string("Del##" + std::to_string(Identifier + 3)).c_str())) {
				it = SelectedActor.GetRails().erase(it);
			}
			else {
				it++;
			}
			Identifier += 4;
		}

		if (glfwGetKey(Window, GLFW_KEY_DELETE) == GLFW_PRESS)
		{
			Actors->erase(Actors->begin() + PickedActorId);
			PickedActorId = -1;
		}
	}

PopupRendering:

	ImGui::End(); //End the Actor information

	/* --- PopUp rendering --- */

	if (!LoadedEConfig)
	{
		if (!SetPathsPopUp.IsOpen()) SetPathsPopUp.IsOpen() = true;

		SetPathsPopUp.Begin();
		if (SetPathsPopUp.BeginPopupModal()) {
			ImGui::Text("Hello there! Please set the path to your RomFS dump as well as the path to \na directory with all .bfres(not .bfres.mc) files. For decompressing, use this tool:");
			ImGui::Text("https://gamebanana.com/tools/13236");
			ImGui::Text("Note that you have to combine the two directories \"mc_output0\" and \"mc_output1\".");

			ImGui::NewLine();

			ImGui::InputText("RomFS", &SetPathsPopUp.GetData()[0]);
			ImGui::InputText("BFRES", &SetPathsPopUp.GetData()[1]);

			if (ImGui::Button("Okay!")) {
				SetPathsPopUp.IsOpen() = false;
				LoadedEConfig = true;

				Config::RomFSPath = SetPathsPopUp.GetData()[0];
				Config::BfresPath = SetPathsPopUp.GetData()[1];

				EditorConfig::Save();

				MapLoader::DetectInternalGameVersion();

				/* Initialization */
				ZStdFile::Initialize(Config::GetRomFSFile("Pack/ZsDic.pack.zs"));
				ActorModelLibrary::Initialize();
			}
			ImGui::SameLine();
		}
		SetPathsPopUp.End();
	}

	if (LoadedEConfig && !LoadedDungeon)
	{
		if (!LoadMapPopUp.IsOpen()) LoadMapPopUp.IsOpen() = true;

		LoadMapPopUp.Begin();
		if (LoadMapPopUp.BeginPopupModal())
		{
			ImGui::Text("Please enter the map type and identifier:");
			ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x * 0.5f);
			const char* TypeDropdownItems[] = { "SmallDungeon", "MainField" };
			ImGui::Combo("Type", reinterpret_cast<int*>(&LoadMapPopUp.IntData), TypeDropdownItems, IM_ARRAYSIZE(TypeDropdownItems));
			ImGui::InputText(LoadMapPopUp.IntData == 0 ? ("Dungeon ID (e.g. 001)") : ("MainField section (e.g. E-6)"), &LoadMapPopUp.GetData()[0]);
			if (ImGui::Button("Load") && LoadMapPopUp.GetData()[0].length() == 3)
			{
				LoadMapPopUp.IsOpen() = false;
				LoadedDungeon = true;
				Actors->clear();
				(*Actors) = MapLoader::LoadMap(LoadMapPopUp.GetData()[0], (MapLoader::Type)LoadMapPopUp.IntData);
				camera.Position.x = Actors->at(0).GetTranslate().GetX();
				camera.Position.y = Actors->at(0).GetTranslate().GetY();
				camera.Position.z = Actors->at(0).GetTranslate().GetZ();
			}
			ImGui::PopItemWidth();
		}
		LoadMapPopUp.End();
	}

	if (ExportModPopUp.IsOpen())
	{
		ExportModPopUp.Begin();
		if (ExportModPopUp.BeginPopupModal())
		{
			ImGui::InputText("Export Path", &ExportModPopUp.GetData()[0]);
			if (ImGui::Button("Export"))
			{
				ExportModPopUp.IsCompleted() = true;
				ExportModPopUp.IsOpen() = false;
			}
			ImGui::SameLine();
			if (ImGui::Button("Return"))
			{
				ExportModPopUp.Reset();
			}
		}
		ExportModPopUp.End();
	}

	if (AddLinkPopUp.IsOpen())
	{
		AddLinkPopUp.Begin();
		if (AddLinkPopUp.BeginPopupModal())
		{
			ImGui::InputText("Dst", &AddLinkPopUp.GetData()[0]);
			ImGui::InputText("Gyml", &AddLinkPopUp.GetData()[1]);
			ImGui::InputText("Name", &AddLinkPopUp.GetData()[2]);
			ImGui::InputText("Src", &AddLinkPopUp.GetData()[3]);
			if (ImGui::Button("Add"))
			{
				AddLinkPopUp.IsCompleted() = true;
				AddLinkPopUp.IsOpen() = false;
			}
			ImGui::SameLine();
			if (ImGui::Button("Return"))
			{
				AddLinkPopUp.Reset();
			}
		}
		AddLinkPopUp.End();
	}

	if (AddRailPopUp.IsOpen())
	{
		AddRailPopUp.Begin();
		if (AddRailPopUp.BeginPopupModal())
		{
			ImGui::InputText("Dst", &AddRailPopUp.GetData()[0]);
			ImGui::InputText("Gyml", &AddRailPopUp.GetData()[1]);
			ImGui::InputText("Name", &AddRailPopUp.GetData()[2]);
			if (ImGui::Button("Add"))
			{
				AddRailPopUp.IsCompleted() = true;
				AddRailPopUp.IsOpen() = false;
			}
			ImGui::SameLine();
			if (ImGui::Button("Return"))
			{
				AddRailPopUp.Reset();
			}
		}
		AddRailPopUp.End();
	}

	if (StackActorsPopUp.IsOpen())
	{
		StackActorsPopUp.Begin();
		if (StackActorsPopUp.BeginPopupModal())
		{
			ImGui::InputText("Original Actor SRTHash", &StackActorsPopUp.GetData()[0]);
			ImGui::InputText("Amount", &StackActorsPopUp.GetData()[1]);
			ImGui::InputText("Offset X", &StackActorsPopUp.GetData()[2]);
			ImGui::InputText("Offset Y", &StackActorsPopUp.GetData()[3]);
			ImGui::InputText("Offset Z", &StackActorsPopUp.GetData()[4]);
			if (ImGui::Button("Stack"))
			{
				StackActorsPopUp.IsCompleted() = true;
				StackActorsPopUp.IsOpen() = false;
			}
			ImGui::SameLine();
			if (ImGui::Button("Return"))
			{
				StackActorsPopUp.Reset();
			}
		}
		StackActorsPopUp.End();
	}

	if (GenerateHashesPopUp.IsOpen())
	{
		GenerateHashesPopUp.Begin();
		if (GenerateHashesPopUp.BeginPopupModal())
		{
			ImGui::Text("Do you really want to generate physics hashes?");
			ImGui::Text("(This cannot be undone)");
			ImGui::NewLine();
			ImGui::Text("Map Model Merged Actor: Fast, use when the original map has a large map model -> Dungeons");
			ImGui::Text("Local Actors: Slower, use when the original map has many actors -> MainField, Dungeons (Requires restart)");
			ImGui::NewLine();
			const char* MethodDropdownItems[] = { "Map Model Merged Actor", "Local Actors" };
			ImGui::Combo("##0", reinterpret_cast<int*>(&GenerateHashesPopUp.IntData), MethodDropdownItems, IM_ARRAYSIZE(MethodDropdownItems));
			if (GenerateHashesPopUp.IntData == 0)
			{
				ImGui::InputText("Map Model SRTHash", &GenerateHashesPopUp.GetData()[0]);
				ImGui::SameLine();
				if (ImGui::Button("Auto"))
				{
					std::pair<uint32_t, uint32_t> BiggestMergedByml;
					for (Actor& Actor : *Actors)
					{
						if (Actor.GetGyml() == "MergedActorUncond" && Actor.GetDynamic().DynamicString.size() == 1)
						{
							struct stat FileStatus;
							stat(Config::GetRomFSFile(Actor.GetDynamic().DynamicString["BancPath"] + ".zs").c_str(), &FileStatus);

							if (FileStatus.st_size > BiggestMergedByml.second)
							{
								BiggestMergedByml.first = Actor.GetSRTHash();
								BiggestMergedByml.second = FileStatus.st_size;
							}
						}
					}

					GenerateHashesPopUp.GetData()[0] = std::to_string(BiggestMergedByml.first);
				}
			}
			ImGui::NewLine();
			if (ImGui::Button("Confirm"))
			{
				GenerateHashesPopUp.IsCompleted() = true;
				GenerateHashesPopUp.IsOpen() = false;
			}
			ImGui::SameLine();
			if (ImGui::Button("Return"))
			{
				GenerateHashesPopUp.Reset();
			}
		}
		GenerateHashesPopUp.End();
	}

	if (DynamicPopUp.IsOpen())
	{
		DynamicPopUp.Begin();
		if (DynamicPopUp.BeginPopupModal())
		{
			ImGui::InputText("Key", &DynamicPopUp.GetData()[0]);
			ImGui::InputText("Value", &DynamicPopUp.GetData()[1]);

			if (ImGui::Button("Add"))
			{
				DynamicPopUp.IsCompleted() = true;
				DynamicPopUp.IsOpen() = false;
			}
			ImGui::SameLine();
			if (ImGui::Button("Close"))
			{
				DynamicPopUp.Reset();
			}
		}
		DynamicPopUp.End();
	}

	if (PhivePlacementPopUp.IsOpen())
	{
		PhivePlacementPopUp.Begin();
		if (PhivePlacementPopUp.BeginPopupModal())
		{
			ImGui::InputText("Key", &PhivePlacementPopUp.GetData()[0]);
			ImGui::InputText("Value", &PhivePlacementPopUp.GetData()[1]);

			if (ImGui::Button("Add"))
			{
				PhivePlacementPopUp.IsCompleted() = true;
				PhivePlacementPopUp.IsOpen() = false;
			}
			ImGui::SameLine();
			if (ImGui::Button("Close"))
			{
				PhivePlacementPopUp.Reset();
			}
		}
		PhivePlacementPopUp.End();
	}

	if (AddActorPopUp.IsOpen())
	{
		AddActorPopUp.Begin();
		if (AddActorPopUp.BeginPopupModal())
		{
			ImGui::InputText("Gyaml", &AddActorPopUp.GetData()[0]);
			ImGui::InputText("Hash", &AddActorPopUp.GetData()[1]);
			ImGui::InputText("SRTHash", &AddActorPopUp.GetData()[2]);

			if (ImGui::Button("Add"))
			{
				AddActorPopUp.IsCompleted() = true;
				AddActorPopUp.IsOpen() = false;
			}
			ImGui::SameLine();
			if (ImGui::Button("Close"))
			{
				AddActorPopUp.Reset();
			}
		}
		AddActorPopUp.End();
	}

	/*
	//Assets
	ImGui::SetNextWindowPos(ImVec2(0, height * 0.75));
	ImGui::SetNextWindowSize(ImVec2(width, height * 0.25));
	ImGui::Begin("Assets", NULL, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
	ImGui::End();
	*/

	if (!NodeEditor.m_File.Loaded)
	{
		NodeEditor.LoadAINB(Config::GetRomFSFile("Logic/Dungeon001_1800.logic.root.ainb"));
	}

	//AINB Drawing
	ImGui::Begin("AINB Node Editor");
	if (ImGui::IsWindowFocused()) PickedActorId = -1;
	NodeEditor.DrawNodeEditor();
	ImGui::End();

	if (FirstFrame)
	{
		ImGui::DockBuilderRemoveNode(DockSpace);
		ImGui::DockBuilderAddNode(DockSpace, ImGuiDockNodeFlags_DockSpace);
		ImGui::DockBuilderSetNodeSize(DockSpace, ImGui::GetMainViewport()->Size);

		ImGuiID DockLeft, DockMiddle, DockRight;
		ImGui::DockBuilderSplitNode(DockSpace, ImGuiDir_Left, 0.2f * Config::UIScale / 1.2f, &DockLeft, &DockMiddle);
		ImGui::DockBuilderSplitNode(DockMiddle, ImGuiDir_Right, 0.3f * Config::UIScale / 1.2f, &DockRight, &DockMiddle);

		ImGui::DockBuilderDockWindow("Actions", DockLeft);
		ImGui::DockBuilderDockWindow("Map View", DockMiddle);
		ImGui::DockBuilderDockWindow("AINB Node Editor", DockMiddle);
		ImGui::DockBuilderDockWindow("Properties", DockRight);

		ImGui::DockBuilderFinish(DockSpace);

		ImGui::SetWindowFocus("Map View");

		FirstFrame = false;
	}

	ImGui::Render();

	camera.Inputs(Window);

	glBindFramebuffer(GL_FRAMEBUFFER, FBO);

	glClearColor(ClearColor.x * ClearColor.w, ClearColor.y * ClearColor.w, ClearColor.z * ClearColor.w, ClearColor.w);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	TextureShader.Activate();
	camera.Matrix(45.0f, 0.1f, 30000.0f, TextureShader, "camMatrix");

	Frustum::CalculateFrustum();

	for (int ActorIndex = 0; ActorIndex < Actors->size(); ActorIndex++)
	{
		Actor& Actor = Actors->at(ActorIndex);

		if (Actor.GetModel()->IsDefaultModel() && !RenderSettings.Invisible) continue;
		if (!Actor.GetModel()->IsDefaultModel() && !RenderSettings.Visible) continue;
		if (Actor.GetGyml().ends_with("_Far") && !RenderSettings.FarActors) continue;
		if (Actor.GetGyml().find("Area") != std::string::npos && !RenderSettings.Area) continue;

		if (Actor.GetModel()->GetModels().size() == 0) continue;
		if (Actor.GetModel()->GetModels()[0].LODs.size() == 0) continue;

		if (!Frustum::SphereInFrustum(Actor.GetTranslate().GetX(), Actor.GetTranslate().GetY(), Actor.GetTranslate().GetZ(), Actor.GetModel()->GetModels()[0].BoundingBoxSphereRadius * Max(Max(Max(1, Actor.GetScale().GetX()), Actor.GetScale().GetY()), Actor.GetScale().GetZ()))) {
			continue;
		}

		glm::mat4 model = glm::mat4(1.0f);  // Identity matrix

		model = glm::translate(model, glm::vec3(Actor.GetTranslate().GetX(), Actor.GetTranslate().GetY(), Actor.GetTranslate().GetZ()));

		model = glm::rotate(model, glm::radians(Actor.GetRotate().GetZ()), glm::vec3(0.0, 0.0f, 1.0));
		model = glm::rotate(model, glm::radians(Actor.GetRotate().GetY()), glm::vec3(0.0f, 1.0, 0.0));
		model = glm::rotate(model, glm::radians(Actor.GetRotate().GetX()), glm::vec3(1.0, 0.0f, 0.0));

		model = glm::scale(model, glm::vec3(Actor.GetScale().GetX(), Actor.GetScale().GetY(), Actor.GetScale().GetZ()));

		float DistanceToCamera = fabs(pow(Actor.GetTranslate().GetX() - camera.Position.x, 2) + pow(Actor.GetTranslate().GetY() - camera.Position.y, 2) + pow(Actor.GetTranslate().GetZ() - camera.Position.z, 2));

		BfresFile::LOD* LODModel = &Actor.GetModel()->GetModels()[0].LODs[0];

		if (DistanceToCamera >= 10000) //Max distance is 100, 10000 = 100 to the power of 2
		{
			LODModel = &Actor.GetModel()->GetModels()[0].LODs[Actor.GetModel()->GetModels()[0].LODs.size() - 1];
		}

		glUniformMatrix4fv(glGetUniformLocation(TextureShader.ID, "modelMatrix"), 1, GL_FALSE, glm::value_ptr(model));

		for (int SubModelIndex = 0; SubModelIndex < LODModel->GL_Textures.size(); SubModelIndex++)
		{
			LODModel->GL_Textures[SubModelIndex]->texUnit(TextureShader, "Texture", 0);
			LODModel->GL_Textures[SubModelIndex]->Bind();
			LODModel->GL_VAO[SubModelIndex].Bind();

			glDrawElements(GL_TRIANGLES, sizeof(int) * LODModel->Faces[SubModelIndex].size(), GL_UNSIGNED_INT, 0);
		}
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	glfwSwapBuffers(Window);
}