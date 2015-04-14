#include "terrainshaderclass.h"

TerrainShaderClass::TerrainShaderClass()
	: m_vertexShader(0)
	, m_pixelShader(0)
	, m_layout(0)
	, m_sampleState(0)
	, m_matrixBuffer(0)
	, m_lightBuffer(0)
{}
TerrainShaderClass::TerrainShaderClass(const TerrainShaderClass& other)
{}

TerrainShaderClass::~TerrainShaderClass()
{}

bool TerrainShaderClass::Initialize(ID3D11Device* device, HWND hwnd)
{
	bool result;
	result = InitializeShader(device, hwnd, L"../Engine/shader/terrain.vs", L"../Engine/shader/terrain.ps");
	return result;
}

void TerrainShaderClass::Shutdown()
{
	ShutdownShader();
	return;
}

bool TerrainShaderClass::Render(ID3D11DeviceContext* deviceContext, int indexCount, D3DXMATRIX worldMatrix, D3DXMATRIX viewMatrix, D3DXMATRIX projectionMatrix,
	D3DXVECTOR4 ambientColor, D3DXVECTOR4 diffuseColor, D3DXVECTOR3 lightDirection)
{
	bool result;
	result = SetShaderParameters(deviceContext, worldMatrix, viewMatrix, projectionMatrix, ambientColor, diffuseColor, lightDirection);
	if(!result)
		return false;

	RenderShader(deviceContext, indexCount);
	return true;
}


