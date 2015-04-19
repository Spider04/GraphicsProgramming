#include "fontshaderclass.h"

FontShaderClass::FontShaderClass()
	: m_vertexShader(0)
	, m_pixelShader(0)
	, m_layout(0)
	, m_constantBuffer(0)
	, m_sampleState(0)
	, m_pixelBuffer(0)
{}
FontShaderClass::FontShaderClass(const FontShaderClass& other)
{
}

FontShaderClass::~FontShaderClass()
{
}


bool FontShaderClass::Initialize(ID3D11Device* device, HWND hwnd)
{
	//init vertex nd pixel shader
	bool result = false;
	result = InitializeShader(device, hwnd, L"../Engine/shader/font.vs", L"../Engine/shader/font.ps");
	return result;
}

bool FontShaderClass::InitializeShader(ID3D11Device* device, HWND hwnd, WCHAR* vsFilename, WCHAR* psFilename)
{
	//compile vertex shader
	ID3D10Blob* errorMessage = 0;
	ID3D10Blob* vertexShaderBuffer = 0;
	HRESULT result;
	result = D3DX11CompileFromFile(vsFilename, NULL, NULL, "FontVertexShader", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, NULL,
		&vertexShaderBuffer, &errorMessage, NULL);
	
	if (FAILED(result))
	{
		//write error messsage if it exists
		if (errorMessage)
			OutputShaderErrorMessage(errorMessage, hwnd, vsFilename);

		//or there was no shader file
		else
			MessageBox(hwnd, vsFilename, L"Missing Shader File", MB_OK);

		return false;
	}

	// Compile the pixel shader code.
	ID3D10Blob* pixelShaderBuffer = 0;
	result = D3DX11CompileFromFile(psFilename, NULL, NULL, "FontPixelShader", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, NULL,
		&pixelShaderBuffer, &errorMessage, NULL);
	
	if (FAILED(result))
	{
		// write error messsage if it exists
		if (errorMessage)
			OutputShaderErrorMessage(errorMessage, hwnd, psFilename);

		//or there was no shader file
		else
			MessageBox(hwnd, psFilename, L"Missing Shader File", MB_OK);

		return false;
	}

	
	//create vertex shader object
	result = device->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), NULL,
		&m_vertexShader);
	if (FAILED(result))
		return false;

	//create pixel shader object
	result = device->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(), pixelShaderBuffer->GetBufferSize(), NULL,
		&m_pixelShader);
	if (FAILED(result))
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

	polygonLayout[1].SemanticName = "TEXCOORD";
	polygonLayout[1].SemanticIndex = 0;
	polygonLayout[1].Format = DXGI_FORMAT_R32G32_FLOAT;
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
	if (FAILED(result))
		return false;

	//release temporary buffers
	vertexShaderBuffer->Release();
	vertexShaderBuffer = 0;

	pixelShaderBuffer->Release();
	pixelShaderBuffer = 0;


	//create const buffer desc
	D3D11_BUFFER_DESC constantBufferDesc;
	constantBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	constantBufferDesc.ByteWidth = sizeof(ConstantBufferType);

	constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constantBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	constantBufferDesc.MiscFlags = 0;
	constantBufferDesc.StructureByteStride = 0;
	
	result = device->CreateBuffer(&constantBufferDesc, NULL, &m_constantBuffer);
	if (FAILED(result))
		return false;

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

	//create texture sample state
	result = device->CreateSamplerState(&samplerDesc, &m_sampleState);
	if (FAILED(result))
		return false;


	//create pixel buffer
	D3D11_BUFFER_DESC pixelBufferDesc;
	pixelBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	pixelBufferDesc.ByteWidth = sizeof(PixelBufferType);
	pixelBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	pixelBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	pixelBufferDesc.MiscFlags = 0;
	pixelBufferDesc.StructureByteStride = 0;

	// Create the pixel constant buffer pointer so we can access the pixel shader constant buffer from within this class.
	result = device->CreateBuffer(&pixelBufferDesc, NULL, &m_pixelBuffer);
	if (FAILED(result))
		return false;

	return true;
}

