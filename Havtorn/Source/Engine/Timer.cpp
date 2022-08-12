// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "Timer.h"

namespace Havtorn
{
	GTimer* GTimer::Instance = nullptr;
	
	GTimer::GTimer()
	{
		Instance = this;
		Last = std::chrono::steady_clock::now();
		First = Last;

		FixedTime = 0.0f;
		FixedTimeInterval = 1.0f / 60.0f;

		DeltaTime = 0.0f;

		AverageFrameTime = 0.0f;
		FrameTimes.fill(0.0f);
		CurrentFrameTimeIndex = 0;
		FrameTimesLoaded = 0;
	}

	GTimer::~GTimer()
	{
		Instance = nullptr;
	}

	//Total duration in seconds since start
	float GTimer::Time()
	{
		return Instance->TotalTime();
	}

	//Delta time in seconds between the last two frames
	float GTimer::Dt()
	{
		return Instance->DeltaTime;
	}

	float GTimer::FixedDt()
	{
		return Instance->FixedTimeInterval;
	}

	F32 GTimer::AverageFrameRate()
	{
		return (1.0f / Instance->AverageFrameTime);
	}

	float GTimer::Mark()
	{
		return Instance->NewFrame();
	}

	bool GTimer::FixedTimeStep()
	{
		return Instance->FixedTime >= Instance->FixedTimeInterval;
	}

	float GTimer::NewFrame()
	{
		const auto old = Last;
		Last = std::chrono::steady_clock::now();
		const std::chrono::duration<float> dt = Last - old;
		DeltaTime = dt.count();

		if (FixedTime > FixedTimeInterval)
		{
			FixedTime -= FixedTimeInterval;
		}

#pragma region Average
		FrameTimes[CurrentFrameTimeIndex] = DeltaTime;
		CurrentFrameTimeIndex = (CurrentFrameTimeIndex + 1) % NO_FRAMES_TO_AVERAGE;
		if (FrameTimesLoaded < NO_FRAMES_TO_AVERAGE)
			FrameTimesLoaded++;

		F32 sum = 0.0f;
		for (U16 i = 0; i < FrameTimesLoaded; i++)
			sum += FrameTimes[i];

		AverageFrameTime = sum / FrameTimesLoaded;
#pragma endregion 

		FixedTime += DeltaTime;

		return DeltaTime;
	}

	float GTimer::TotalTime() const
	{
		return std::chrono::duration<float>(std::chrono::steady_clock::now() - First).count();
	}
}