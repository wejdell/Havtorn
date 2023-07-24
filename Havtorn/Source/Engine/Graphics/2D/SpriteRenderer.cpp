//#include "stdafx.h"
//#include "SpriteRenderer.h"
//#include "RenderManager.h"
//#include "GraphicsHelpers.h"
//
//#include "Sprite.h"
//#include "SpriteInstance.h"
//#include "AnimatedUIElement.h"
//#include "DDSTextureLoader.h"
//#include "RandomNumberGenerator.h"
//
//#include <fstream>
//
//CSpriteRenderer::CSpriteRenderer() : myContext(nullptr), myObjectBuffer()
//{
//}
//
//CSpriteRenderer::~CSpriteRenderer()
//{
//}
//
//bool CSpriteRenderer::Init(CDirectXFramework* aFramework)
//{
//	if (!aFramework) {
//		return false;
//	}
//
//	myContext = aFramework->GetContext();
//	if (!myContext) {
//		return false;
//	}
//
//    ID3D11Device* device = aFramework->GetDevice();
//    if (!device) {
//        return false;
//    }
//
//    D3D11_BUFFER_DESC bufferDescription = { 0 };
//    bufferDescription.Usage = D3D11_USAGE_DYNAMIC;
//    bufferDescription.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
//    bufferDescription.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
//
//    bufferDescription.ByteWidth = sizeof(SObjectBufferData);
//    ENGINE_HR_BOOL_MESSAGE(device->CreateBuffer(&bufferDescription, nullptr, &myObjectBuffer), "Object Buffer could not be created.");
//
//    bufferDescription.ByteWidth = sizeof(STextureScrollingData);
//    ENGINE_HR_BOOL_MESSAGE(device->CreateBuffer(&bufferDescription, nullptr, &myTextureScrollingBuffer), "Texture Scrolling Buffer could not be created.");
//
//    std::ifstream vsFile;
//    vsFile.open("Shaders/SpriteVertexShader.cso", std::ios::binary);
//    std::string vsData = { std::istreambuf_iterator<char>(vsFile), std::istreambuf_iterator<char>() };
//    ID3D11VertexShader* vertexShader;
//    ENGINE_HR_BOOL_MESSAGE(device->CreateVertexShader(vsData.data(), vsData.size(), nullptr, &vertexShader), "Vertex Shader could not be created.");
//    vsFile.close();
//    mySpriteVertexShader = vertexShader;
//
//    std::ifstream gsFile;
//    gsFile.open("Shaders/SpriteGeometryShader.cso", std::ios::binary);
//    std::string gsData = { std::istreambuf_iterator<char>(gsFile), std::istreambuf_iterator<char>() };
//    ID3D11GeometryShader* geometryShader;
//    ENGINE_HR_BOOL_MESSAGE(device->CreateGeometryShader(gsData.data(), gsData.size(), nullptr, &geometryShader), "Geometry Shader could not be created.");
//    gsFile.close();
//    mySpriteGeometryShader = geometryShader;
//
//    std::ifstream psFile;
//    psFile.open("Shaders/SpritePixelShader.cso", std::ios::binary);
//    std::string psData = { std::istreambuf_iterator<char>(psFile), std::istreambuf_iterator<char>() };
//    ID3D11PixelShader* pixelShader;
//    ENGINE_HR_BOOL_MESSAGE(device->CreatePixelShader(psData.data(), psData.size(), nullptr, &pixelShader), "Pixel Shader could not be created.");
//    psFile.close();
//    mySpritePixelShader = pixelShader;
//
//	return true;
//}
//
//void CSpriteRenderer::Render(std::vector<CSpriteInstance*>& aSpriteList)
//{
//    for (unsigned int instanceIndex = 0; instanceIndex < aSpriteList.size(); ++instanceIndex)
//    {
//        CSpriteInstance* instance = aSpriteList[instanceIndex];
//        CSprite* sprite = instance->GetSprite();
//
//        myObjectBufferData.myPosition = instance->GetPosition();
//        myObjectBufferData.myColor = instance->GetColor();
//        myObjectBufferData.myUVRect = instance->GetUVRect();
//        myObjectBufferData.mySize = instance->GetSize();
//        myObjectBufferData.myRotation = DirectX::XMConvertToRadians(instance->GetRotation());
//
//        BindBuffer(myObjectBuffer, myObjectBufferData, "Object Buffer");
//
//        const CSprite::SSpriteData& spriteData = sprite->GetSpriteData();
//
//        myContext->IASetPrimitiveTopology(spriteData.myPrimitiveTopology);
//        myContext->IASetInputLayout(nullptr);
//        myContext->IASetVertexBuffers(0, 0, nullptr, nullptr, nullptr);
//        myContext->IASetIndexBuffer(nullptr, DXGI_FORMAT_UNKNOWN, 0);
//
//        myContext->VSSetConstantBuffers(0, 1, &myObjectBuffer);
//        myContext->VSSetShader(mySpriteVertexShader, nullptr, 0);
//
//        myContext->GSSetShader(mySpriteGeometryShader, nullptr, 0);
//
//        myContext->PSSetSamplers(0, 1, &spriteData.mySampler);
//        myContext->PSSetConstantBuffers(0, 1, &myObjectBuffer);
//        myContext->PSSetShaderResources(0, 1, &spriteData.myTexture);
//        myContext->PSSetShader(mySpritePixelShader, nullptr, 0);
//
//        myContext->Draw(3, 0);
//        CRenderManager::myNumberOfDrawCallsThisFrame++;
//
//        //Reset Resources
//        ID3D11ShaderResourceView* nullView = NULL;
//        myContext->PSSetShaderResources(0, 1, &nullView);
//
//        myContext->GSSetShader(nullptr, nullptr, 0);
//    }
//}
//
//void CSpriteRenderer::Render(std::vector<CAnimatedUIElement*>& someAnimatedElements) {
//    for (unsigned int instanceIndex = 0; instanceIndex < someAnimatedElements.size(); ++instanceIndex)
//    {
//        SAnimatedSpriteData* data = someAnimatedElements[instanceIndex]->myData;
//        CSpriteInstance* instance = someAnimatedElements[instanceIndex]->mySpriteInstance;
//        CSprite* sprite = instance->GetSprite();
//
//        myObjectBufferData.myPosition = instance->GetPosition();
//        myObjectBufferData.myColor = instance->GetColor();
//        myObjectBufferData.myUVRect = instance->GetUVRect();
//        myObjectBufferData.mySize = instance->GetSize();
//        myObjectBufferData.myRotation = DirectX::XMConvertToRadians(instance->GetRotation());
//
//        BindBuffer(myObjectBuffer, myObjectBufferData, "Object Buffer");
//
//        float scrollingScale = 0.15f;
//        myTextureScrollingData.scrollSpeed1 = data->scrollSpeed1;
//        myTextureScrollingData.scrollSpeed2 = data->scrollSpeed2;
//        myTextureScrollingData.scrollSpeed3 = data->scrollSpeed3;
//        myTextureScrollingData.scrollSpeed4 = data->scrollSpeed4;
//        myTextureScrollingData.scrollSpeed5 = data->scrollSpeed5;
//        myTextureScrollingData.uvScale1 = data->uvScale1;
//        myTextureScrollingData.uvScale2 = data->uvScale2;
//        myTextureScrollingData.uvScale3 = data->uvScale3;
//        myTextureScrollingData.uvScale4 = data->uvScale4;
//        myTextureScrollingData.uvScale5 = data->uvScale5;
//        myTextureScrollingData.glowColor = data->glowColor;
//        myTextureScrollingData.glowWidth = data->glowWidth;
//        myTextureScrollingData.scrollTimer = CTimer::Time() * scrollingScale; //..is now!
//        myTextureScrollingData.level = someAnimatedElements[instanceIndex]->myLevel;
//        myTextureScrollingData.verticalDirectionOfChange = data->verticalDirectionOfChange;
//        myTextureScrollingData.maskOffset = data->maskOffset;
//        myTextureScrollingData.randomOffset = data->randomOffset;
//        BindBuffer(myTextureScrollingBuffer, myTextureScrollingData, "Texture Scrolling Buffer");
//
//        const CSprite::SSpriteData& spriteData = sprite->GetSpriteData();
//
//        myContext->IASetPrimitiveTopology(spriteData.myPrimitiveTopology);
//        myContext->IASetInputLayout(nullptr);
//        myContext->IASetVertexBuffers(0, 0, nullptr, nullptr, nullptr);
//        myContext->IASetIndexBuffer(nullptr, DXGI_FORMAT_UNKNOWN, 0);
//
//        myContext->VSSetConstantBuffers(0, 1, &myObjectBuffer);
//        myContext->VSSetShader(mySpriteVertexShader, nullptr, 0);
//
//        myContext->GSSetShader(mySpriteGeometryShader, nullptr, 0);
//
//        myContext->PSSetSamplers(0, 1, &spriteData.mySampler);
//        myContext->PSSetConstantBuffers(0, 1, &myObjectBuffer);
//        myContext->PSSetConstantBuffers(1, 1, &myTextureScrollingBuffer);
//        myContext->PSSetShaderResources(0, 5, &data->myTexture[0]);
//        myContext->PSSetShader(data->myPixelShader, nullptr, 0);
//
//        myContext->Draw(3, 0);
//        CRenderManager::myNumberOfDrawCallsThisFrame++;
//
//        //Reset Resources
//        ID3D11ShaderResourceView* nullView = NULL;
//        myContext->PSSetShaderResources(0, 1, &nullView);
//        myContext->PSSetShaderResources(1, 1, &nullView);
//        myContext->PSSetShaderResources(2, 1, &nullView);
//        myContext->PSSetShaderResources(3, 1, &nullView);
//        myContext->PSSetShaderResources(4, 1, &nullView);
//        myContext->PSSetShaderResources(5, 1, &nullView);
//        myContext->PSSetShaderResources(6, 1, &nullView);
//        myContext->PSSetShaderResources(7, 1, &nullView);
//        myContext->PSSetShaderResources(8, 1, &nullView);
//
//        myContext->GSSetShader(nullptr, nullptr, 0);
//    }
//}
