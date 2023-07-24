//#pragma once
//#include <d3d11.h>
//#include <map>
//
//class CDirectXFramework;
//class CSprite;
//struct SAnimatedSpriteData;
//
//class CSpriteFactory
//{
//	friend class CEngine;
//public:
//	bool Init(CDirectXFramework* aFramework);
//
//	CSprite* LoadSprite(std::string aFilePath);
//	CSprite* GetSprite(std::string aFilePath);
//	SAnimatedSpriteData* LoadVFXSprite(std::string aTexturePath);
//	SAnimatedSpriteData* GetVFXSprite(std::string aTexturePath);
//
//	static CSpriteFactory* GetInstance();
//
//private:
//	ID3D11ShaderResourceView* GetShaderResourceView(ID3D11Device* aDevice, std::string aTexturePath);
//	DirectX::SimpleMath::Vector2 GetTextureDimensions(ID3D11ShaderResourceView* texture) const;
//
//private:
//	CSpriteFactory();
//	~CSpriteFactory();
//
//private:
//	static CSpriteFactory* ourInstance;
//	std::map<std::string, CSprite*> mySpriteMap;
//	std::map<std::string, SAnimatedSpriteData*> myVFXSpriteMap;
//	CDirectXFramework* myFramework;
//
//	ID3D11VertexShader* myVertexShader;
//	ID3D11PixelShader* myPixelShader;
//	ID3D11GeometryShader* myGeometryShader;
//};