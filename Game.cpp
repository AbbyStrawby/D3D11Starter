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
			XMFLOAT3(0, 2, -20.0f),
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

	// Initialize Lights
	Light dirLight1 = {};
	dirLight1.Type = LIGHT_TYPE_DIRECTIONAL;
	dirLight1.Direction = XMFLOAT3(1, 0, 0);
	dirLight1.Color = XMFLOAT3(1.0f, 0.1f, 0.1f);
	dirLight1.Intensity = 1.0f;

	Light dirLight2 = {};
	dirLight2.Type = LIGHT_TYPE_DIRECTIONAL;
	dirLight2.Direction = XMFLOAT3(0, -1, 0);
	dirLight2.Color = XMFLOAT3(0.1f, 1.0f, 0.1f);
	dirLight2.Intensity = 1.0f;

	Light dirLight3 = {};
	dirLight3.Type = LIGHT_TYPE_DIRECTIONAL;
	dirLight3.Direction = XMFLOAT3(-1, 1, 0);
	dirLight3.Color = XMFLOAT3(0.1f, 0.1f, 1.0f);
	dirLight3.Intensity = 1.0f;

	Light pointLight1 = {};
	pointLight1.Type = LIGHT_TYPE_POINT;
	pointLight1.Position = XMFLOAT3(-1.5f, 0, 0);
	pointLight1.Color = XMFLOAT3(1, 1, 1);
	pointLight1.Intensity = 1.0f;
	pointLight1.Range = 8.0f;

	Light pointLight2 = {};
	pointLight2.Type = LIGHT_TYPE_POINT;
	pointLight2.Position = XMFLOAT3(1.5f, 0, 0);
	pointLight2.Color = XMFLOAT3(1, 1, 1);
	pointLight2.Intensity = 0.5f;
	pointLight2.Range = 12.0f;

	lights.push_back(dirLight1);
	lights.push_back(dirLight2);
	lights.push_back(dirLight3);
	lights.push_back(pointLight1);
	lights.push_back(pointLight2);
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

	// Load Sampler State
	Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler;
	D3D11_SAMPLER_DESC samplerDesc = {};
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	samplerDesc.MaxAnisotropy = 4;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	Graphics::Device.Get()->CreateSamplerState(&samplerDesc, sampler.GetAddressOf());

	// Load textures
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> tilesSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> tilesSpecularSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> brokenTilesSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> brokenTilesSpecularSRV;

	CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(), 
		FixPath(L"../../Assets/Textures/tiles.png").c_str(), 0, tilesSRV.GetAddressOf());
	CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(), 
		FixPath(L"../../Assets/Textures/tiles_specular.png").c_str(), 0, tilesSpecularSRV.GetAddressOf());
	CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(),
		FixPath(L"../../Assets/Textures/brokentiles.png").c_str(), 0, brokenTilesSRV.GetAddressOf());
	CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(),
		FixPath(L"../../Assets/Textures/brokentiles_specular.png").c_str(), 0, brokenTilesSpecularSRV.GetAddressOf());

	// Create materials before creating entities
	shared_ptr<Material> matWhite = make_shared<Material>(vertexShader, pixelShader, XMFLOAT3(1, 1, 1), 0.5f, 1, 0);
	//shared_ptr<Material> matPurple = make_shared<Material>(vertexShader, pixelShader, XMFLOAT3(0.8f, 0, 0.8f), 0.5f);
	//shared_ptr<Material> matOrange = make_shared<Material>(vertexShader, pixelShader, XMFLOAT3(1.0f, 0.5f, 0), 0.5f);
	shared_ptr<Material> matUV = make_shared<Material>(vertexShader, uvPS, XMFLOAT3(0, 0, 0), 0.5f, 1, 0);
	shared_ptr<Material> matNormal = make_shared<Material>(vertexShader, normalPS, XMFLOAT3(0, 0, 0), 0.5f, 1, 0);
	shared_ptr<Material> matCustom = make_shared<Material>(vertexShader, customPS, XMFLOAT3(1, 1, 1), 0.5f, 1, 0);
	shared_ptr<Material> matTiles = make_shared<Material>(vertexShader, pixelShader, XMFLOAT3(1, 1, 1), 0.5f, 1, 0.1f);
	shared_ptr<Material> matBrokenTiles = make_shared<Material>(vertexShader, pixelShader, XMFLOAT3(1, 1, 1), 0.5f, 1, 0.1f);

	// Add Textures/Samplers to Materials
	matTiles->AddTextureSRV("SurfaceTexture", tilesSRV);
	matTiles->AddTextureSRV("SpecularMap", tilesSpecularSRV);
	matTiles->AddSampler("BasicSampler", sampler);

	matBrokenTiles->AddTextureSRV("SurfaceTexture", brokenTilesSRV);
	matBrokenTiles->AddTextureSRV("SpecularMap", brokenTilesSpecularSRV);
	matBrokenTiles->AddSampler("BasicSampler", sampler);

	// Add material objects to vector
	materials.push_back(matWhite);
	//materials.push_back(matPurple);
	//materials.push_back(matOrange);
	materials.push_back(matUV);
	materials.push_back(matNormal);
	materials.push_back(matTiles);
	materials.push_back(matBrokenTiles);

	// Make game entities
	std::shared_ptr<Entity> entity1 = std::make_shared<Entity>(cubeMesh, matBrokenTiles);
	std::shared_ptr<Entity> entity2 = std::make_shared<Entity>(cylinderMesh, matBrokenTiles);
	std::shared_ptr<Entity> entity3 = std::make_shared<Entity>(helixMesh, matBrokenTiles);
	std::shared_ptr<Entity> entity4 = std::make_shared<Entity>(sphereMesh, matTiles);
	std::shared_ptr<Entity> entity5 = std::make_shared<Entity>(torusMesh, matTiles);
	std::shared_ptr<Entity> entity6 = std::make_shared<Entity>(quadMesh, matTiles);
	std::shared_ptr<Entity> entity7 = std::make_shared<Entity>(quadDoubleSideMesh, matTiles);

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
			// Set pixel shader data
			std::shared_ptr<SimplePixelShader> ps = e->GetMaterial()->GetPixelShader();
			ps->SetFloat3("ambient", ambientColor);
			ps->SetData("lights", &lights[0], sizeof(Light) * (int)lights.size());

			// Draw the entity
			e->Draw(cameras[activeCam]);
		}
	}

	// Frame END
	// - These should happen exactly ONCE PER FRAME
	// - At the very end of the frame (after drawing *everything*)
	{
		// Draw ImGui on top of everything else
		ImGui::Render(); // Turns this frame’s UI into renderable triangles
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

	if (ImGui::TreeNode("Lights"))
	{
		// Ambient Color
		ImGui::Spacing();
		ImGui::ColorEdit3("Ambient Color", &ambientColor.x);
		ImGui::Spacing();

		for (int i = 0; i < lights.size(); i++) {


			string lightName;

			if (lights[i].Type == LIGHT_TYPE_DIRECTIONAL)
				lightName = "Directional Light";
			else if (lights[i].Type == LIGHT_TYPE_POINT)
				lightName = "Point Light";
			else if (lights[i].Type == LIGHT_TYPE_SPOT)
				lightName = "Spot Light";
			
			ImGui::PushID(&lights[i]);

			if (ImGui::TreeNode("Light", lightName.c_str())) {
				ImGui::Spacing();

				// Drag sliders for values
				// Color
				ImGui::ColorEdit3("Color", &lights[i].Color.x);

				ImGui::Spacing();
				
				// Position (Point Light Exclusive)
				if (lightName == "Point Light") {
					XMFLOAT3 position = lights[i].Position;
					ImGui::DragFloat3("Position", &lights[i].Position.x, 0.01f);

					ImGui::Spacing();
				}

				ImGui::TreePop();
			}

			ImGui::PopID();
		}

		ImGui::TreePop();
	}
	

	
	ImGui::End(); // Ends the current window
}


