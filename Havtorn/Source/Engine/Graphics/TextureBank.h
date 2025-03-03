// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once
#include <map>
#include <unordered_map>

namespace Havtorn
{
	class CGraphicsFramework;

	class CTextureBank
	{
		friend class GEngine;

	private:
		CTextureBank() = default;
		~CTextureBank();
		bool Init(const CGraphicsFramework* framework);

	public:
		ENGINE_API U32 GetTextureIndex(const std::string& fileName);
		ENGINE_API ID3D11ShaderResourceView* GetTexture(const std::string& fileName);
		ENGINE_API ID3D11ShaderResourceView* GetTexture(U32 index);
		ENGINE_API std::string GetTexturePath(U32 index);

		ENGINE_API U32 AddTexture(ID3D11ShaderResourceView* srv);

		ENGINE_API const std::vector<ID3D11ShaderResourceView*>& GetTextures();

	private:
		void AddTexture(const std::string& fileName);

	private:
		ID3D11Device* Device = nullptr;
		std::vector<ID3D11ShaderResourceView*> Textures = {};
		std::unordered_map<std::string, U32> Indices = {};
		std::unordered_map<U32, std::string> TexturePaths = {};
	};
}
