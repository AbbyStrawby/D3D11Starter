#include "Game.h"
#include "Graphics.h"
#include "Vertex.h"
#include "Input.h"
#include "PathHelpers.h"
#include "Window.h"

#include <DirectXMath.h>

// This code assumes files are in "ImGui" subfolder!
// Adjust as necessary for your own folder structure and project setup
#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_dx11.h"
#include "ImGui/imgui_impl_win32.h"

// Needed for a helper function to load pre-compiled shader files
#pragma comment(lib, "d3dcompiler.lib")
#include <d3dcompiler.h>

// For the DirectX Math library
using namespace DirectX;
using namespace std;

// --------------------------------------------------------
// Called once per program, after the window and graphics API
// are initialized but before the game loop begins
// --------------------------------------------------------
void Game::Initialize()
{
	// Helper methods for loading shaders, creating some basic
	// geometry to draw and some simple camera matrices.
	//  - You'll be expanding and/or replacing these later
	LoadShadersAndCreateGeometry();

	// Set initial graphics API state
	//  - These settings persist until we change them
	//  - Some of these, like the primitive topology & input layout, probably won't change
	//  - Others, like setting shaders, will need to be moved elsewhere later
	{
		// Tell the input assembler (IA) stage of the pipeline what kind of
		// geometric primitives (points, lines or triangles) we want to draw.  
		// Essentially: "What kind of shape should the GPU draw with our vertices?"
		Graphics::Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	}

	// Initialize ImGui itself & platform/renderer backends
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui_ImplWin32_Init(Window::Handle());
	ImGui_ImplDX11_Init(Graphics::Device.Get(), Graphics::Context.Get());
	// Pick a style (uncomment one of these 3)
	//ImGui::StyleColorsDark();
	//ImGui::StyleColorsLight();
	ImGui::StyleColorsClassic();

	// Create Camera(s)
	std::shared_ptr<Camera> cam1 =
		std::make_shared<Camera>(
			(float)Window::Width() / Window::Height(),
			XMFLOAT3(0, 4, -20.0f),
			XMFLOAT3(),
			XM_PIDIV4,
			0.01f,
			100.0f,
			7.0f,
			0.004f);
	std::shared_ptr<Camera> cam2 =
		std::make_shared<Camera>(
			(float)Window::Width() / Window::Height(),
			XMFLOAT3(2.5f, 1.5f, -2.5f),
			XMFLOAT3(XM_PI / 8, -XM_PIDIV4, 0),
			XM_PIDIV2,
			0.01f,
			100.0f,
			5.0f,
			0.004f);


	cameras.push_back(cam1);
	cameras.push_back(cam2);
}


// --------------------------------------------------------
// Clean up memory or objects created by this class
// 
// Note: Using smart pointers means there probably won't
//       be much to manually clean up here!
// --------------------------------------------------------
Game::~Game()
{
	// ImGui clean up
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}


