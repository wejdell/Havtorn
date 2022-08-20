// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once
#include <chrono>
#include "hvpch.h"

namespace Havtorn
{
#define NO_FRAMES_TO_AVERAGE 100

	class GTimer 
	{
	public:
		HAVTORN_API static F32 Time();
		HAVTORN_API static F32 Dt();
		HAVTORN_API static F32 FixedDt();
		HAVTORN_API static F32 AverageFrameRate();

		GTimer();
		~GTimer();

		static F32 Mark();

		static bool FixedTimeStep();

	private:
		float NewFrame();
		float TotalTime() const;

	private:
		static GTimer* Instance;
		std::chrono::steady_clock::time_point First;
		std::chrono::steady_clock::time_point Last;
		
		std::array<F32, NO_FRAMES_TO_AVERAGE> FrameTimes;
		
		F32 DeltaTime;
		F32 FixedTime;
		F32 FixedTimeInterval;

		F32 AverageFrameTime;
		U16 CurrentFrameTimeIndex;
		U16 FrameTimesLoaded;
	};
}
