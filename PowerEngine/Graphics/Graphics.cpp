#include "Graphics.h"
#include "..//Engine/PowerEngine.h"
PowerEngine engine;

// Indicates to hybrid graphics systems to prefer the discrete part by default
extern "C"
{
	__declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
	__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}

bool Graphics::Initialize(HWND hwnd, int width, int height)
{
	this->windowWidth = width;
	this->windowHeight = height;

	this->fpsTimer.Start();

	if (!InitializeDirectX(hwnd))
		return false;

	if (!InitializeShaders())
		return false;

	if (!InitializeScene())
		return false;

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	ImGui_ImplWin32_Init(hwnd);
	ImGui_ImplDX11_Init(this->device.Get(), this->deviceContext.Get());
	ImGui::StyleColorsDark();

	return true;
}

void Graphics::RenderFrame()
{
	static int fpsCounter = 0;
	static std::string fpsString = "FPS:";
	fpsCounter += 1;
	if (fpsTimer.GetMilisecondsElapsed() > 1000.0f)
	{
		fpsString = "FPS: " + std::to_string(fpsCounter);
		fpsCounter = 0;
		fpsTimer.Restart();
	}

	this->cb_ps_light.data.dynamicLightColor = light.lightColor;
	this->cb_ps_light.data.dynamicLightStrength = light.lightStrength;
	this->cb_ps_light.data.dynamicLightPosition = light.GetPositionFloat3();
	this->cb_ps_light.ApplyChanges();
	this->deviceContext->PSSetConstantBuffers(0, 1, this->cb_ps_light.GetAddressOf());
	
	static float bgcolor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	static float scale[] = { 1.0f, 1.0f, 1.0f };
	static float lightColor[] = { 1, 1, 1};
	static float lightStrength = 1;
	static float alpha = 1.0f;
	this->deviceContext->ClearRenderTargetView(this->renderTargetView.Get(), bgcolor);
	this->deviceContext->ClearDepthStencilView(this->depthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	this->deviceContext->IASetInputLayout(this->vertexshader.GetInputLayout());
	this->deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	this->deviceContext->RSSetState(this->rasterizerState.Get());
	this->deviceContext->OMSetDepthStencilState(this->depthStencilState.Get(), 0);
	this->deviceContext->OMSetBlendState(NULL, NULL, 0xFFFFFFFF);  //Jpres, but if i don't set it as NULL all goes wrong...
	this->deviceContext->PSSetSamplers(0, 1, this->samplerState.GetAddressOf());

	this->deviceContext->VSSetShader(vertexshader.GetShader(), NULL, 0);
	this->deviceContext->PSSetShader(pixelshader.GetShader(), NULL, 0);

	UINT offset = 0;

	{
		this->gameObject.SetScale(XMFLOAT3(scale[0], scale[1], scale[2]));
		this->gameObject.SetAlpha(alpha);
		this->gameObject.Draw(camera.GetViewMatrix() * camera.GetProjectionMatrix());	
		this->gameObject2.Draw(camera.GetViewMatrix() * camera.GetProjectionMatrix());
	}

	{
		this->deviceContext->PSSetShader(pixelshader_nolight.GetShader(), NULL, 0);
		this->light.SetLightColor(lightColor[0], lightColor[1], lightColor[2]);
		this->light.SetLightStrength(lightStrength);
		this->light.Draw(camera.GetViewMatrix() * camera.GetProjectionMatrix());
	}


	spriteBatch->Begin();
	spriteFont->DrawString(spriteBatch.get(), "Multi Test of Engine capabilities", DirectX::XMFLOAT2(0, 0), { 0.19f , 0.42f, 0.11f , 1.0f }, 0.0f, DirectX::XMFLOAT2(0.0f, 0.0f), DirectX::XMFLOAT2(1.0f, 1.0f));
	spriteBatch->End();

	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	ImGui::Begin("Basic data and operations");

	ImGui::Text(fpsString.c_str());
	ImGui::DragFloat4("R/G/B/A values of background", bgcolor, 0.01f, 0.0f, 1.0f);
	ImGui::DragFloat("Alpha of model", &alpha, 0.01f, 0.0f, 1.0f);
	ImGui::DragFloat3("X/Y/Z values of model scale", scale, 0.01f, 0.0f, 1000.0f);
	if (ImGui::Button("Apply scale of model"))
	{
		this->gameObject.ApplyScale();
		scale[0] = 1.0f;
		scale[1] = 1.0f;
		scale[2] = 1.0f;
	}
	ImGui::DragFloat3("Ambient Light Color", &this->cb_ps_light.data.ambientLightColor.x, 0.01f, 0.0f, 1.0f);
	ImGui::DragFloat("Ambient Light Strength", &this->cb_ps_light.data.ambientLightStrength, 0.01f, 0.0f, 100.0f);
	ImGui::DragFloat3("Dynamic Light Color", lightColor, 0.01f, 0.0f, 1.0f);
	ImGui::DragFloat("Dynamic Light Strength", &lightStrength, 0.01f, 0.0f, 100.0f);
		
	ImGui::End();
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	this->swapchain->Present(0, NULL);
}

bool Graphics::InitializeDirectX(HWND hwnd)
{
	try
	{

		std::vector<AdapterData> adapters = AdapterReader::GetAdapters();

		if (adapters.size() < 1)
		{
			ErrorLogger::Log("No IDXGI Adapters found.");
			return false;
		}

		DXGI_SWAP_CHAIN_DESC scd = { 0 };

		scd.BufferDesc.Width = this->windowWidth;
		scd.BufferDesc.Height = this->windowHeight;
		scd.BufferDesc.RefreshRate.Numerator = 170;
		scd.BufferDesc.RefreshRate.Denominator = 1;
		scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		scd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		scd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

		scd.SampleDesc.Count = 1;
		scd.SampleDesc.Quality = 0;

		scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		scd.BufferCount = 1;
		scd.OutputWindow = hwnd;
		scd.Windowed = engine.IsWindowed;
		scd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
		scd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

		HRESULT hr;
		hr = D3D11CreateDeviceAndSwapChain(
			nullptr,                // Video adapter (physical GPU) to use, or null for default
			D3D_DRIVER_TYPE_HARDWARE,   // We want to use the hardware (GPU)
			nullptr,                // Used when doing software rendering
			NULL,                // Any special options
			nullptr,        // Optional array of possible versions we want as fallback
			0,              // The number of fallback in the above param
			D3D11_SDK_VERSION,          // Current version of the SDK
			&scd,                  // Address of swap chain options
			this->swapchain.GetAddressOf(),                 // Pointer to our Swap Chain pointer
			this->device.GetAddressOf(),                    // Pointer to our Device pointer
			NULL,            // This will hold the actual feature level the app will use
			this->deviceContext.GetAddressOf());                  // Pointer to our Device Context pointer

		COM_ERROR_IF_FAILED(hr, "Failed to create device and swapchain.");

		Microsoft::WRL::ComPtr<ID3D11Texture2D> backBuffer;
		hr = this->swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(backBuffer.GetAddressOf()));
		COM_ERROR_IF_FAILED(hr, "GetBuffer Failed.");

		hr = this->device->CreateRenderTargetView(backBuffer.Get(), NULL, this->renderTargetView.GetAddressOf());
		COM_ERROR_IF_FAILED(hr, "Failed to create render target view.");

		CD3D11_TEXTURE2D_DESC depthStencilTextureDesc(DXGI_FORMAT_D24_UNORM_S8_UINT, this->windowWidth, this->windowHeight);
		depthStencilTextureDesc.MipLevels = 1;
		depthStencilTextureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

		hr = this->device->CreateTexture2D(&depthStencilTextureDesc, NULL, this->depthStencilBuffer.GetAddressOf());
		COM_ERROR_IF_FAILED(hr, "Failed to create depth stencil buffer.");

		hr = this->device->CreateDepthStencilView(this->depthStencilBuffer.Get(), NULL, this->depthStencilView.GetAddressOf());
		COM_ERROR_IF_FAILED(hr, "Failed to create depth stencil view.");

		this->deviceContext->OMSetRenderTargets(1, this->renderTargetView.GetAddressOf(), this->depthStencilView.Get());

		//Create depth stencil state
		CD3D11_DEPTH_STENCIL_DESC depthstencildesc(D3D11_DEFAULT);
		depthstencildesc.DepthFunc = D3D11_COMPARISON_FUNC::D3D11_COMPARISON_LESS_EQUAL;

		hr = this->device->CreateDepthStencilState(&depthstencildesc, this->depthStencilState.GetAddressOf());
		COM_ERROR_IF_FAILED(hr, "Failed to create depth stencil state.");

		//Create the Viewport
		CD3D11_VIEWPORT viewport(0.0f, 0.0f, static_cast<float>(this->windowWidth), static_cast<float>(this->windowHeight));;
		this->deviceContext->RSSetViewports(1, &viewport);

		//Create Rasterizer State
		CD3D11_RASTERIZER_DESC rasterizerDesc(D3D11_DEFAULT);
		hr = this->device->CreateRasterizerState(&rasterizerDesc, this->rasterizerState.GetAddressOf());
		COM_ERROR_IF_FAILED(hr, "Failed to create rasterizer state.");

		//Create Rasterizer State for culling front
		CD3D11_RASTERIZER_DESC rasterizerDesc_CullFront(D3D11_DEFAULT);
		rasterizerDesc_CullFront.CullMode = D3D11_CULL_MODE::D3D11_CULL_FRONT;
		hr = this->device->CreateRasterizerState(&rasterizerDesc_CullFront, this->rasterizerState_CullFront.GetAddressOf());
		COM_ERROR_IF_FAILED(hr, "Failed to create rasterizer state.");

		//Create Blend State
		D3D11_BLEND_DESC blendDesc;
		ZeroMemory(&blendDesc, sizeof(blendDesc));

		D3D11_RENDER_TARGET_BLEND_DESC rtbd;
		ZeroMemory(&rtbd, sizeof(rtbd));

		rtbd.BlendEnable = true;
		rtbd.SrcBlend = D3D11_BLEND::D3D11_BLEND_SRC_ALPHA;
		rtbd.DestBlend = D3D11_BLEND::D3D11_BLEND_INV_SRC_ALPHA;
		rtbd.BlendOp = D3D11_BLEND_OP::D3D11_BLEND_OP_ADD;
		rtbd.SrcBlendAlpha = D3D11_BLEND::D3D11_BLEND_ONE;
		rtbd.DestBlendAlpha = D3D11_BLEND::D3D11_BLEND_ZERO;
		rtbd.BlendOpAlpha = D3D11_BLEND_OP::D3D11_BLEND_OP_ADD;
		rtbd.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE::D3D11_COLOR_WRITE_ENABLE_ALL;

		blendDesc.RenderTarget[0] = rtbd;
		blendDesc.RenderTarget[0].BlendEnable = true;

		hr = this->device->CreateBlendState(&blendDesc, this->blendState.GetAddressOf());
		COM_ERROR_IF_FAILED(hr, "Failed to create blend state.");

		spriteBatch = std::make_unique<DirectX::SpriteBatch>(this->deviceContext.Get());
		spriteFont = std::make_unique<DirectX::SpriteFont>(this->device.Get(), L"Extras\\Fonts\\imperial.spritefont");

		//Create sampler description for sampler state
		CD3D11_SAMPLER_DESC sampDesc(D3D11_DEFAULT);
		sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		hr = this->device->CreateSamplerState(&sampDesc, this->samplerState.GetAddressOf()); //Create sampler state
		COM_ERROR_IF_FAILED(hr, "Failed to create sampler state.");
	}

	catch (COMException& exception)
	{
		ErrorLogger::Log(exception);
		return false;
	}

	return true;
}

