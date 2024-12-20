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
	dirLight1.Direction = XMFLOAT3(1, -1, 1);
	dirLight1.Color = XMFLOAT3(0.8f, 0.8f, 0.8f);
	dirLight1.Intensity = 1.0f;

	Light dirLight2 = {};
	dirLight2.Type = LIGHT_TYPE_DIRECTIONAL;
	dirLight2.Direction = XMFLOAT3(1, 0, 0);
	dirLight2.Color = XMFLOAT3(1.0f, 0.1f, 0.1f);
	dirLight2.Intensity = 0.8f;

	Light dirLight3 = {};
	dirLight3.Type = LIGHT_TYPE_DIRECTIONAL;
	dirLight3.Direction = XMFLOAT3(-1, 1, 0);
	dirLight3.Color = XMFLOAT3(0.1f, 0.1f, 1.0f);
	dirLight3.Intensity = 0.8f;

	Light pointLight1 = {};
	pointLight1.Type = LIGHT_TYPE_POINT;
	pointLight1.Position = XMFLOAT3(-1.5f, 0, 0);
	pointLight1.Color = XMFLOAT3(1, 1, 1);
	pointLight1.Intensity = 0.5f;
	pointLight1.Range = 8.0f;

	Light pointLight2 = {};
	pointLight2.Type = LIGHT_TYPE_POINT;
	pointLight2.Position = XMFLOAT3(1.5f, 0, 0);
	pointLight2.Color = XMFLOAT3(1, 1, 1);
	pointLight2.Intensity = 0.3f;
	pointLight2.Range = 12.0f;

	lights.push_back(dirLight1);
	lights.push_back(dirLight2);
	lights.push_back(dirLight3);
	lights.push_back(pointLight1);
	lights.push_back(pointLight2);

	// Create Shadow Map Resources
	CreateShadowMapResources();
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
	skyVS = std::make_shared<SimpleVertexShader>(Graphics::Device, 
		Graphics::Context, FixPath(L"SkyVertexShader.cso").c_str());
	skyPS = std::make_shared<SimplePixelShader>(Graphics::Device, 
		Graphics::Context, FixPath(L"SkyPixelShader.cso").c_str());
	shadowVS = std::make_shared<SimpleVertexShader>(Graphics::Device, 
		Graphics::Context, FixPath(L"ShadowVS.cso").c_str());
	ppVS = std::make_shared<SimpleVertexShader>(Graphics::Device,
		Graphics::Context, FixPath(L"PostProcessVS.cso").c_str());
	ppPS = std::make_shared<SimplePixelShader>(Graphics::Device,
		Graphics::Context, FixPath(L"PostProcessBlurPS.cso").c_str());
	cappPS = std::make_shared<SimplePixelShader>(Graphics::Device,
		Graphics::Context, FixPath(L"PostProcessChromaticAberationPS.cso").c_str());

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

	// Create Sky object
	skybox = std::make_shared<Sky>(
		cubeMesh,
		sampler,
		skyVS,
		skyPS,
		FixPath(L"../../Assets/Skyboxes/Clouds Pink/right.png").c_str(),
		FixPath(L"../../Assets/Skyboxes/Clouds Pink/left.png").c_str(),
		FixPath(L"../../Assets/Skyboxes/Clouds Pink/up.png").c_str(),
		FixPath(L"../../Assets/Skyboxes/Clouds Pink/down.png").c_str(),
		FixPath(L"../../Assets/Skyboxes/Clouds Pink/front.png").c_str(),
		FixPath(L"../../Assets/Skyboxes/Clouds Pink/back.png").c_str()
		);

	// Load textures
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> flatNormalsSRV;

	CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(),
		FixPath(L"../../Assets/Textures/flat_normals.png").c_str(), 0, flatNormalsSRV.GetAddressOf());

	// Rock
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> rockSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> rockNormalSRV;

	CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(),
		FixPath(L"../../Assets/Textures/rock.png").c_str(), 0, rockSRV.GetAddressOf());
	CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(),
		FixPath(L"../../Assets/Textures/rock_normals.png").c_str(), 0, rockNormalSRV.GetAddressOf());

	// Scratched Surface
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> scratchedSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> scratchedNormalSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> scratchedRoughnessSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> scratchedMetalnessSRV;

	CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(),
		FixPath(L"../../Assets/Textures/scratched_albedo.png").c_str(), 0, scratchedSRV.GetAddressOf());
	CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(),
		FixPath(L"../../Assets/Textures/scratched_normals.png").c_str(), 0, scratchedNormalSRV.GetAddressOf());
	CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(),
		FixPath(L"../../Assets/Textures/scratched_roughness.png").c_str(), 0, scratchedRoughnessSRV.GetAddressOf());
	CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(),
		FixPath(L"../../Assets/Textures/scratched_metal.png").c_str(), 0, scratchedMetalnessSRV.GetAddressOf());

	// Wood Surface
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> woodSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> woodNormalSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> woodRoughnessSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> woodMetalnessSRV;

	CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(),
		FixPath(L"../../Assets/Textures/wood_albedo.png").c_str(), 0, woodSRV.GetAddressOf());
	CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(),
		FixPath(L"../../Assets/Textures/wood_normals.png").c_str(), 0, woodNormalSRV.GetAddressOf());
	CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(),
		FixPath(L"../../Assets/Textures/wood_roughness.png").c_str(), 0, woodRoughnessSRV.GetAddressOf());
	CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(),
		FixPath(L"../../Assets/Textures/wood_metal.png").c_str(), 0, woodMetalnessSRV.GetAddressOf());

	// Create materials before creating entities
	shared_ptr<Material> matWhite = make_shared<Material>(vertexShader, pixelShader, XMFLOAT3(1, 1, 1), 0.5f, 1.0f, 0.0f);
	//shared_ptr<Material> matPurple = make_shared<Material>(vertexShader, pixelShader, XMFLOAT3(0.8f, 0, 0.8f), 0.5f);
	//shared_ptr<Material> matOrange = make_shared<Material>(vertexShader, pixelShader, XMFLOAT3(1.0f, 0.5f, 0), 0.5f);
	shared_ptr<Material> matUV = make_shared<Material>(vertexShader, uvPS, XMFLOAT3(0, 0, 0), 0.5f, 1.0f, 0.0f);
	shared_ptr<Material> matNormal = make_shared<Material>(vertexShader, normalPS, XMFLOAT3(0, 0, 0), 0.5f, 1.0f, 0.0f);
	shared_ptr<Material> matCustom = make_shared<Material>(vertexShader, customPS, XMFLOAT3(1, 1, 1), 0.5f, 1.0f, 0.0f);
	shared_ptr<Material> matRocks = make_shared<Material>(vertexShader, pixelShader, XMFLOAT3(1, 1, 1), 0.5f, 1.0f, 0.0f);
	shared_ptr<Material> matScratched = make_shared<Material>(vertexShader, pixelShader, XMFLOAT3(1, 1, 1), 0.5f, 1.0f, 0.0f);
	shared_ptr<Material> matWood = make_shared<Material>(vertexShader, pixelShader, XMFLOAT3(1, 1, 1), 0.5f, 1.0f, 0.0f);

	// Add Textures/Samplers to Materials
	matRocks->AddTextureSRV("Albedo", rockSRV);
	matRocks->AddTextureSRV("NormalMap", rockNormalSRV);
	matRocks->AddSampler("BasicSampler", sampler);

	matScratched->AddTextureSRV("Albedo", scratchedSRV);
	matScratched->AddTextureSRV("NormalMap", scratchedNormalSRV);
	matScratched->AddTextureSRV("RoughnessMap", scratchedRoughnessSRV);
	matScratched->AddTextureSRV("MetalnessMap", scratchedMetalnessSRV);
	matScratched->AddSampler("BasicSampler", sampler);

	matWood->AddTextureSRV("Albedo", woodSRV);
	matWood->AddTextureSRV("NormalMap", woodNormalSRV);
	matWood->AddTextureSRV("RoughnessMap", woodRoughnessSRV);
	matWood->AddTextureSRV("MetalnessMap", woodMetalnessSRV);
	matWood->AddSampler("BasicSampler", sampler);

	// Add material objects to vector
	materials.push_back(matWhite);
	//materials.push_back(matPurple);
	//materials.push_back(matOrange);
	materials.push_back(matUV);
	materials.push_back(matNormal);
	materials.push_back(matRocks);
	materials.push_back(matScratched);
	materials.push_back(matWood);

	// Make game entities
	std::shared_ptr<Entity> entity1 = std::make_shared<Entity>(cubeMesh, matWood);
	std::shared_ptr<Entity> entity2 = std::make_shared<Entity>(cubeMesh, matScratched);
	std::shared_ptr<Entity> entity3 = std::make_shared<Entity>(helixMesh, matScratched);
	std::shared_ptr<Entity> entity4 = std::make_shared<Entity>(sphereMesh, matScratched);
	std::shared_ptr<Entity> entity5 = std::make_shared<Entity>(torusMesh, matWood);
	std::shared_ptr<Entity> entity6 = std::make_shared<Entity>(quadMesh, matScratched);
	std::shared_ptr<Entity> entity7 = std::make_shared<Entity>(quadDoubleSideMesh, matScratched);

	// Spread out some of the entities so they aren't all on top of one another
	entity1->GetTransform()->MoveAbsolute(0, -3, 0);
	entity1->GetTransform()->SetScale(25, 1, 25);
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

	// Post Process Setup
	CreateResizePostProcess();

	// Sampler state for post processing
	D3D11_SAMPLER_DESC ppSampDesc = {};
	ppSampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	ppSampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	ppSampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	ppSampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	ppSampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	Graphics::Device->CreateSamplerState(&ppSampDesc, ppSampler.GetAddressOf());
}

