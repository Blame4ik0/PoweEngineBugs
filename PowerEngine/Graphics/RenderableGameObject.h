#pragma once
#include "GameObject.h"

class RenderableGameObject : public GameObject
{
public:
	bool Initialize(const std::string& filePath, ID3D11Device* device, ID3D11DeviceContext* deviceContext, ConstantBuffer<CB_VS_vertexshader>& cb_vs_vertexshader, ConstantBuffer<CB_PS_light>& cb_ps_light, PixelShader pixelShader, XMFLOAT3 basicScale);

	void Draw(const XMMATRIX& viewProjectionMatrix);

protected:
	Model model;
	void UpdateMatrix() override;
	PixelShader pixelShader;
	XMMATRIX worldMatrix = XMMatrixIdentity();
};