// --------------------------------------------------------
// Creates the geometry we're going to draw
// --------------------------------------------------------
void Game::LoadShadersAndCreateGeometry()
{
	// Create vertex and pixel shaders
	vertexShader = std::make_shared<SimpleVertexShader>(Graphics::Device,
		Graphics::Context, FixPath(L"VertexShader.cso").c_str());
	pixelShader = std::make_shared<SimplePixelShader>(Graphics::Device,
		Graphics::Context, FixPath(L"PixelShader.cso").c_str());
	uvPS = std::make_shared<SimplePixelShader>(Graphics::Device,
		Graphics::Context, FixPath(L"uvPS.cso").c_str());
	normalPS = std::make_shared<SimplePixelShader>(Graphics::Device,
		Graphics::Context, FixPath(L"normalPS.cso").c_str());
	customPS = std::make_shared<SimplePixelShader>(Graphics::Device,
		Graphics::Context, FixPath(L"customPS.cso").c_str());

	// Load 3D Models
	shared_ptr<Mesh> cubeMesh = make_shared<Mesh>("Cube", FixPath("../../Assets/Models/cube.obj").c_str());
	shared_ptr<Mesh> cylinderMesh = make_shared<Mesh>("Cylinder", FixPath("../../Assets/Models/cylinder.obj").c_str());
	shared_ptr<Mesh> helixMesh = make_shared<Mesh>("Helix", FixPath("../../Assets/Models/helix.obj").c_str());
	shared_ptr<Mesh> sphereMesh = make_shared<Mesh>("Sphere", FixPath("../../Assets/Models/sphere.obj").c_str());
	shared_ptr<Mesh> torusMesh = make_shared<Mesh>("Torus", FixPath("../../Assets/Models/torus.obj").c_str());
	shared_ptr<Mesh> quadMesh = make_shared<Mesh>("Quad", FixPath("../../Assets/Models/quad.obj").c_str());
	shared_ptr<Mesh> quadDoubleSideMesh = make_shared<Mesh>("Double-Sided Quad", FixPath("../../Assets/Models/quad_double_sided.obj").c_str());

	// Create materials before creating entities
	shared_ptr<Material> matWhite = make_shared<Material>(vertexShader, pixelShader, XMFLOAT4(1, 1, 1, 0));
	shared_ptr<Material> matPurple = make_shared<Material>(vertexShader, pixelShader, XMFLOAT4(0.8f, 0, 0.8f, 0));
	shared_ptr<Material> matOrange = make_shared<Material>(vertexShader, pixelShader, XMFLOAT4(1.0f, 0.5f, 0, 0));
	shared_ptr<Material> matUV = make_shared<Material>(vertexShader, uvPS, XMFLOAT4(0, 0, 0, 0));
	shared_ptr<Material> matNormal = make_shared<Material>(vertexShader, normalPS, XMFLOAT4(0, 0, 0, 0));
	shared_ptr<Material> matCustom = make_shared<Material>(vertexShader, customPS, XMFLOAT4(1, 1, 1, 0));

	// Add material objects to vector
	materials.push_back(matWhite);
	materials.push_back(matPurple);
	materials.push_back(matOrange);
	materials.push_back(matUV);
	materials.push_back(matNormal);

	// Make game entities
	std::shared_ptr<Entity> entity1 = std::make_shared<Entity>(cubeMesh, matCustom);
	std::shared_ptr<Entity> entity2 = std::make_shared<Entity>(cylinderMesh, matWhite);
	std::shared_ptr<Entity> entity3 = std::make_shared<Entity>(helixMesh, matWhite);
	std::shared_ptr<Entity> entity4 = std::make_shared<Entity>(sphereMesh, matCustom);
	std::shared_ptr<Entity> entity5 = std::make_shared<Entity>(torusMesh, matWhite);
	std::shared_ptr<Entity> entity6 = std::make_shared<Entity>(quadMesh, matWhite);
	std::shared_ptr<Entity> entity7 = std::make_shared<Entity>(quadDoubleSideMesh, matWhite);

	// Spread out some of the entities so they aren't all on top of one another
	entity1->GetTransform()->MoveAbsolute(-9, 0, 0);
	entity2->GetTransform()->MoveAbsolute(-6, 0, 0);
	entity3->GetTransform()->MoveAbsolute(-3, 0, 0);
	entity4->GetTransform()->MoveAbsolute(0, 0, 0);
	entity5->GetTransform()->MoveAbsolute(3, 0, 0);
	entity6->GetTransform()->MoveAbsolute(6, 0, 0);
	entity7->GetTransform()->MoveAbsolute(9, 0, 0);


	// Add entity objects to the list
	entities.push_back(entity1);
	entities.push_back(entity2);
	entities.push_back(entity3);
	entities.push_back(entity4);
	entities.push_back(entity5);
	entities.push_back(entity6);
	entities.push_back(entity7);

	// Create entities similar to sample with uv and normal materials
	int numEntities = (int)entities.size();
	for (int i = 0; i < numEntities; i++) {
		// Get the mesh of the entity
		shared_ptr<Mesh> mesh = entities[i]->GetMesh();

		// Make uv entity, transform it, and add it to list of entities
		shared_ptr<Entity> uvEntity = make_shared<Entity>(mesh, matUV);
		uvEntity->GetTransform()->MoveAbsolute(entities[i]->GetTransform()->GetPosition());	// Move to initial pos
		uvEntity->GetTransform()->MoveAbsolute(0, 3, 0);									// Then move it up
		entities.push_back(uvEntity);

		// Repeat for normal entity
		shared_ptr<Entity> normalEntity = make_shared<Entity>(mesh, matNormal);
		normalEntity->GetTransform()->MoveAbsolute(entities[i]->GetTransform()->GetPosition());	// Move to initial pos
		normalEntity->GetTransform()->MoveAbsolute(0, 6, 0);									// Then move it up
		entities.push_back(normalEntity);
	}
}