// Create Post Process Resources 
// Called after each window resize
void Game::CreateResizePostProcess()
{
	// Reset SRV and RTV views if they exist
	ppSRV.Reset();
	ppRTV.Reset();
	cappSRV.Reset();
	cappRTV.Reset();

	// Describe the texture we're creating
	D3D11_TEXTURE2D_DESC textureDesc = {};
	textureDesc.Width = Window::Width();
	textureDesc.Height = Window::Height();
	textureDesc.ArraySize = 1;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDesc.MipLevels = 1;
	textureDesc.MiscFlags = 0;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;

	// Create the resource (no need to track it after the views are created below)
	Microsoft::WRL::ComPtr<ID3D11Texture2D> ppTexture;
	Graphics::Device->CreateTexture2D(&textureDesc, 0, ppTexture.GetAddressOf());

	// Create the Render Target View
	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.Format = textureDesc.Format;
	rtvDesc.Texture2D.MipSlice = 0;
	rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	Graphics::Device->CreateRenderTargetView(
		ppTexture.Get(),
		&rtvDesc,
		ppRTV.ReleaseAndGetAddressOf());

	// Create the Shader Resource View
	// By passing it a null description for the SRV, we
	// get a "default" SRV that has access to the entire resource
	Graphics::Device->CreateShaderResourceView(
		ppTexture.Get(),
		0,
		ppSRV.ReleaseAndGetAddressOf());



	// Repeat for chromatic aberation

	// Describe the texture we're creating
	D3D11_TEXTURE2D_DESC textureDescCA = {};
	textureDescCA.Width = Window::Width();
	textureDescCA.Height = Window::Height();
	textureDescCA.ArraySize = 1;
	textureDescCA.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	textureDescCA.CPUAccessFlags = 0;
	textureDescCA.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDescCA.MipLevels = 1;
	textureDescCA.MiscFlags = 0;
	textureDescCA.SampleDesc.Count = 1;
	textureDescCA.SampleDesc.Quality = 0;
	textureDescCA.Usage = D3D11_USAGE_DEFAULT;

	// Create the resource (no need to track it after the views are created below)
	Microsoft::WRL::ComPtr<ID3D11Texture2D> cappTexture;
	Graphics::Device->CreateTexture2D(&textureDescCA, 0, cappTexture.GetAddressOf());

	// Create the Render Target View
	D3D11_RENDER_TARGET_VIEW_DESC rtvDescCA = {};
	rtvDescCA.Format = textureDescCA.Format;
	rtvDescCA.Texture2D.MipSlice = 0;
	rtvDescCA.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	Graphics::Device->CreateRenderTargetView(
		cappTexture.Get(),
		&rtvDescCA,
		cappRTV.ReleaseAndGetAddressOf());

	// Create the Shader Resource View
	// By passing it a null description for the SRV, we
	// get a "default" SRV that has access to the entire resource
	Graphics::Device->CreateShaderResourceView(
		cappTexture.Get(),
		0,
		cappSRV.ReleaseAndGetAddressOf());
}

