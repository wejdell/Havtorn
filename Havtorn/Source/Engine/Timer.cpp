// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "Timer.h"

namespace Havtorn
{
	GTime* GTime::Instance = nullptr;
	
	GTime::GTime()
	{
		Instance = this;
		Timers[ETimerCategory::Frame] = {};
		Timers[ETimerCategory::Frame].Last = std::chrono::steady_clock::now();
		Timers[ETimerCategory::Frame].First = Timers[ETimerCategory::Frame].Last;
		
		Timers[ETimerCategory::Frame].FixedTime = 0.0f;
		Timers[ETimerCategory::Frame].FixedTimeInterval = 1.0f / 60.0f;
		
		Timers[ETimerCategory::Frame].DeltaTime = 0.0f;
		
		Timers[ETimerCategory::Frame].AverageFrameTime = 0.0f;
		Timers[ETimerCategory::Frame].FrameTimes.fill(0.0f);
		Timers[ETimerCategory::Frame].CurrentFrameTimeIndex = 0;
		Timers[ETimerCategory::Frame].FrameTimesLoaded = 0;
		
		Timers[ETimerCategory::CPU] = {};
		Timers[ETimerCategory::CPU].Last = std::chrono::steady_clock::now();
		Timers[ETimerCategory::CPU].First = Timers[ETimerCategory::CPU].Last;

		Timers[ETimerCategory::CPU].FixedTime = 0.0f;
		Timers[ETimerCategory::CPU].FixedTimeInterval = 1.0f / 60.0f;

		Timers[ETimerCategory::CPU].DeltaTime = 0.0f;

		Timers[ETimerCategory::CPU].AverageFrameTime = 0.0f;
		Timers[ETimerCategory::CPU].FrameTimes.fill(0.0f);
		Timers[ETimerCategory::CPU].CurrentFrameTimeIndex = 0;
		Timers[ETimerCategory::CPU].FrameTimesLoaded = 0;
		
		Timers[ETimerCategory::GPU] = {};
		Timers[ETimerCategory::GPU].Last = std::chrono::steady_clock::now();
		Timers[ETimerCategory::GPU].First = Timers[ETimerCategory::GPU].Last;

		Timers[ETimerCategory::GPU].FixedTime = 0.0f;
		Timers[ETimerCategory::GPU].FixedTimeInterval = 1.0f / 60.0f;

		Timers[ETimerCategory::GPU].DeltaTime = 0.0f;

		Timers[ETimerCategory::GPU].AverageFrameTime = 0.0f;
		Timers[ETimerCategory::GPU].FrameTimes.fill(0.0f);
		Timers[ETimerCategory::GPU].CurrentFrameTimeIndex = 0;
		Timers[ETimerCategory::GPU].FrameTimesLoaded = 0;
	}

	GTime::~GTime()
	{
		Instance = nullptr;
	}

	//Total duration in seconds since start
	float GTime::Time(ETimerCategory category)
	{
		return Instance->TotalTime(category);
	}

	//Delta time in seconds between the last two frames
	float GTime::Dt(ETimerCategory category)
	{
		return Instance->Timers[category].DeltaTime;
	}

	float GTime::FixedDt(ETimerCategory category)
	{
		return Instance->Timers[category].FixedTimeInterval;
	}

	F32 GTime::AverageFrameRate(ETimerCategory category)
	{
		return (1.0f / Instance->Timers[category].AverageFrameTime);
	}

	float GTime::Mark(ETimerCategory category)
	{
		return Instance->NewFrame(category);
	}

	void GTime::BeginTracking(ETimerCategory category)
	{
		Instance->Timers[category].Last = std::chrono::steady_clock::now();
	}

	void GTime::EndTracking(ETimerCategory category)
	{
		Instance->NewFrame(category);
	}

	bool GTime::FixedTimeStep(ETimerCategory category)
	{
		return Instance->Timers[category].FixedTime >= Instance->Timers[category].FixedTimeInterval;
	}

	float GTime::NewFrame(ETimerCategory category)
	{
		STimeData& timer = Timers[category];

		const auto old = timer.Last;
		timer.Last = std::chrono::steady_clock::now();
		const std::chrono::duration<float> dt = timer.Last - old;
		timer.DeltaTime = dt.count();

		if (timer.FixedTime > timer.FixedTimeInterval)
		{
			timer.FixedTime -= timer.FixedTimeInterval;
		}

#pragma region Average
		timer.FrameTimes[timer.CurrentFrameTimeIndex] = timer.DeltaTime;
		timer.CurrentFrameTimeIndex = (timer.CurrentFrameTimeIndex + 1) % NO_FRAMES_TO_AVERAGE;
		if (timer.FrameTimesLoaded < NO_FRAMES_TO_AVERAGE)
			timer.FrameTimesLoaded++;

		F32 sum = 0.0f;
		for (U16 i = 0; i < timer.FrameTimesLoaded; i++)
			sum += timer.FrameTimes[i];

		timer.AverageFrameTime = sum / timer.FrameTimesLoaded;
#pragma endregion 

		timer.FixedTime += timer.DeltaTime;

		return timer.DeltaTime;
	}

	float GTime::TotalTime(ETimerCategory category) const
	{
		return std::chrono::duration<float>(std::chrono::steady_clock::now() - Instance->Timers[category].First).count();
	}
}