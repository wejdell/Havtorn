// Copyright 2022 Team Havtorn. All Rights Reserved.

// Copyright 2023 Team Havtorn. All Rights Reserved.

#pragma once
#include "ECS/Component.h"

namespace Havtorn
{
	//hur gör vi en Transition från en grupp till en annan

	//NaIVE: -> Byter instantly. på framen.

	//Controlled Transition -> Byt istället när Duration är färdig.
	//Kan ta med oss information från the Transition och baserat på
	//datan vi får från transitionen välja en ny start Frame som passar
	//bättre med förra framen

	struct SSpriteAnimationClip
	{
		std::vector<SVector4> UVRects;
		std::vector<float> Durations;
		bool IsLooping = false;
	};

	struct SSpriteAnimatorGraphNode //This is basically a Switch
	{
		U32 AnimationClipKey;
		std::vector<SSpriteAnimatorGraphNode> Nodes;
	};

	struct SSpriteAnimatorGraphComponent : public SComponent
	{
		SSpriteAnimatorGraphComponent()
			: SComponent(EComponentType::SpriteAnimatorGraphComponent)
		{
		}

		SSpriteAnimatorGraphNode Graph;
		U32 CurrentFrame = 0;
		U32 CurrentAnimationClipKey = 0;
		U32 ResolvedAnimationClipKey = 0;
		F32 Time = 0.0f;
		std::vector<SSpriteAnimationClip> AnimationClips;

		/*

		 // Om vi inte har "köat" ett byte-
			 //"Vi resolvar vilket klipp vi ska byta till"

		 //Vi har ett klipp vi spelar just nu
			//Vi ska byta state,
			if(TimeSinceStartup >= TimeWhenWeChangedToThisClip + Durations[CurrentIndex])
			{
				CurrentIndex = CurrentIndex + 1 % AnimationClips[CurrentAnimationClipKey].Size();

				if(CurrentAnimationClipKey != ResolvedAnimationClipKey)
				{
					CurrentIndex = 0; // Left Right foot or upsidedown or whatever>:)
				}

				CurrentAnimationClipKey = ResolvedAnimationClipKey;
			}

			uvRect = AnimationClips[CurrentAnimationClipKey].UVRects[CurrentIndex];

//






		ResolvedAnimationClipKey = graph.Resolve();

		if(parameters.HasChanged())
		{
			if(resolvedAnimationClipKey != CurrentAnimationClipKey)
			{

			}
		}

		*/

		//Map<U32, SSpriteAnimationClip> AnimationClipMap;

	};
}



//struct SSpriteAnimationClip
//{
//	std::vector<SVector4> UVRects;
//	std::vector<float> Durations;
//	bool IsLooping = false;
//};

//struct SSpriteAnimatorState
//{
//	std::vector<SSpriteAnimatorTransition> Transitions;
//	CHavtornStaticString<128> Name;
//	SSpriteAnimationClip AnimationClip;
//	float Speed = 1.0f;
//	float Time = 0.0f;
//};

//struct SSpriteAnimatorTransition
//{
//	std::vector<std::function<bool()>> Conditions;
//	CHavtornStaticString<128> TargetState;
//};
