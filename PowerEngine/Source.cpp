#include "..\\PowerEngine/Engine\PowerEngine.h"


int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{

	HRESULT hr = CoInitialize(NULL);
	if (FAILED(hr))
	{
		ErrorLogger::Log(hr, "Failed to CoInitialize");
		return -1;
	}

	PowerEngine engine;
	engine.IsWindowed = FALSE;
	if (engine.Initialize(hInstance, "Test Window", "TestWindowClass", GetSystemMetrics(SM_CXFULLSCREEN), GetSystemMetrics(SM_CYFULLSCREEN)))
	{
		while (engine.ProcessMessages() == true)
		{
			engine.Update();
			engine.RenderFrame();
		}
	}
	else
	{
		ErrorLogger::Log(E_ABORT, "Unable to initialize engine, abort");
		exit(-1);
		return false;
	}
	return 0;
}