bool TerrainShaderClass::InitializeShader(ID3D11Device* device, HWND hwnd, WCHAR* vsFilename, WCHAR* psFilename)
{
	//init pointer for this function with null
	ID3D10Blob* errorMessage;
	errorMessage = 0;

	ID3D10Blob* vertexShaderBuffer;
	vertexShaderBuffer = 0;

	//compile shader programs into buffers
	HRESULT result;
	result = D3DX11CompileFromFile(vsFilename, NULL, NULL, "TerrainVertexShader", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, NULL,
		&vertexShaderBuffer, &errorMessage, NULL);

	//display error messages if necessary
	if(FAILED(result))
	{
		if(errorMessage)
			OutputShaderErrorMessage(errorMessage, hwnd, vsFilename);

		//no error message --> shader file is missing
		else
			MessageBox(hwnd, vsFilename, L"Missing Shader File", MB_OK);

		return false;
	}

	//compile pixel shader
	ID3D10Blob* pixelShaderBuffer;
	pixelShaderBuffer = 0;
	result = D3DX11CompileFromFile(psFilename, NULL, NULL, "TerrainPixelShader", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, NULL,
		&pixelShaderBuffer, &errorMessage, NULL);

	//display error messages if necessary
	if(FAILED(result))
	{
		if(errorMessage)
			OutputShaderErrorMessage(errorMessage, hwnd, psFilename);

		//no error message --> shader file is missing
		else
			MessageBox(hwnd, psFilename, L"Missing Shader File", MB_OK);

		return false;
	}
	

	//create vertex shader object
	result = device->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), NULL, &m_vertexShader);
	if(FAILED(result))
		return false;

	//create pixel shader object
	result = device->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(), pixelShaderBuffer->GetBufferSize(), NULL, &m_pixelShader);
	if(FAILED(result))
		return false;
	

	//create layout of vertext data which is processed by the shader
	D3D11_INPUT_ELEMENT_DESC polygonLayout[2];
	polygonLayout[0].SemanticName = "POSITION";
	polygonLayout[0].SemanticIndex = 0;
	polygonLayout[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	polygonLayout[0].InputSlot = 0;
	polygonLayout[0].AlignedByteOffset = 0;
	polygonLayout[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[0].InstanceDataStepRate = 0;

	polygonLayout[1].SemanticName = "NORMAL";
	polygonLayout[1].SemanticIndex = 0;
	polygonLayout[1].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	polygonLayout[1].InputSlot = 0;
	polygonLayout[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	polygonLayout[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[1].InstanceDataStepRate = 0;

	//get size of layout description in order to create the input layout
	unsigned int numElements;
	numElements = sizeof(polygonLayout) / sizeof(polygonLayout[0]);

	//create vertex input layout
	result = device->CreateInputLayout(polygonLayout, numElements, vertexShaderBuffer->GetBufferPointer(),
		vertexShaderBuffer->GetBufferSize(), &m_layout);
	if(FAILED(result))
		return false;

	//release temporary buffers
	vertexShaderBuffer->Release();
	vertexShaderBuffer = 0;

	pixelShaderBuffer->Release();
	pixelShaderBuffer = 0;

	//create smapler desc- filter is most important
	D3D11_SAMPLER_DESC samplerDesc;
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 1;

	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.BorderColor[0] = 0;
	samplerDesc.BorderColor[1] = 0;
	samplerDesc.BorderColor[2] = 0;
	samplerDesc.BorderColor[3] = 0;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	// Create the texture sampler state.
	result = device->CreateSamplerState(&samplerDesc, &m_sampleState);
	if(FAILED(result))
		return false;

	//create constant buffer
	D3D11_BUFFER_DESC matrixBufferDesc;
	matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
	matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	matrixBufferDesc.MiscFlags = 0;
	matrixBufferDesc.StructureByteStride = 0;

	result = device->CreateBuffer(&matrixBufferDesc, NULL, &m_matrixBuffer);
	if(FAILED(result))
		return false;

	D3D11_BUFFER_DESC lightBufferDesc;
	lightBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	lightBufferDesc.ByteWidth = sizeof(LightBufferType);
	lightBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	lightBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	lightBufferDesc.MiscFlags = 0;
	lightBufferDesc.StructureByteStride = 0;

	// Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
	result = device->CreateBuffer(&lightBufferDesc, NULL, &m_lightBuffer);
	if(FAILED(result))
		return false;

	return true;
}

void TerrainShaderClass::ShutdownShader()
{
	if(m_lightBuffer)
	{
		m_lightBuffer->Release();
		m_lightBuffer = 0;
	}

	if(m_matrixBuffer)
	{
		m_matrixBuffer->Release();
		m_matrixBuffer = 0;
	}

	if(m_sampleState)
	{
		m_sampleState->Release();
		m_sampleState = 0;
	}

	if(m_layout)
	{
		m_layout->Release();
		m_layout = 0;
	}

	if(m_pixelShader)
	{
		m_pixelShader->Release();
		m_pixelShader = 0;
	}

	if(m_vertexShader)
	{
		m_vertexShader->Release();
		m_vertexShader = 0;
	}

	return;
}

void TerrainShaderClass::OutputShaderErrorMessage(ID3D10Blob* errorMessage, HWND hwnd, WCHAR* shaderFilename)
{
	char* compileErrors;
	compileErrors = (char*)(errorMessage->GetBufferPointer());

	//get length of message
	unsigned long bufferSize;
	bufferSize = errorMessage->GetBufferSize();

	//write the error message into a file
	ofstream fout;
	fout.open("shader-error.txt");

	for(unsigned long i = 0; i < bufferSize; i++)
	{
		fout << compileErrors[i];
	}

	fout.close();

	//release error message
	errorMessage->Release();
	errorMessage = 0;

	MessageBox(hwnd, L"Error compiling shader. Check shader-error.txt to read the message.", shaderFilename, MB_OK);

	return;
}

bool TerrainShaderClass::SetShaderParameters(ID3D11DeviceContext* deviceContext, D3DXMATRIX worldMatrix, D3DXMATRIX viewMatrix, D3DXMATRIX projectionMatrix,
	D3DXVECTOR4 ambientColor, D3DXVECTOR4 diffuseColor, D3DXVECTOR3 lightDirection)
{
	// Transpose the matrices to prepare them for the shader.
	D3DXMatrixTranspose(&worldMatrix, &worldMatrix);
	D3DXMatrixTranspose(&viewMatrix, &viewMatrix);
	D3DXMatrixTranspose(&projectionMatrix, &projectionMatrix);

	//lock the constant buffer
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	result = deviceContext->Map(m_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if(FAILED(result))
		return false;

	//get pointer and copy matrices into constant buffer
	MatrixBufferType* dataPtr;
	dataPtr = (MatrixBufferType*)mappedResource.pData;
	dataPtr->world = worldMatrix;
	dataPtr->view = viewMatrix;
	dataPtr->projection = projectionMatrix;

	//unlock constant buffer
	deviceContext->Unmap(m_matrixBuffer, 0);
	
	//set constant buffer in vertex shader with updated values
	unsigned int bufferNumber;
	bufferNumber = 0;
	deviceContext->VSSetConstantBuffers(bufferNumber, 1, &m_matrixBuffer);

	//lock light constant buffer and write into it
	result = deviceContext->Map(m_lightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if(FAILED(result))
		return false;

	LightBufferType* dataPtr2;
	dataPtr2 = (LightBufferType*)mappedResource.pData;
	dataPtr2->ambientColor = ambientColor;
	dataPtr2->diffuseColor = diffuseColor;
	dataPtr2->lightDirection = lightDirection;
	dataPtr2->padding = 0.0f;

	//unlock the light buffer
	deviceContext->Unmap(m_lightBuffer, 0);

	//set light buffer with updated values
	bufferNumber = 0;
	deviceContext->PSSetConstantBuffers(bufferNumber, 1, &m_lightBuffer);

	return true;
}

void TerrainShaderClass::RenderShader(ID3D11DeviceContext* deviceContext, int indexCount)
{
	//set vertex input layout
	deviceContext->IASetInputLayout(m_layout);

	//set vertex and pixel shader
	deviceContext->VSSetShader(m_vertexShader, NULL, 0);
	deviceContext->PSSetShader(m_pixelShader, NULL, 0);

	//set sampler state
	deviceContext->PSSetSamplers(0, 1, &m_sampleState);

	//render
	deviceContext->DrawIndexed(indexCount, 0, 0);

	return;
}