bool Graphics::InitializeShaders()
{
	static bool isIde = true;

	std::wstring shaderfolderIde = L"";
	std::wstring shaderfolderExe = L"";
#pragma region DetermineShaderPath
	if (IsDebuggerPresent() == TRUE)
	{
#ifdef _DEBUG //Debug Mode
#ifdef _WIN64 //x64
		shaderfolderIde = L"..\\x64\\Debug\\";
#else  //x86 (Win32)
		shaderfolderIde = L"..\\Debug\\";
#endif
#else //Release Mode
#ifdef _WIN64 //x64
		shaderfolderIde = L"..\\x64\\Release\\";
#else  //x86 (Win32)
		shaderfolderIde = L"..\\Release\\";
#endif
#endif
	}

	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{"POSITION", 0, DXGI_FORMAT::DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA, 0  },
		{"TEXCOORD", 0, DXGI_FORMAT::DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA, 0  },
		{"NORMAL", 0, DXGI_FORMAT::DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA, 0  },
	};

	UINT numElements = ARRAYSIZE(layout);

	if (isIde == true)
	{
		if (!vertexshader.Initialize(this->device, shaderfolderIde + L"vertexshader.cso", layout, numElements))
			return false;

		if (!pixelshader.Initialize(this->device, shaderfolderIde + L"pixelshader.cso"))
			return false;

		if (!pixelshader_nolight.Initialize(this->device, shaderfolderIde + L"pixelshader_nolight.cso"))
			return false;
	}
	if (isIde == false)
	{
		if (!vertexshader.Initialize(this->device,L"vertexshader.cso", layout, numElements))
			return false;

		if (!pixelshader.Initialize(this->device,L"pixelshader.cso"))
			return false;

		if (!pixelshader_nolight.Initialize(this->device, L"pixelshader_nolight.cso"))
			return false;
	}

	return true;
}

