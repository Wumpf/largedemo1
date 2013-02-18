#include "stdafx.h"
#include "PerlinNoiseGenerator.h"
#include "Utils.h"
#include "Shader.h"
#include "Texture.h"
#include "DeviceManager.h"
#include "BufferObject.h"

const unsigned int PerlinNoiseGenerator::NUM_THREADS_PER_GROUP = 512;

PerlinNoiseGenerator::PerlinNoiseGenerator() :
	_computeShader(ComputeShader::FromFile("shader/perlinnoisegenerator.cso"))
{
}

std::shared_ptr<Texture2D> PerlinNoiseGenerator::Generate(unsigned int width, unsigned int height)
{
	std::shared_ptr<Texture2D> whiteNoise(Texture2D::CreateFromData(Utils::RandomFloats(width* height, 0.0f, 1.0f).get(), DXGI_FORMAT_R32_FLOAT, width, height, TextureCreationFlags::SHADERRES_VIEW));
	std::shared_ptr<Texture2D> noiseOutput(Texture2D::CreateEmpty(DXGI_FORMAT_R32_FLOAT, width, height, TextureCreationFlags::SHADERRES_VIEW | TextureCreationFlags::UNORDEREDACCESS_VIEW));

	auto view = noiseOutput->GetUnorderedAcessView().p;	// make easier!
	DeviceManager::Get().GetContext()->CSSetUnorderedAccessViews(0, 1, &view, nullptr);
	_computeShader->Activate();

	struct SettingsBuffer
	{
		SimpleMath::Vector2 TextureSize;
		float Persistance;
		uint32_t OctaveCount;
	};
	SettingsBuffer settings;
	settings.TextureSize.x = static_cast<float>(width);
	settings.TextureSize.y = static_cast<float>(height);
	settings.Persistance = 0.7f;
	settings.OctaveCount = 8;

	auto constantBuffer = BufferObject::CreateConstantBuffer(sizeof(settings));
	constantBuffer->Write(&settings);
	DeviceManager::Get().GetContext()->CSSetConstantBuffers(0, 1, constantBuffer->GetBufferPointer());
	
	auto viewPointer = whiteNoise->GetShaderResourceView().p;
	DeviceManager::Get().GetContext()->CSSetShaderResources(0, 1, &viewPointer);

	DeviceManager::Get().GetContext()->Dispatch(static_cast<UINT>(ceilf(width) / static_cast<float>(NUM_THREADS_PER_GROUP) ), height, 1);

	view = nullptr;
	DeviceManager::Get().GetContext()->CSSetUnorderedAccessViews(0, 1, &view, nullptr);

	return noiseOutput;
}