void Game::CreateShadowMapResources()
{
	// Create the actual texture that will be the shadow map
	shadowMapResolution = 1024;

	D3D11_TEXTURE2D_DESC shadowDesc = {};
	shadowDesc.Width = shadowMapResolution; // Ideally a power of 2 (like 1024)
	shadowDesc.Height = shadowMapResolution; // Ideally a power of 2 (like 1024)
	shadowDesc.ArraySize = 1;
	shadowDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	shadowDesc.CPUAccessFlags = 0;
	shadowDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	shadowDesc.MipLevels = 1;
	shadowDesc.MiscFlags = 0;
	shadowDesc.SampleDesc.Count = 1;
	shadowDesc.SampleDesc.Quality = 0;
	shadowDesc.Usage = D3D11_USAGE_DEFAULT;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> shadowTexture;
	Graphics::Device->CreateTexture2D(&shadowDesc, 0, shadowTexture.GetAddressOf());

	// Create the depth/stencil view
	D3D11_DEPTH_STENCIL_VIEW_DESC shadowDSDesc = {};
	shadowDSDesc.Format = DXGI_FORMAT_D32_FLOAT;
	shadowDSDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	shadowDSDesc.Texture2D.MipSlice = 0;
	Graphics::Device->CreateDepthStencilView(
		shadowTexture.Get(),
		&shadowDSDesc,
		shadowDSV.GetAddressOf());

	// Create the SRV for the shadow map
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.MostDetailedMip = 0;
	Graphics::Device->CreateShaderResourceView(
		shadowTexture.Get(),
		&srvDesc,
		shadowSRV.GetAddressOf());

	// Create Light Matrices
	XMVECTOR lightDirection = XMVectorSet(1, -1, 1, 0);

	// View matrix
	XMMATRIX lightView = XMMatrixLookToLH(
		lightDirection * -20, // Position: "Backing up" 20 units from origin
		lightDirection, // Direction: light's direction
		XMVectorSet(0, 1, 0, 0)); // Up: World up vector (Y axis)
	XMStoreFloat4x4(&lightViewMatrix, lightView);

	// Projection 
	float lightProjectionSize = 15.0f; // Tweak for your scene!
	XMMATRIX lightProjection = XMMatrixOrthographicLH(
		lightProjectionSize,
		lightProjectionSize,
		1.0f,
		100.0f);
	XMStoreFloat4x4(&lightProjectionMatrix, lightProjection);

	// Create Rasterizer 
	D3D11_RASTERIZER_DESC shadowRastDesc = {};
	shadowRastDesc.FillMode = D3D11_FILL_SOLID;
	shadowRastDesc.CullMode = D3D11_CULL_BACK;
	shadowRastDesc.DepthClipEnable = true;
	shadowRastDesc.DepthBias = 1000; // Min. precision units, not world units!
	shadowRastDesc.SlopeScaledDepthBias = 1.0f; // Bias more based on slope
	Graphics::Device->CreateRasterizerState(&shadowRastDesc, &shadowRasterizer);

	// Create Sampler for the shadow
	D3D11_SAMPLER_DESC shadowSampDesc = {};
	shadowSampDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
	shadowSampDesc.ComparisonFunc = D3D11_COMPARISON_LESS;
	shadowSampDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	shadowSampDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	shadowSampDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
	shadowSampDesc.BorderColor[0] = 1.0f; // Only need the first component
	Graphics::Device->CreateSamplerState(&shadowSampDesc, &shadowSampler);
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

	// Re-Create Post Process Resourcees
	if (Graphics::Device)
		CreateResizePostProcess();
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
		//e->GetTransform()->Rotate(0, deltaTime, 0);
	}

	float move = sin(totalTime) * 2.0f;

	entities[1]->GetTransform()->SetPosition(-4, move, 0);
	entities[2]->GetTransform()->SetPosition(0, move, 0);
	entities[3]->GetTransform()->SetPosition(4, move, 0);
	
	// entities[0]->GetTransform()->Rotate(deltaTime, 0, deltaTime);
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

	// Render shadow map first before drawing geometry
	RenderShadowMap();

	// Post Processing Pre Draw ===============
	// Clear render targets
	const float clearColor[4] = { 0, 0, 0, 1 };
	Graphics::Context->ClearRenderTargetView(ppRTV.Get(), clearColor);

	// Swap Active Render Target
	Graphics::Context->OMSetRenderTargets(1, ppRTV.GetAddressOf(), Graphics::DepthBufferDSV.Get());

	// DRAW geometry
	// Loop through and draw every mesh
	{
		for (std::shared_ptr<Entity> e : entities) {
			std::shared_ptr<SimpleVertexShader> vs = e->GetMaterial()->GetVertexShader();
			vs->SetMatrix4x4("lightView", lightViewMatrix);
			vs->SetMatrix4x4("lightProjection", lightProjectionMatrix);

			// Set pixel shader data
			std::shared_ptr<SimplePixelShader> ps = e->GetMaterial()->GetPixelShader();
			ps->SetData("lights", &lights[0], sizeof(Light) * (int)lights.size());

			ps->SetShaderResourceView("ShadowMap", shadowSRV);
			ps->SetSamplerState("ShadowSampler", shadowSampler);

			// Draw the entity
			e->Draw(cameras[activeCam]);
		}
	}

	// Draw skybox
	skybox->Draw(cameras[activeCam]);

	// Post Processing Post Draw ===========
	// Restore Back Buffer
	Graphics::Context->OMSetRenderTargets(1, cappRTV.GetAddressOf(), 0);

	// Activate shaders and bind resources
	ppVS->SetShader();
	ppPS->SetShader();
	ppPS->SetShaderResourceView("Pixels", ppSRV.Get());
	ppPS->SetSamplerState("ClampSampler", ppSampler.Get());

	// Also set any required cbuffer data
	ppPS->SetFloat("pixelWidth", 1.0f / Window::Width());
	ppPS->SetFloat("pixelHeight", 1.0f / Window::Height());
	ppPS->SetInt("blurRadius", blurDistance);
	ppPS->CopyAllBufferData();

	Graphics::Context->Draw(3, 0); // Draw exactly 3 vertices (one triangle)

	// Repeat for Chromatic Aberation
	// Restore Back Buffer
	Graphics::Context->OMSetRenderTargets(1, Graphics::BackBufferRTV.GetAddressOf(), 0);

	// Activate shaders and bind resources
	cappPS->SetShader();
	cappPS->SetShaderResourceView("Pixels", cappSRV.Get());
	cappPS->SetSamplerState("ClampSampler", ppSampler.Get());

	// Also set any required cbuffer data
	cappPS->SetFloat("pixelWidth", 1.0f / Window::Width());
	cappPS->SetFloat("pixelHeight", 1.0f / Window::Height());
	cappPS->SetFloat("redOffset", redOffset);
	cappPS->SetFloat("greenOffset", greenOffset);
	cappPS->SetFloat("blueOffset", blueOffset);
	cappPS->CopyAllBufferData();

	Graphics::Context->Draw(3, 0); // Draw exactly 3 vertices (one triangle)

	
	// Unbind shadow map (and any other SRVs)
	ID3D11ShaderResourceView* nullSRVs[128] = {};
	Graphics::Context->PSSetShaderResources(0, 128, nullSRVs);

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

