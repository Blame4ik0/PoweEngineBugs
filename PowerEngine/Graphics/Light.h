#pragma once
#include "RenderableGameObject.h"

class Light : public RenderableGameObject
{
public:
	bool Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext, ConstantBuffer<CB_VS_vertexshader>& cb_vs_vertexshader, ConstantBuffer<CB_PS_light>& cb_ps_light, XMFLOAT3 basicScale);
	void SetLightColor(XMFLOAT3& color);
	void SetLightColor(float r, float g, float b);
	void SetLightStrength(float strength);

	DirectX::XMFLOAT3 lightColor = DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f);
	float lightStrength = 1.0f;
};