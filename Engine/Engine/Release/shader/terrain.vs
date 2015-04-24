cbuffer MatrixBuffer
{
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
};

struct VertexInputType
{
    float4 position : POSITION;
	float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
};

struct PixelInputType
{
    float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
};

PixelInputType TerrainVertexShader(VertexInputType input)
{
	//init forth position
    PixelInputType output;
    input.position.w = 1.0f;

    //calc vertex position with all matrices
    output.position = mul(input.position, worldMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);

	//get texture coordinates
	output.tex = input.tex;
    
    //calc normal vector against world matrix only
    output.normal = mul(input.normal, (float3x3)worldMatrix);
	
    //normalize the normal vector
    output.normal = normalize(output.normal);

    return output;
}