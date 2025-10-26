// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once

namespace Havtorn
{
	class CScene;

	class ISystem
	{
	public:
		ISystem() = default;
		virtual ~ISystem() = default;

		virtual void Update(std::vector<Ptr<CScene>>& scenes) = 0;
	};
}
