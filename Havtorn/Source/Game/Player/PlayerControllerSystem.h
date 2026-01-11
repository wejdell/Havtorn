// Copyright 2026 Team Havtorn. All Rights Reserved.
#include <ECS/System.h>

namespace Havtorn
{
	class CScene;

	class CPlayerControllerSystem : public ISystem
	{
	public:
		CPlayerControllerSystem();
		~CPlayerControllerSystem() override ;
		void Update(std::vector<Ptr<CScene>>& scenes) override;

	private:

	};
}