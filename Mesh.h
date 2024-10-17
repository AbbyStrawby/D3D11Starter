#pragma once

#include <d3d11.h>
#include <wrl/client.h>

#include "Vertex.h"

class Mesh 
{
private:

	// Buffers to store mesh geometry data
	Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;

	// Buffer relevant fields
	unsigned int numVertices;		// How many vertices are in the mesh's vertex buffer
	unsigned int numIndices;			// How many indices are in the mesh's index buffer	

	// UI related fields
	const char* name;

	// Helper functions
	void CreateBuffers(Vertex* vertArray, size_t numVertices, unsigned int* indexArray, size_t numIndices);

public:

	// Con/destructor
	Mesh(const char* name, Vertex* vertArray, size_t numVertices, unsigned int* indexArray, size_t numIndices);
	Mesh(const char* name, const char* filename);
	~Mesh();

	// Access ComPtrs
	Microsoft::WRL::ComPtr<ID3D11Buffer> GetVertexBuffer();
	Microsoft::WRL::ComPtr<ID3D11Buffer> GetIndexBuffer();

	// Access Buffer Fields
	int GetVertexCount();
	int GetIndexCount();

	// Access UI Field
	const char* GetName();

	// Draw
	void Draw();


};