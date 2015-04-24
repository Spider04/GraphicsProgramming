cbuffer MatrixBuffer
{
	matrix worldMatrix;
	matrix viewMatrix;
	matrix projectionMatrix;
};


struct VertexInputData
{
    float4 pos : POS;
    float2 tex : TEXCOORD0;
};

struct PixelInputData
{
    float4 pos : SV_POSITION;
    float2 tex : TEXCOORD0;
};


PixelInputData ColorFilterVertexShader(VertexInputData input)
{
    PixelInputData output;
    
	//Change the position vector to be 4 units for proper matrix calculations.
    input.pos.w = 1.0f;

	//calculate vertex position against world, view and projection matrix
    output.pos = mul(input.pos, worldMatrix);
    output.pos = mul(output.pos, viewMatrix);
    output.pos = mul(output.pos, projectionMatrix);
    
	//use texture cordinates for pixel shader
	output.tex = input.tex;
    
    return output;
}