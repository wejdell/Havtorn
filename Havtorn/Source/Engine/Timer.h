// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once
#include <chrono>
#include "hvpch.h"

namespace Havtorn
{
#define NO_FRAMES_TO_AVERAGE 100

	enum class ETimerCategory
	{
		Frame,
		CPU,
		GPU
	};

	struct STimeData
	{
		std::chrono::steady_clock::time_point First;
		std::chrono::steady_clock::time_point Last;

		std::array<F32, NO_FRAMES_TO_AVERAGE> FrameTimes;

		F32 DeltaTime;
		F32 AverageFrameTime;

		F32 FixedTime;
		F32 FixedTimeInterval;

		U16 CurrentFrameTimeIndex;
		U16 FrameTimesLoaded;
	};

	class GTime 
	{
	public:
		HAVTORN_API static F32 Time(ETimerCategory category = ETimerCategory::Frame);
		HAVTORN_API static F32 Dt(ETimerCategory category = ETimerCategory::Frame);
		HAVTORN_API static F32 FixedDt(ETimerCategory category = ETimerCategory::Frame);
		HAVTORN_API static F32 AverageFrameRate(ETimerCategory category = ETimerCategory::Frame);

		GTime();
		~GTime();

		static F32 Mark(ETimerCategory category = ETimerCategory::Frame);

		static void BeginTracking(ETimerCategory category = ETimerCategory::Frame);
		static void EndTracking(ETimerCategory category = ETimerCategory::Frame);

		static bool FixedTimeStep(ETimerCategory category = ETimerCategory::Frame);

	private:
		float NewFrame(ETimerCategory category = ETimerCategory::Frame);
		float TotalTime(ETimerCategory category = ETimerCategory::Frame) const;

	private:
		static GTime* Instance;

		std::map<ETimerCategory, STimeData> Timers;
	};
}
