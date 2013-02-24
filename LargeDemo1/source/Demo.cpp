#include "stdafx.h"
#include "Demo.h"
#include "Window.h"
#include "DeviceManager.h"
#include "Cube.h"
#include "Terrain.h"
#include "Timer.h"
#include "Utils.h"
#include "FreeCamera.h"
#include "InputManager.h"

#include "SamplerState.h"
#include "BufferObject.h"

#include "FontSheet.h"
#include "FontRenderer.h"


Demo::Demo() :
	_window(new Window()),
	_passedTimeSinceStart(0.0f),
	_frameNumber(0)
{
}

Demo::~Demo()
{
}

bool Demo::Initialize(HINSTANCE hInstance)
{
	if(!_window->InitWindow(hInstance, L"LargeDemo1", 1600, 900))
		return false;

	// device
	if(!DeviceManager::Get().InitDevice())
		return false;
	DeviceManager::Get().CreateSwapChainAndBackBuffer(_window->GetHandle(), DeviceManager::Get().GetAvailableMultisamplingSettings().back());
	
	// camera
	_camera.reset(new FreeCamera(Utils::DegToRad(70.0f), DeviceManager::Get().GetBackBufferAspectRatio()));
	_camera->ActivateCameraConstantBufferForAllShader(CONSTANT_BUFFER_INDEX_CAMERA);

	// screen constants
	_screenConstants.reset(new ConstantBuffer<ScreenConstants>());
	_screenConstants->GetContent().ScreenSize.x = static_cast<float>(DeviceManager::Get().GetBackBufferWidth());
	_screenConstants->GetContent().ScreenSize.y = static_cast<float>(DeviceManager::Get().GetBackBufferHeight());
	_screenConstants->GetContent().AspectRatio = _screenConstants->GetContent().ScreenSize.x / _screenConstants->GetContent().ScreenSize.y;
	_screenConstants->UpdateGPUBuffer();
	DeviceManager::Get().GetContext()->CSSetConstantBuffers(CONSTANT_BUFFER_INDEX_SCREEN, 1, _screenConstants->GetBufferPointer());
	DeviceManager::Get().GetContext()->GSSetConstantBuffers(CONSTANT_BUFFER_INDEX_SCREEN, 1, _screenConstants->GetBufferPointer());
	DeviceManager::Get().GetContext()->PSSetConstantBuffers(CONSTANT_BUFFER_INDEX_SCREEN, 1, _screenConstants->GetBufferPointer());
	DeviceManager::Get().GetContext()->VSSetConstantBuffers(CONSTANT_BUFFER_INDEX_SCREEN, 1, _screenConstants->GetBufferPointer());
	DeviceManager::Get().GetContext()->HSSetConstantBuffers(CONSTANT_BUFFER_INDEX_SCREEN, 1, _screenConstants->GetBufferPointer());
	DeviceManager::Get().GetContext()->DSSetConstantBuffers(CONSTANT_BUFFER_INDEX_SCREEN, 1, _screenConstants->GetBufferPointer());

	// register standard sampler
	DeviceManager::Get().SetSamplerState(SamplerState::PointWrap, 0);
	DeviceManager::Get().SetSamplerState(SamplerState::PointClamp, 1);
	DeviceManager::Get().SetSamplerState(SamplerState::TriLinearWrap, 2);
	DeviceManager::Get().SetSamplerState(SamplerState::TriLinearClamp, 3);
	DeviceManager::Get().SetSamplerState(SamplerState::AnisotropicWrap, 4);
	DeviceManager::Get().SetSamplerState(SamplerState::AnisotropicClamp, 5);


	// objects
	_cube.reset(new Cube());
	_terrain.reset(new Terrain(2048*2, 2048, 8));

	// font
	_fontRenderer.reset(new FontRenderer());
	_font0.reset(new FontSheet(L"Arial", 50.0f, FontSheet::FontStyle::Regular));

	return true;
}

void Demo::RunMainLoop()
{
	Timer timer;
	float lastFrameLength = 0.0f;

	while(true)
	{
		lastFrameLength = timer.GetTimeSeconds();
		_passedTimeSinceStart += lastFrameLength;
		timer.Reset();


		if(!_window->MessagePump())
			break;

		Update(lastFrameLength);
		++_frameNumber;
		Draw(lastFrameLength);
	}
}


void Demo::Update(float timeSinceLastUpdate)
{
	_camera->Update(timeSinceLastUpdate);

	if(InputManager::Get().WasKeyboardKeyPressed(Key::W))
		_terrain->SetWireframe(!_terrain->GetWireframe());
}

void Demo::Draw(float timeSinceLastUpdate)
{
	DeviceManager::Get().ClearBackAndDepthBuffer();

	_camera->UpdateGPUBuffer();

//	_cube->Draw(*_camera, _passedTimeSinceStart);
//	_terrain->Draw(*_camera, 100.0f);

	_fontRenderer->DrawString(*_font0, "ich hab meinen schatz lieb :3", SimpleMath::Vector2(50.0f));

	DeviceManager::Get().GetSwapChain()->Present(0, 0);
}