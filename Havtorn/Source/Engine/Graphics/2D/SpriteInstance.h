//#pragma once
//#include <SimpleMath.h>// Added for ModelViewer solution, gave error here... :/
//#include "PostMasterStructs.h"
//
//class CSprite;
//class CScene;
//
//enum class ERenderOrder {
//	//BackgroundLayer,
//	//Layer1,
//	//ForegroundLayer,
//	//PauseLayer
//	Layer0,
//	Layer1,
//	Layer2,
//	Layer3,
//	Layer4,
//	Layer5,
//	Layer6,
//
//	Count
//};
//
//struct SSpriteSheetPositionData
//{
//	float mySpriteWidth;
//	float mySpriteHeight;
//	float myVerticalStartingPosition;
//	float mySpeedInFramesPerSecond;
//	float myRotationSpeedInSeconds;
//	int myFramesOffset;
//	int myNumberOfFrames;
//	int myTransitionToIndex;
//	int myReverseTransitionToIndex;
//	bool myIsLooping;
//
//	std::string myAnimationName = "";
//};
//
//struct SSpriteAnimationData
//{
//	float myFramesPerSecond;
//	float myRotationSpeedInSeconds;
//	int myFramesOffset;
//	int myNumberOfFrames;
//	int myTransitionToIndex;
//	int myReverseTransitionToIndex;
//	bool myIsLooping;
//	std::string myAnimationName;
//};
//
//class CSpriteInstance
//{
//public:
//	CSpriteInstance();
//	~CSpriteInstance();
//
//	bool Init(CSprite* aSprite, const Vector2& aScale = {1.0f,1.0f});
//	bool Init(CSprite* aSprite, const std::vector<SSpriteSheetPositionData>& someSpriteSheetPositionData, const Vector2& aScale = { 1.0f, 1.0f });
//	void SetPosition(DirectX::SimpleMath::Vector2 aPosition);
//	void SetNormalPosition(DirectX::SimpleMath::Vector2 aPosition);
//	void SetColor(DirectX::SimpleMath::Vector4 aColor);
//	void SetUVRect(DirectX::SimpleMath::Vector4 aUVRect);
//	void SetSize(DirectX::SimpleMath::Vector2 aSize);
//	void SetRotation(float aRotation);
//	void Rotate(float aRotation);
//	void SetShouldRender(bool aBool);
//
//	void Update();
//
//	void PlayAnimation(unsigned int anIndex, bool aShouldLoop = false, bool aShouldBeReversed = false);
//	void PlayAnimationUsingInternalData(const int& anIndex, const bool& aShouldBeReversed = false);
//	void PlayAnimation(PostMaster::SCrossHairData& aData);
//	void PlayAnimation(std::string aName, bool aShouldLoop = false, bool aShouldBeReversed = false);
//
//public:
//	const DirectX::SimpleMath::Vector4 GetPosition() const { return myPosition; }
//	const DirectX::SimpleMath::Vector4 GetColor() const { return myColor; }
//	const DirectX::SimpleMath::Vector4 GetUVRect() const { return myUVRect; }
//	const DirectX::SimpleMath::Vector2 GetSize() const { return mySize; }
//	const float GetRotation() const { return myRotation; }
//	bool GetShouldRender() const { return myShouldRender; }
//	bool GetShouldAnimate() const { return myShouldAnimate; }
//	CSprite* GetSprite() const { return mySprite; }
//
//	inline const bool HasAnimations() const { return myAnimationFrames.size() > 0; }
//
//	ERenderOrder GetRenderOrder() const { return myRenderOrder; }
//	void SetRenderOrder(ERenderOrder aRenderOrder);
//
//private:
//	std::vector<Vector4> myAnimationFrames;
//	std::vector<SSpriteAnimationData> myAnimationData;
//	DirectX::SimpleMath::Vector4 myPosition = { 0.0f, 0.0f, 0.0f, 1.0f };
//	DirectX::SimpleMath::Vector4 myColor = { 1.0f, 1.0f, 1.0f, 1.0f };
//	DirectX::SimpleMath::Vector4 myUVRect = { 0.0f, 0.0f, 1.0f, 1.0f };
//	DirectX::SimpleMath::Vector2 mySize = { 1.0f, 1.0f };
//	float myRotation;
//	CSprite* mySprite;
//	ERenderOrder myRenderOrder;
//	float myAnimationTimer;
//	float myCurrentAnimationSpeed;
//	unsigned int myCurrentAnimationIndex;
//	int myCurrentAnimationFrame;
//	bool myShouldRender = true;
//	bool myShouldAnimate;
//	bool myShouldLoopAnimation;
//	bool myShouldReverseAnimation;
//};
//
