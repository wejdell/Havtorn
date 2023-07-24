//#include "stdafx.h"
//#include "SpriteInstance.h"
//#include "Sprite.h"
//#include "Scene.h"
//#include "Engine.h"
//#include "WindowHandler.h"
//
//CSpriteInstance::CSpriteInstance()
//	: mySprite(nullptr)
//	, myRenderOrder(ERenderOrder::Layer0)
//	, myShouldRender(true)
//	, myShouldAnimate(false)
//	, myAnimationTimer(0.0f)
//	, myCurrentAnimationIndex(0)
//	, myCurrentAnimationFrame(0)
//	, myCurrentAnimationSpeed(1.0f / 60.0f)
//	, myRotation(0.0f)
//	, myShouldLoopAnimation(false)
//	, myShouldReverseAnimation(false)
//{
//}
//
//CSpriteInstance::~CSpriteInstance()
//{
//}
//
//bool CSpriteInstance::Init(CSprite* aSprite, const Vector2& aScale)
//{
//	if (!aSprite) {
//		return false;
//	}
//
//	mySprite = aSprite;
//	this->SetSize(aScale);
//	myRenderOrder = ERenderOrder::Layer2;
//
//	return true;
//}
//
//bool CSpriteInstance::Init(CSprite* aSprite, const std::vector<SSpriteSheetPositionData>& someSpriteSheetPositionData, const Vector2& aScale)
//{
//	if (!aSprite) {
//		return false;
//	}
//
//	mySprite = aSprite;
//	this->SetSize(aScale);
//	myRenderOrder = ERenderOrder::Layer2;
//
//	Vector2 sheetDimensions = aSprite->GetSpriteData().myDimensions;
//
//	for (unsigned int i = 0; i < someSpriteSheetPositionData.size(); ++i)
//	{
//		myAnimationData.emplace_back();
//		auto& animationDataToLoad = someSpriteSheetPositionData[i];
//		SSpriteAnimationData& spriteData = myAnimationData.back();
//		spriteData.myAnimationName = animationDataToLoad.myAnimationName;
//		spriteData.myFramesOffset = static_cast<int>(myAnimationFrames.size());
//		spriteData.myNumberOfFrames = animationDataToLoad.myNumberOfFrames;
//		spriteData.myFramesPerSecond = animationDataToLoad.mySpeedInFramesPerSecond;
//		spriteData.myRotationSpeedInSeconds = animationDataToLoad.myRotationSpeedInSeconds;
//
//		spriteData.myIsLooping = animationDataToLoad.myIsLooping;
//		spriteData.myTransitionToIndex = animationDataToLoad.myTransitionToIndex;
//		spriteData.myReverseTransitionToIndex = animationDataToLoad.myReverseTransitionToIndex;
//
//		for (unsigned int j = 0; j < static_cast<unsigned int>(animationDataToLoad.myNumberOfFrames); ++j)
//		{
//			myAnimationFrames.emplace_back();
//			auto& currentUV = myAnimationFrames.back();
//			currentUV.x = (animationDataToLoad.mySpriteWidth * (j + animationDataToLoad.myFramesOffset)) / sheetDimensions.x; // Left
//			currentUV.y = (animationDataToLoad.myVerticalStartingPosition) / sheetDimensions.y; // Up
//			currentUV.z = (animationDataToLoad.mySpriteWidth * (j + animationDataToLoad.myFramesOffset + 1)) / sheetDimensions.x; // Right
//			currentUV.w = (animationDataToLoad.myVerticalStartingPosition + animationDataToLoad.mySpriteHeight) / sheetDimensions.y; // Down
//		}
//	}
//
//	if (!myAnimationFrames.empty())
//	{
//		Vector2 frameSize = { someSpriteSheetPositionData.back().mySpriteWidth,  someSpriteSheetPositionData.back().mySpriteHeight };
//		Vector2 scaleProportions = (frameSize / sheetDimensions);
//		this->SetSize(aScale * scaleProportions);
//
//		myCurrentAnimationIndex = 0;
//		myShouldAnimate = true;
//		myShouldLoopAnimation = myAnimationData[myCurrentAnimationIndex].myIsLooping;
//		
//		this->SetUVRect(myAnimationFrames[0]);
//	}
//	
//	return true;
//}
//
//void CSpriteInstance::SetSize(DirectX::SimpleMath::Vector2 aSize)
//{
//	//Sprites are assumed to be the correct native size in a 1920x1080 resolution
//	CSprite::SSpriteData mySpriteData = mySprite->GetSpriteData();
//	mySize = mySpriteData.myDimensions;
//	mySize /= 1080.0f;
//	mySize *= aSize;
//}
//
//void CSpriteInstance::SetRotation(float aRotation)
//{
//	myRotation = aRotation;
//	if (myRotation < 0.0f)
//		myRotation = 360.0f - myRotation;
//	if (myRotation > 360.0f)
//		myRotation = 0.0f + myRotation;
//}
//
//void CSpriteInstance::Rotate(float aRotation)
//{
//	myRotation += aRotation;
//}
//
//void CSpriteInstance::SetShouldRender(bool aBool)
//{
//	myShouldRender = aBool;
//}
//
//void CSpriteInstance::Update()
//{
//	if (!myShouldAnimate)
//		return;
//
//	if (!myShouldReverseAnimation)
//		this->Rotate(myAnimationData[myCurrentAnimationIndex].myRotationSpeedInSeconds * CTimer::Dt());
//	else 
//		this->Rotate(-myAnimationData[myCurrentAnimationIndex].myRotationSpeedInSeconds * CTimer::Dt());
//
//	if ((myAnimationTimer += CTimer::Dt()) > (1.0f / myAnimationData[myCurrentAnimationIndex].myFramesPerSecond))
//	{
//		myAnimationTimer = 0.0f; // doing it properly doesn't seem to work, as CTimer is not marked at the start of this state
//		
//		if (!myShouldReverseAnimation)
//		{
//			myCurrentAnimationFrame++;
//			if (myCurrentAnimationFrame > (myAnimationData[myCurrentAnimationIndex].myNumberOfFrames + myAnimationData[myCurrentAnimationIndex].myFramesOffset - 1))
//			{
//				myShouldAnimate = myShouldLoopAnimation;
//				if (!myShouldAnimate)
//				{
//					PlayAnimationUsingInternalData(myAnimationData[myCurrentAnimationIndex].myTransitionToIndex);
//					this->SetUVRect(myAnimationFrames[myCurrentAnimationFrame]);
//					return;
//				}
//
//				myCurrentAnimationFrame = myAnimationData[myCurrentAnimationIndex].myFramesOffset;
//			}
//		}
//		else 
//		{
//			myCurrentAnimationFrame--;
//			if (myCurrentAnimationFrame < myAnimationData[myCurrentAnimationIndex].myFramesOffset)
//			{
//				myShouldAnimate = myShouldLoopAnimation;
//				
//				if (!myShouldAnimate)
//				{
//					PlayAnimationUsingInternalData(myAnimationData[myCurrentAnimationIndex].myReverseTransitionToIndex);
//					this->SetUVRect(myAnimationFrames[myCurrentAnimationFrame]);
//					return;
//				}
//				
//				myCurrentAnimationFrame = (myAnimationData[myCurrentAnimationIndex].myNumberOfFrames + myAnimationData[myCurrentAnimationIndex].myFramesOffset - 1);
//			}
//		}
//
//		this->SetUVRect(myAnimationFrames[myCurrentAnimationFrame]);
//	}
//}
//
//void CSpriteInstance::PlayAnimation(unsigned int anIndex, bool aShouldLoop, bool aShouldBeReversed)
//{
//	if (anIndex >= myAnimationData.size() || anIndex < 0)
//		return;
//
//	myCurrentAnimationIndex = anIndex;
//	myShouldLoopAnimation = aShouldLoop;
//	myShouldReverseAnimation = aShouldBeReversed;
//	myAnimationTimer = 0.0f;
//
//	if (!myShouldReverseAnimation)
//		myCurrentAnimationFrame = myAnimationData[myCurrentAnimationIndex].myFramesOffset;
//	else
//		myCurrentAnimationFrame = (myAnimationData[myCurrentAnimationIndex].myNumberOfFrames + myAnimationData[myCurrentAnimationIndex].myFramesOffset - 1);
//
//	myShouldAnimate = true;
//}
//
//void CSpriteInstance::PlayAnimationUsingInternalData(const int& anIndex, const bool& aShouldBeReversed)
//{
//	if (anIndex >= myAnimationData.size() || anIndex < 0)
//		return;
//
//	myCurrentAnimationIndex = anIndex;
//	myShouldLoopAnimation = myAnimationData[anIndex].myIsLooping;
//	myShouldReverseAnimation = aShouldBeReversed;
//	myAnimationTimer = 0.0f;
//
//	if (!myShouldReverseAnimation)
//		myCurrentAnimationFrame = myAnimationData[myCurrentAnimationIndex].myFramesOffset;
//	else
//		myCurrentAnimationFrame = (myAnimationData[myCurrentAnimationIndex].myNumberOfFrames + myAnimationData[myCurrentAnimationIndex].myFramesOffset - 1);
//
//	myShouldAnimate = true;
//}
//
//void CSpriteInstance::PlayAnimation(PostMaster::SCrossHairData& /*aData*/)
//{
//	//PlayAnimation(aData.myIndex, aData.myShouldLoop, aData.myShouldLoop);
//}
//
//void CSpriteInstance::PlayAnimation(std::string aName, bool aShouldLoop, bool aShouldBeReversed)
//{
//	unsigned int newIndex = 99999;
//	for (unsigned int i = 0; i < myAnimationData.size(); ++i)
//	{
//		if (myAnimationData[i].myAnimationName.find(aName) != std::string::npos)
//		{
//			newIndex = i;
//			break;
//		}
//	}
//
//	if (newIndex >= myAnimationData.size())
//		return;
//
//	myCurrentAnimationIndex = newIndex;
//	myShouldLoopAnimation = aShouldLoop;
//	myShouldReverseAnimation = aShouldBeReversed;
//
//	if (!myShouldReverseAnimation)
//		myCurrentAnimationFrame = myAnimationData[myCurrentAnimationIndex].myFramesOffset;
//	else
//		myCurrentAnimationFrame = (myAnimationData[myCurrentAnimationIndex].myNumberOfFrames + myAnimationData[myCurrentAnimationIndex].myFramesOffset - 1);
//
//	myShouldAnimate = true;
//}
//
//void CSpriteInstance::SetRenderOrder(ERenderOrder aRenderOrder)
//{
//	myRenderOrder = aRenderOrder;
//}
//
///// <summary>
/////The y-position is flipped in this function so that we go from Shader space
/////([-1, -1] in the lower left corner) to Shader space with [-1, -1] in the
///// upper left corner. I think this is more intuitive, but might as well
///// revert this.
///// </summary>
///// <param name="aPosition"></param>
//void CSpriteInstance::SetPosition(DirectX::SimpleMath::Vector2 aPosition)
//{
//	myPosition.x = aPosition.x * (16.0f / 9.0f); // Did not feel like a problem before, is needed for UI / Nico 12/4 - 2021
//	myPosition.y = -aPosition.y;
//}
//
//void CSpriteInstance::SetNormalPosition(DirectX::SimpleMath::Vector2 aPosition)
//{
//	//aPosition /= 2.0f;
//	//aPosition.x += 0.5f;
//	//aPosition.y += 0.5f;
//	myPosition.x = aPosition.x /** CEngine::GetInstance()->GetWindowHandler()->GetResolution().x*/;
//	myPosition.y = aPosition.y /** CEngine::GetInstance()->GetWindowHandler()->GetResolution().y*/;
//}
//
//void CSpriteInstance::SetColor(DirectX::SimpleMath::Vector4 aColor)
//{
//	myColor = aColor;
//}
//
//void CSpriteInstance::SetUVRect(DirectX::SimpleMath::Vector4 aUVRect)
//{
//	myUVRect = aUVRect;
//}
