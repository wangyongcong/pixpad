cbuffer cbPerObject : register(b0)
{
	float4x4 mvp;
};

struct VertexIn
{
	float3 pos: POSITION;
	float4 color: COLOR;
};

struct VertexOut
{
	float4 pos: SV_POSITION;
	float4 color: COLOR;
};

VertexOut VS(VertexIn vertex)
{
	VertexOut ret;
	ret.pos = mul(float4(vertex.pos, 1.0f), mvp);
	ret.color = vertex.color;
	return ret;
}

float4 PS(VertexOut vertex) : SV_Target
{
	return vertex.color;
}
