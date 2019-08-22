#pragma once

// https://github.com/guided-hacking/GH_D3D11_Hook/blob/master/GH_D3D11_Hook/shadez.h

constexpr const char* szShadez = R"(
// ConstantBuffer
cbuffer ConstantBuffer
{
	float2 translate;
}
// VertexShader
float4 VS( float2 pos : POSITION) : SV_POSITION
{
	if (pos.y < 0)
	{
		return float4((pos + translate), 0.0f, 1.0f);
	}
	else
	{
		return float4((pos - translate), 0.0f, 1.0f);
	}
}
// PixelShader
float4 PS() : SV_TARGET
{
	return float4(0.0f, 0.0f, 0.0f, 1.0f);
}
)";