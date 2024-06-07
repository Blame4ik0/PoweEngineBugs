#include "Light.h"

bool Light::Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext, ConstantBuffer<CB_VS_vertexshader>& cb_vs_vertexshader, ConstantBuffer<CB_PS_light>& cb_ps_light, XMFLOAT3 basicScale)
{
	if (!model.Initialize("Extras\\Objects\\light.fbx", device, deviceContext, cb_vs_vertexshader, cb_ps_light))
		return false;


	this->SetPosition(0.0f, 0.0f, 0.0f);
	this->SetRotation(0.0f, 0.0f, 0.0f);
	if (basicScale.x != 0 && basicScale.y != 0 && basicScale.z != 0)
	{
		this->SetBasicScale(basicScale);
		this->SetScale(1, 1, 1);
	}
	this->UpdateMatrix();
	return true;
}

void Light::SetLightColor(XMFLOAT3& color)
{
	this->lightColor.x = color.x;
	this->lightColor.y = color.y;
	this->lightColor.z = color.z;
	this->UpdateMatrix();
}

void Light::SetLightColor(float r, float g, float b)
{
	this->lightColor.x = r;
	this->lightColor.y = g;
	this->lightColor.z = b;
	this->UpdateMatrix();
}

void Light::SetLightStrength(float strength)
{
	this->lightStrength = strength;
	this->UpdateMatrix();
}