// --------------------------------------------------------
// Handle resizing to match the new window size
//  - Eventually, we'll want to update our 3D camera
// --------------------------------------------------------
void Game::OnResize()
{
	// Update Camera projection matrices
	for (std::shared_ptr<Camera> c : cameras) {
		c->UpdateProjectionMatrix((float)Window::Width() / Window::Height());
	}
}


// --------------------------------------------------------
// Update your game here - user input, move objects, AI, etc.
// --------------------------------------------------------
void Game::Update(float deltaTime, float totalTime)
{
	// Update the ImGui
	UIUpdate(deltaTime);

	// Build the custom UI
	BuildUI(deltaTime);

	// Update the active cam only
	cameras[activeCam]->Update(deltaTime);

	// Example input checking: Quit if the escape key is pressed
	if (Input::KeyDown(VK_ESCAPE))
		Window::Quit();

	// Update Transformations
	
	for (std::shared_ptr<Entity> e : entities) {
		e->GetTransform()->Rotate(0, deltaTime, 0);
	}
	
	entities[0]->GetTransform()->Rotate(deltaTime, 0, deltaTime);
	//float scaleSize = (float)sin(totalTime * 2) * 0.2f + 0.8f;
	//entities[0]->GetTransform()->SetScale(scaleSize, scaleSize, scaleSize);
	//entities[0]->GetTransform()->Rotate(0, 0, deltaTime);

	//float moveDistance = (float)cos(totalTime) * 0.5f;
	//entities[3]->GetTransform()->SetPosition(moveDistance, 0.6f, 0);
	//entities[4]->GetTransform()->SetPosition(-moveDistance, 0.6f, 0);
}


// --------------------------------------------------------
// Clear the screen, redraw everything, present to the user
// --------------------------------------------------------
void Game::Draw(float deltaTime, float totalTime)
{
	// Frame START
	// - These things should happen ONCE PER FRAME
	// - At the beginning of Game::Draw() before drawing *anything*
	{
		// Clear the back buffer (erase what's on screen) and depth buffer
		const float color[4] = { 0.4f, 0.6f, 0.75f, 0.0f };
		Graphics::Context->ClearRenderTargetView(Graphics::BackBufferRTV.Get(),	backgroundColor);
		Graphics::Context->ClearDepthStencilView(Graphics::DepthBufferDSV.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
	}

	// DRAW geometry
	// Loop through and draw every mesh
	{
		for (std::shared_ptr<Entity> e : entities) {
			e->Draw(cameras[activeCam]);
		}
	}

	// Frame END
	// - These should happen exactly ONCE PER FRAME
	// - At the very end of the frame (after drawing *everything*)
	{
		// Draw ImGui on top of everything else
		ImGui::Render(); // Turns this frame�s UI into renderable triangles
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData()); // Draws it to the screen

		// Present at the end of the frame
		bool vsync = Graphics::VsyncState();
		Graphics::SwapChain->Present(
			vsync ? 1 : 0,
			vsync ? 0 : DXGI_PRESENT_ALLOW_TEARING);

		// Re-bind back buffer and depth buffer after presenting
		Graphics::Context->OMSetRenderTargets(
			1,
			Graphics::BackBufferRTV.GetAddressOf(),
			Graphics::DepthBufferDSV.Get());
	}
}

// --------------------------------------------------------
// Create new frame for the UI
// --------------------------------------------------------
void Game::UIUpdate(float deltaTime) 
{
	// Feed fresh data to ImGui
	ImGuiIO& io = ImGui::GetIO();
	io.DeltaTime = deltaTime;
	io.DisplaySize.x = (float)Window::Width();
	io.DisplaySize.y = (float)Window::Height();

	// Reset the frame
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	// Determine new input capture
	Input::SetKeyboardCapture(io.WantCaptureKeyboard);
	Input::SetMouseCapture(io.WantCaptureMouse);
}

