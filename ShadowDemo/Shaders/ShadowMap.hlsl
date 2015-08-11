Texture2D txDiffuse : register(t0);
SamplerState samLinear : register(s0);

cbuffer cbChangeOnResize : register(b1)
{
};

cbuffer cbPerFrame : register(b2)
{
};

cbuffer cbPerObject: register(b3)
{
	float4x4 lightWVP;
};

struct VS_INPUT
{
	float3 PosL : POSITION;
	float3 NorL : NORMAL;
	float2 Tex : TEXCOORD;
	float3 TangentL : TANGENT;
};

struct PS_INPUT
{
	float4 PosH : SV_POSITION;
	float2 Tex : TEXCOORD;
};

PS_INPUT VS(VS_INPUT input)
{
	PS_INPUT output = (PS_INPUT)0;
	output.PosH = mul(float4(input.PosL, 1.0f), lightWVP);
	output.Tex = input.Tex;
	return output;
}

void PS(PS_INPUT input) 
{
	float4 diffuse = txDiffuse.Sample(samLinear, input.Tex);
	clip(diffuse.a - 0.1f);
}

