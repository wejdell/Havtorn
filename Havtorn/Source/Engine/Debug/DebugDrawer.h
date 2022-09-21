// Copyright 2022 Team Havtorn. All Rights Reserved.

namespace Havtorn
{
	namespace Debug
	{
		class GDebugDrawer
		{
			friend class GEngine;
		public:
			/*
				Color should be something other than an SVector4. Temp implementation.
			*/
			static HAVTORN_API void DrawLine(const SVector& start, const SVector& end, const SVector4& color/*temp*/);

		private:
			static HAVTORN_API GDebugDrawer* Instance;

			GDebugDrawer();
			~GDebugDrawer();

			static bool InstanceExists();

		};
	}
}