bool Graphics::InitializeScene()
{
	std::string mainFilePath = "Extras\\Objects\\nanosuit\\";
	std::string modelFilePath = mainFilePath + "nanosuit.obj";
	std::string mainFilePath2 = "Extras\\Objects\\GlassObjects\\";
	std::string modelFilePath2 = mainFilePath2 + "glass.fbx";

	try
	{
		//Initialize Constant Buffer(s)
		HRESULT hr = this->cb_vs_vertexshader.Initialize(this->device.Get(), this->deviceContext.Get());
		COM_ERROR_IF_FAILED(hr, "Failed to initialize constant buffer.");

		hr = this->cb_ps_light.Initialize(this->device.Get(), this->deviceContext.Get());
		COM_ERROR_IF_FAILED(hr, "Failed to initialize constant buffer.");

		this->cb_ps_light.data.ambientLightColor = XMFLOAT3(1.0f, 1.0f, 1.0f);
		this->cb_ps_light.data.ambientLightStrength = 1.0f;

		if (!gameObject.Initialize(modelFilePath, this->device.Get(), this->deviceContext.Get(), this->cb_vs_vertexshader, this->cb_ps_light, this->pixelshader, XMFLOAT3(1, 1, 1)))
		{
			return false;
		}

		if (!gameObject2.Initialize(modelFilePath2, this->device.Get(), this->deviceContext.Get(), this->cb_vs_vertexshader, this->cb_ps_light, this->pixelshader, XMFLOAT3(0.05, 0.05, 0.05)))
		{
			return false;
		}

		if (!light.Initialize(this->device.Get(), this->deviceContext.Get(), this->cb_vs_vertexshader, this->cb_ps_light, XMFLOAT3(1, 1, 1)))
		{
			return false;
		}

		camera.SetPosition(0.0f, 0.0f, -2.0f);
		camera.SetProjectionValues(90, static_cast<float>(windowWidth) / static_cast<float>(windowHeight), 0.1f, 5000.0f);
	}

	catch (COMException& exception)
	{
		ErrorLogger::Log(exception);
		return false;
	}

	return true;
}