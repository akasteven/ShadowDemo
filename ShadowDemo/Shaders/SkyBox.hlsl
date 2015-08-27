TextureCube skyTex: register(t0);
SamplerState samTriLinear: register(s0);

cbuffer cbPerFrame: register(b0)
{
	float4x4 WVP;
};

struct VS_INPUT
{
	float3 PosL: POSITION; 
};

struct PS_INPUT
{
	float4 PosH : SV_POSITION;
	float3 PosL : POSITION;
};

PS_INPUT VS(VS_INPUT input)
{
	PS_INPUT output = (PS_INPUT)0;
	output.PosH = mul(float4(input.PosL, 1.0f), WVP).xyww;
	output.PosL = input.PosL;
	return output;
}

float4 PS(PS_INPUT input): SV_Target
{
	return skyTex.Sample(samTriLinear, input.PosL);
}

