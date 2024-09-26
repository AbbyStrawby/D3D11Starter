#include "Game.h"
#include "Graphics.h"
#include "Vertex.h"
#include "Input.h"
#include "PathHelpers.h"
#include "Window.h"
#include "BufferStructs.h"

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

// --------------------------------------------------------
// Called once per program, after the window and graphics API
// are initialized but before the game loop begins
// --------------------------------------------------------
void Game::Initialize()
{
	// Helper methods for loading shaders, creating some basic
	// geometry to draw and some simple camera matrices.
	//  - You'll be expanding and/or replacing these later
	LoadShaders();
	CreateGeometry();

	// Set initial graphics API state
	//  - These settings persist until we change them
	//  - Some of these, like the primitive topology & input layout, probably won't change
	//  - Others, like setting shaders, will need to be moved elsewhere later
	{
		// Tell the input assembler (IA) stage of the pipeline what kind of
		// geometric primitives (points, lines or triangles) we want to draw.  
		// Essentially: "What kind of shape should the GPU draw with our vertices?"
		Graphics::Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		// Ensure the pipeline knows how to interpret all the numbers stored in
		// the vertex buffer. For this course, all of your vertices will probably
		// have the same layout, so we can just set this once at startup.
		Graphics::Context->IASetInputLayout(inputLayout.Get());

		// Set the active vertex and pixel shaders
		//  - Once you start applying different shaders to different objects,
		//    these calls will need to happen multiple times per frame
		Graphics::Context->VSSetShader(vertexShader.Get(), 0, 0);
		Graphics::Context->PSSetShader(pixelShader.Get(), 0, 0);
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

	// Create a constant buffer for the vertex shader data

	// Calculate size of the buffer
	unsigned int size = sizeof(VertexShaderData);
	size = (size + 15) / 16 * 16;

	// Create constant buffer descripition
	D3D11_BUFFER_DESC cbDesc = {};
	cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;	// We want a constant buffer
	cbDesc.ByteWidth = size;						// Calculated size value, multiple of 16
	cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;	// We want to write data to this each frame
	cbDesc.Usage = D3D11_USAGE_DYNAMIC;				// Contents will be changed after creation

	Graphics::Device->CreateBuffer(&cbDesc, 0, vsConstantBuffer.GetAddressOf());

	// Bind the constant buffer to the correct slot for the vertex shader to use
	Graphics::Context->VSSetConstantBuffers(
		0,									// Which register to bind to
		1,									// How many buffers we're setting
		vsConstantBuffer.GetAddressOf()		// Address of buffer(s) (just one since we're only using one)
	);	
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
// Loads shaders from compiled shader object (.cso) files
// and also created the Input Layout that describes our 
// vertex data to the rendering pipeline. 
// - Input Layout creation is done here because it must 
//    be verified against vertex shader byte code
// - We'll have that byte code already loaded below
// --------------------------------------------------------
void Game::LoadShaders()
{
	// BLOBs (or Binary Large OBjects) for reading raw data from external files
	// - This is a simplified way of handling big chunks of external data
	// - Literally just a big array of bytes read from a file
	ID3DBlob* pixelShaderBlob;
	ID3DBlob* vertexShaderBlob;

	// Loading shaders
	//  - Visual Studio will compile our shaders at build time
	//  - They are saved as .cso (Compiled Shader Object) files
	//  - We need to load them when the application starts
	{
		// Read our compiled shader code files into blobs
		// - Essentially just "open the file and plop its contents here"
		// - Uses the custom FixPath() helper from Helpers.h to ensure relative paths
		// - Note the "L" before the string - this tells the compiler the string uses wide characters
		D3DReadFileToBlob(FixPath(L"PixelShader.cso").c_str(), &pixelShaderBlob);
		D3DReadFileToBlob(FixPath(L"VertexShader.cso").c_str(), &vertexShaderBlob);

		// Create the actual Direct3D shaders on the GPU
		Graphics::Device->CreatePixelShader(
			pixelShaderBlob->GetBufferPointer(),	// Pointer to blob's contents
			pixelShaderBlob->GetBufferSize(),		// How big is that data?
			0,										// No classes in this shader
			pixelShader.GetAddressOf());			// Address of the ID3D11PixelShader pointer

		Graphics::Device->CreateVertexShader(
			vertexShaderBlob->GetBufferPointer(),	// Get a pointer to the blob's contents
			vertexShaderBlob->GetBufferSize(),		// How big is that data?
			0,										// No classes in this shader
			vertexShader.GetAddressOf());			// The address of the ID3D11VertexShader pointer
	}

	// Create an input layout 
	//  - This describes the layout of data sent to a vertex shader
	//  - In other words, it describes how to interpret data (numbers) in a vertex buffer
	//  - Doing this NOW because it requires a vertex shader's byte code to verify against!
	//  - Luckily, we already have that loaded (the vertex shader blob above)
	{
		D3D11_INPUT_ELEMENT_DESC inputElements[2] = {};

		// Set up the first element - a position, which is 3 float values
		inputElements[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;				// Most formats are described as color channels; really it just means "Three 32-bit floats"
		inputElements[0].SemanticName = "POSITION";							// This is "POSITION" - needs to match the semantics in our vertex shader input!
		inputElements[0].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;	// How far into the vertex is this?  Assume it's after the previous element

		// Set up the second element - a color, which is 4 more float values
		inputElements[1].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;			// 4x 32-bit floats
		inputElements[1].SemanticName = "COLOR";							// Match our vertex shader input!
		inputElements[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;	// After the previous element

		// Create the input layout, verifying our description against actual shader code
		Graphics::Device->CreateInputLayout(
			inputElements,							// An array of descriptions
			2,										// How many elements in that array?
			vertexShaderBlob->GetBufferPointer(),	// Pointer to the code of a shader that uses this layout
			vertexShaderBlob->GetBufferSize(),		// Size of the shader code that uses this layout
			inputLayout.GetAddressOf());			// Address of the resulting ID3D11InputLayout pointer
	}
}


// --------------------------------------------------------
// Creates the geometry we're going to draw
// --------------------------------------------------------
void Game::CreateGeometry()
{
	// Create some temporary variables to represent colors
	// - Not necessary, just makes things more readable
	// In case you ever need to make more colors, heres the site you used: https://rgbcolorpicker.com/0-1
	XMFLOAT4 red = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
	XMFLOAT4 green = XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);
	XMFLOAT4 blue = XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f);
	XMFLOAT4 none = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	XMFLOAT4 pink = XMFLOAT4(1.0f, 0.4f, 0.75f, 0.0f);
	XMFLOAT4 plink = XMFLOAT4(0.75f, 0.4f, 0.75f, 0.0f);
	XMFLOAT4 purple = XMFLOAT4(0.9f, 0.2f, 0.9f, 0.0f);
	

	// Set up the vertices of the triangle we would like to draw
	// - We're going to copy this array, exactly as it exists in CPU memory
	//    over to a Direct3D-controlled data structure on the GPU (the vertex buffer)
	// - Note: Since we don't have a camera or really any concept of
	//    a "3d world" yet, we're simply describing positions within the
	//    bounds of how the rasterizer sees our screen: [-1 to +1] on X and Y
	// - This means (0,0) is at the very center of the screen.
	// - These are known as "Normalized Device Coordinates" or "Homogeneous 
	//    Screen Coords", which are ways to describe a position without
	//    knowing the exact size (in pixels) of the image/window/etc.  
	// - Long story short: Resizing the window also resizes the triangle,
	//    since we're describing the triangle in terms of the window itself
	
	// Mesh 1
	Vertex vertices1[] =
	{
		{ XMFLOAT3(+0.0f, +0.5f, +0.0f), red },
		{ XMFLOAT3(+0.5f, -0.5f, +0.0f), blue },
		{ XMFLOAT3(-0.5f, -0.5f, +0.0f), green },
	};
	// Set up indices, which tell us which vertices to use and in which order
	// - This is redundant for just 3 vertices, but will be more useful later
	// - Indices are technically not required if the vertices are in the buffer 
	//    in the correct order and each one will be used exactly once
	// - But just to see how it's done...
	unsigned int indices1[] = { 0, 1, 2 };

	// Mesh 2
	Vertex vertices2[] =
	{
		{ XMFLOAT3(-0.1f, +0.1f, +0.0f), red },
		{ XMFLOAT3(+0.1f, +0.1f, +0.0f), blue },
		{ XMFLOAT3(+0.1f, -0.1f, +0.0f), green },
		{ XMFLOAT3(-0.1f, -0.1f, +0.0f), none },
	};
	unsigned int indices2[] =
	{
		0, 1, 2,
		0, 2, 3
	};

	// Mesh 3
	Vertex vertices3[] =
	{
		{ XMFLOAT3(-0.1f, +0.30f, +0.0f), pink },	// -.1, .30
		{ XMFLOAT3(+0.0f, +0.15f, +0.0f), plink },	// 0, .15
		{ XMFLOAT3(-0.2f, +0.15f, +0.0f), plink },	// -.2, .15
		{ XMFLOAT3(+0.1f, +0.30f, +0.0f), pink },	// .1, .30
		{ XMFLOAT3(+0.2f, +0.15f, +0.0f), plink },	// .2, .15
		{ XMFLOAT3(+0.0f, -0.30f, +0.0f), purple },	// 0, -.30
	};
	unsigned int indices3[] =
	{
		0, 1, 2,
		3, 4, 1,
		2, 4, 5
	};

	// Make the mesh objects
	std::shared_ptr<Mesh> mesh1 = std::make_shared<Mesh>(
		"Triangle", vertices1, ARRAYSIZE(vertices1), indices1, ARRAYSIZE(indices1)
	);
	std::shared_ptr<Mesh> mesh2 = std::make_shared<Mesh>(
		"Quad", vertices2, ARRAYSIZE(vertices2), indices2, ARRAYSIZE(indices2)
	);
	std::shared_ptr<Mesh> mesh3 = std::make_shared<Mesh>(
		"Heart", vertices3, ARRAYSIZE(vertices3), indices3, ARRAYSIZE(indices3)
	);

	// Add mesh objects to the list
	meshes.push_back(mesh1);
	meshes.push_back(mesh2);
	meshes.push_back(mesh3);

	// Make game entities
	std::shared_ptr<Entity> entity1 = std::make_shared<Entity>(mesh1);
	std::shared_ptr<Entity> entity2 = std::make_shared<Entity>(mesh2);
	std::shared_ptr<Entity> entity3 = std::make_shared<Entity>(mesh3);
	std::shared_ptr<Entity> entity4 = std::make_shared<Entity>(mesh3);
	std::shared_ptr<Entity> entity5 = std::make_shared<Entity>(mesh3);

	// Spread out some of the entities so they aren't all on top of one another
	entity1->GetTransform()->MoveAbsolute(0, 0, 0.01f);
	entity2->GetTransform()->MoveAbsolute(0, -0.8f, 0);
	entity3->GetTransform()->MoveAbsolute(0.8f, -0.8f, 0);
	entity3->GetTransform()->Scale(0.5f);
	entity4->GetTransform()->MoveAbsolute(0, .4f, 0);
	entity5->GetTransform()->MoveAbsolute(0, .4f, 0);

	// Add entity objects to the list
	entities.push_back(entity1);
	entities.push_back(entity2);
	entities.push_back(entity3);
	entities.push_back(entity4);
	entities.push_back(entity5);
}


// --------------------------------------------------------
// Handle resizing to match the new window size
//  - Eventually, we'll want to update our 3D camera
// --------------------------------------------------------
void Game::OnResize()
{
}


// --------------------------------------------------------
// Update your game here - user input, move objects, AI, etc.
// --------------------------------------------------------
void Game::Update(float deltaTime, float totalTime)
{
	// Update the ImGui
	UIUpdate(deltaTime);

	// Build the custom UI
	BuildUI();

	// Example input checking: Quit if the escape key is pressed
	if (Input::KeyDown(VK_ESCAPE))
		Window::Quit();

	// Update Transformations
	float scaleSize = (float)sin(totalTime * 2) * 0.2f + 0.8f;
	entities[0]->GetTransform()->SetScale(scaleSize, scaleSize, scaleSize);
	entities[0]->GetTransform()->Rotate(0, 0, deltaTime);

	float moveDistance = (float)cos(totalTime) * 0.5f;
	entities[3]->GetTransform()->SetPosition(moveDistance, 0.6f, 0);
	entities[4]->GetTransform()->SetPosition(-moveDistance, 0.6f, 0);
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
		for (std::shared_ptr<Entity> m : entities) {
			m->Draw(vsConstantBuffer);
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
void Game::BuildUI() 
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

				//ImGui::DragFloat3("Position", &vsData.offset.x, 0.01f);

				ImGui::Spacing();

				ImGui::TreePop();
			}

			ImGui::PopID();
		}

		ImGui::TreePop();
	}

	

	
	ImGui::End(); // Ends the current window
}


