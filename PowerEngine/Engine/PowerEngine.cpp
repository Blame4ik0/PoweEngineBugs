#include "PowerEngine.h"

uint64_t timeSinceEpochMillisec() {
	using namespace std::chrono;
	return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
}

bool PowerEngine::Initialize(HINSTANCE hInstance, std::string window_title, std::string window_class, int width, int height)
{
	timer.Start();

	if (!this->render_window.Initialize(this, hInstance, window_title, window_class, width, height))
		return false;

	if (!gfx.Initialize(this->render_window.GetHWND(), width, height))
		return false;

	return true;
}

bool PowerEngine::ProcessMessages()
{
	return this->render_window.ProcessMessages();
}

void PowerEngine::Update()
{
	float deltaTime = timer.GetMilisecondsElapsed();
	timer.Restart();

	while (!keyboard.CharBufferIsEmpty())
	{
		unsigned char ch = keyboard.ReadChar();
	}

	while (!keyboard.KeyBufferIsEmpty())
	{
		KeyboardEvent kbe = keyboard.ReadKey();
		unsigned char keycode = kbe.GetKeyCode();
	}

	while (!mouse.EventBufferIsEmpty())
	{
		MouseEvent me = mouse.ReadEvent();
		if (mouse.IsRightDown())
		{
			if (me.GetType() == MouseEvent::EventType::RAW_MOVE)
			{
				this->gfx.camera.AdjustRotation((float)me.GetPosY() * 0.001f, (float)me.GetPosX() * 0.001f, 0);
			}
		}
	}

	float cameraSpeed = 0.003f;
	if (keyboard.KeyIsPressed(VK_SHIFT))
	{
		cameraSpeed = 0.3f;
	}

	if (keyboard.KeyIsPressed('C'))
	{
		XMVECTOR lightPosition = this->gfx.camera.GetPositionVector();
		lightPosition += this->gfx.camera.GetForwardVector();
		this->gfx.light.SetPosition(lightPosition);
		this->gfx.light.SetRotation(this->gfx.camera.GetRotationFloat3());
	}

	if (keyboard.KeyIsPressed('R'))
	{
		this->gfx.camera.SetPosition(this->gfx.gameObject.GetPositionFloat3());
	}

	if (keyboard.KeyIsPressed('W'))
	{
		this->gfx.camera.AdjustPosition(this->gfx.camera.GetForwardVector() * cameraSpeed * deltaTime);
	}
	if (keyboard.KeyIsPressed('S'))
	{
		this->gfx.camera.AdjustPosition(this->gfx.camera.GetBackwardVector() * cameraSpeed * deltaTime);
	}
	if (keyboard.KeyIsPressed('A'))
	{
		this->gfx.camera.AdjustPosition(this->gfx.camera.GetLeftVector() * cameraSpeed * deltaTime);
	}
	if (keyboard.KeyIsPressed('D'))
	{
		this->gfx.camera.AdjustPosition(this->gfx.camera.GetRightVector() * cameraSpeed * deltaTime);
	}
	if (keyboard.KeyIsPressed(VK_SPACE))
	{
		this->gfx.camera.AdjustPosition(0.0f, cameraSpeed * deltaTime, 0.0f);
	}
	if (keyboard.KeyIsPressed(VK_CONTROL))
	{
		this->gfx.camera.AdjustPosition(0.0f, -cameraSpeed * deltaTime, 0.0f);
	}
}

void PowerEngine::RenderFrame()
{
	gfx.RenderFrame();
}