// Render Shadow Map from light's perspective
void Game::RenderShadowMap()
{
	// Clear shadow map
	Graphics::Context->ClearDepthStencilView(shadowDSV.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

	// Set shadow map as current depth buffer
	ID3D11RenderTargetView* nullRTV{};
	Graphics::Context->OMSetRenderTargets(1, &nullRTV, shadowDSV.Get());

	// Activate Rasterizer state
	Graphics::Context->RSSetState(shadowRasterizer.Get());

	// Deactivate pixel shader (dont need to draw anything to screen)
	Graphics::Context->PSSetShader(0, 0, 0);

	// Change viewport
	D3D11_VIEWPORT viewport = {};
	viewport.Width = (float)shadowMapResolution;
	viewport.Height = (float)shadowMapResolution;
	viewport.MaxDepth = 1.0f;
	Graphics::Context->RSSetViewports(1, &viewport);

	// Activate shadow vertex shader
	shadowVS->SetShader();
	shadowVS->SetMatrix4x4("view", lightViewMatrix);
	shadowVS->SetMatrix4x4("projection", lightProjectionMatrix);

	// Loop and draw all entities
	for (auto& e : entities)
	{
		shadowVS->SetMatrix4x4("world", e->GetTransform()->GetWorldMatrix());
		shadowVS->CopyAllBufferData();
		// Draw the mesh directly to avoid the entity's material
		// Note: Your code may differ significantly here!
		e->GetMesh()->Draw();
	}

	// Reset pipeline back for regular Drawing
	viewport.Width = (float)Window::Width();
	viewport.Height = (float)Window::Height();
	Graphics::Context->RSSetViewports(1, &viewport);
	Graphics::Context->OMSetRenderTargets(
		1,
		Graphics::BackBufferRTV.GetAddressOf(),
		Graphics::DepthBufferDSV.Get());
	Graphics::Context->RSSetState(0);
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
	
	ImGui::Spacing();
	
	ImGui::SliderInt("Blur Distance", &blurDistance, 0, 25);

	ImGui::Spacing();

	ImGui::SliderFloat("Red Offset", &redOffset, -0.01f, 0.01f);
	ImGui::SliderFloat("Green Offset", &greenOffset, -0.01f, 0.01f);
	ImGui::SliderFloat("Blue Offset", &blueOffset, -0.01f, 0.01f);

	ImGui::Spacing();

	ImGui::Image(shadowSRV.Get(), ImVec2(512, 512));

	
	ImGui::End(); // Ends the current window
}


