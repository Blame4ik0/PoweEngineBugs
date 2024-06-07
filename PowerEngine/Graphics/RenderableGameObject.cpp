#include "RenderableGameObject.h"

bool RenderableGameObject::Initialize(const std::string& filePath, ID3D11Device* device, ID3D11DeviceContext* deviceContext, ConstantBuffer<CB_VS_vertexshader>& cb_vs_vertexshader, ConstantBuffer<CB_PS_light>& cb_ps_light, PixelShader pixelShader, XMFLOAT3 basicScale)
{
	if (!model.Initialize(filePath, device, deviceContext, cb_vs_vertexshader, cb_ps_light))
		return false;

	this->pixelShader = pixelShader;

	this->SetPosition(0.0f, 0.0f, 0.0f);
	this->SetRotation(0.0f, 0.0f, 0.0f);
	if (basicScale.x != 0 && basicScale.y != 0 && basicScale.z != 0)
	{
		this->SetBasicScale(basicScale);
		this->SetScale(1, 1, 1);
	}
	this->SetAlpha(1.0f);
	this->UpdateMatrix();
	return true;
}

void RenderableGameObject::Draw(const XMMATRIX& viewProjectionMatrix)
{
	model.Draw(this->worldMatrix, viewProjectionMatrix);
}

void RenderableGameObject::UpdateMatrix()
{
	this->worldMatrix = XMMatrixRotationRollPitchYaw(this->rot.x, this->rot.y, this->rot.z) * XMMatrixTranslation(this->pos.x, this->pos.y, this->pos.z) * XMMatrixScaling(this->scale.x, this->scale.y, this->scale.z);
	this->UpdateDirectionVectors();
}