void FontShaderClass::Shutdown()
{
	//---- shutdown vertex and pixel shader

	//release pixel buffer
	if (m_pixelBuffer)
	{
		m_pixelBuffer->Release();
		m_pixelBuffer = 0;
	}

	//release sample sate
	if (m_sampleState)
	{
		m_sampleState->Release();
		m_sampleState = 0;
	}

	//release constant buffer
	if (m_constantBuffer)
	{
		m_constantBuffer->Release();
		m_constantBuffer = 0;
	}

	//release layout
	if (m_layout)
	{
		m_layout->Release();
		m_layout = 0;
	}

	//release pixel shader
	if (m_pixelShader)
	{
		m_pixelShader->Release();
		m_pixelShader = 0;
	}

	//release vertex shader
	if (m_vertexShader)
	{
		m_vertexShader->Release();
		m_vertexShader = 0;
	}

	return;
}

bool FontShaderClass::Render(ID3D11DeviceContext* deviceContext, int indexCount, D3DXMATRIX worldMatrix, D3DXMATRIX viewMatrix, 
							 D3DXMATRIX projectionMatrix, ID3D11ShaderResourceView* texture, D3DXVECTOR4 pixelColor)
{
	//set shader parameters and check if there was an error
	bool result = SetShaderParameters(deviceContext, worldMatrix, viewMatrix, projectionMatrix, texture, pixelColor);
	if(!result)
		return false;

	//render buffer
	RenderShader(deviceContext, indexCount);

	return true;
}


void FontShaderClass::OutputShaderErrorMessage(ID3D10Blob* errorMessage, HWND hwnd, WCHAR* shaderFilename)
{
	char* compileErrors;
	compileErrors = (char*)(errorMessage->GetBufferPointer());

	//get length of message
	unsigned long bufferSize;
	bufferSize = errorMessage->GetBufferSize();

	//write the error message into a file
	ofstream fout;
	fout.open("shader-error.txt");

	for (unsigned long i = 0; i < bufferSize; i++)
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

bool FontShaderClass::SetShaderParameters(ID3D11DeviceContext* deviceContext, D3DXMATRIX worldMatrix, D3DXMATRIX viewMatrix, 
										  D3DXMATRIX projectionMatrix, ID3D11ShaderResourceView* texture, D3DXVECTOR4 pixelColor)
{
	//transpose matrices for shader
	D3DXMatrixTranspose(&worldMatrix, &worldMatrix);
	D3DXMatrixTranspose(&viewMatrix, &viewMatrix);
	D3DXMatrixTranspose(&projectionMatrix, &projectionMatrix);

	
	//lock the constant buffer
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	result = deviceContext->Map(m_constantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if(FAILED(result))
		return false;

	//get pointer and copy matrices into constant buffer
	ConstantBufferType* dataPtr;
	dataPtr = (ConstantBufferType*)mappedResource.pData;
	dataPtr->world = worldMatrix;
	dataPtr->view = viewMatrix;
	dataPtr->projection = projectionMatrix;

	//unlock constant buffer
    deviceContext->Unmap(m_constantBuffer, 0);

	
	//set constant buffer in vertex shader with updated values
	unsigned int bufferNumber = 0;
    deviceContext->VSSetConstantBuffers(bufferNumber, 1, &m_constantBuffer);

	// Set shader texture resource in the pixel shader.
	deviceContext->PSSetShaderResources(0, 1, &texture);

	//lock pixel buffer and write into it
	result = deviceContext->Map(m_pixelBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if(FAILED(result))
		return false;

	PixelBufferType* dataPtr2;
	dataPtr2 = (PixelBufferType*)mappedResource.pData;
	dataPtr2->pixelColor = pixelColor;

	//unlock pixel buffer
    deviceContext->Unmap(m_pixelBuffer, 0);

	//update pixel shader wwith pixel buffer
	bufferNumber = 0;
    deviceContext->PSSetConstantBuffers(bufferNumber, 1, &m_pixelBuffer);

	return true;
}

void FontShaderClass::RenderShader(ID3D11DeviceContext* deviceContext, int indexCount)
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