// --------------------------------------------------------
// Build Custom UI
// --------------------------------------------------------
void Game::BuildUI(float deltaTime)
{
	// Check if we want to show demo window
	if (showDemoUI) {
		ImGui::ShowDemoWindow();
	}

	ImGui::Begin("Abby Program Analyzer"); // Everything after is part of the window

	ImGui::Spacing();

	// Group Info from ImGui Assignment together

	if (ImGui::TreeNode("App Details")) 
	{
		// Replace the %f with the next parameter, and format as a float
		ImGui::BulletText("Framerate: %f fps", ImGui::GetIO().Framerate);

		// Replace each % d with the next parameter, and format as decimal integers
		// The "x" will be printed as-is between the numbers, like so: 800x600
		ImGui::BulletText("Window Resolution: %dx%d", Window::Width(), Window::Height());

		ImGui::Spacing();

		// Can create a 3 or 4-component color editors, too!
		// - Notice the two different function names below
		ImGui::ColorEdit4("RGBA color editor", &backgroundColor[0]);

		ImGui::Spacing();

		if (ImGui::Button("Toggle Demo Display"))
		{
			// Toggle the state of show demo ui
			showDemoUI = !showDemoUI;
		}

		ImGui::Spacing();

		static bool check = true;
		ImGui::Checkbox("Box Checked", &check);

		ImGui::Spacing();

		static int slider = 0;
		ImGui::SliderInt("Slider int", &slider, -100, 100);

		ImGui::TreePop();
	}

	if (ImGui::TreeNode("Mesh Details"))
	{
		for (int i = 0; i < meshes.size(); i++) {
			ImGui::PushID(meshes[i].get());

			if (ImGui::TreeNode("Mesh Node", "%s", meshes[i]->GetName())) {
				ImGui::Spacing();

				ImGui::Text("Triangles: %d", meshes[i]->GetIndexCount() / 3);
				ImGui::Text("Vertices: %d", meshes[i]->GetVertexCount());
				ImGui::Text("Indicecs: %d", meshes[i]->GetIndexCount());

				ImGui::Spacing();

				ImGui::TreePop();
			}

			ImGui::PopID();
		}

		ImGui::TreePop();
	}

	if (ImGui::TreeNode("Game Entities"))
	{
		for (int i = 0; i < entities.size(); i++) {
			ImGui::PushID(entities[i].get());

			if (ImGui::TreeNode("Entity", "Entity %d", i)) {
				ImGui::Spacing();

				// Mesh name
				ImGui::Text("Mesh: %s", entities[i]->GetMesh()->GetName());

				ImGui::Spacing();

				// Transform variables
				std::shared_ptr<Transform> transform = entities[i]->GetTransform();

				XMFLOAT3 position = transform->GetPosition();
				XMFLOAT3 rotation = transform->GetPitchYawRoll();
				XMFLOAT3 scale = transform->GetScale();

				// Drag sliders for transform values
				// Position
				if (ImGui::DragFloat3("Position", &position.x, 0.01f))
					transform->SetPosition(position);
				// Rotation
				if (ImGui::DragFloat3("Rotation", &rotation.x, 0.01f))
					transform->SetRotation(rotation);
				// Scale
				if (ImGui::DragFloat3("Scale", &scale.x, 0.01f))
					transform->SetScale(scale);

				ImGui::Spacing();

				ImGui::TreePop();
			}

			ImGui::PopID();
		}

		ImGui::TreePop();
	}

	if (ImGui::TreeNode("Cameras"))
	{
		// Camera name
		ImGui::Text("Camera %d", activeCam);
		ImGui::Spacing();

		XMFLOAT3 pos = cameras[activeCam]->GetTransform()->GetPosition();
		XMFLOAT3 rot = cameras[activeCam]->GetTransform()->GetPitchYawRoll();

		ImGui::Text("Pos: %f, %f, %f", pos.x, pos.y, pos.z);
		ImGui::Text("Angle: %f, %f, %f", rot.x, rot.y, rot.z);
		ImGui::Text("delta: %f", deltaTime);
		ImGui::Text("FOV: %f", cameras[activeCam]->GetFOV());
		ImGui::Text("Near: %f", cameras[activeCam]->GetNearClip());
		ImGui::Text("Far: %f", cameras[activeCam]->GetFarClip());

		// Buttons to toggle through cameras
		if (ImGui::Button("Cam 0")) { activeCam = 0; } ImGui::SameLine();
		if (ImGui::Button("Cam 1")) { activeCam = 1; }

		ImGui::TreePop();
	}

	

	
	ImGui::End(); // Ends the current window
}


