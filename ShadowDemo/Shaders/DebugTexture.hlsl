
Texture2D gTexture :register(t0);

cbuffer cbPerFrame: register(b0)
{
	float4x4 matWVP;
}

SamplerState samLinear : register(s0);

struct VertexIn
{
	float3 PosL    : POSITION;
	float3 NormalL : NORMAL;
	float2 Tex     : TEXCOORD;
};

struct VertexOut
{
	float4 PosH : SV_POSITION;
	float2 Tex  : TEXCOORD;
};

VertexOut VS(VertexIn vin)
{
	VertexOut vout;

	vout.PosH = mul(float4(vin.PosL, 1.0f), matWVP);
	vout.Tex = vin.Tex;

	return vout;
}

float4 PS(VertexOut pin) : SV_Target
{
	float4 c = gTexture.Sample(samLinear, pin.Tex).r;
	return float4(c.rrr, 1);
}