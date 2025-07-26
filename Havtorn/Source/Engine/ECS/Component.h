// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once
#include "Entity.h"

namespace Havtorn
{
	class CScene;



	template<typename TValidate>
	concept IsValidatable = requires(const TValidate & toValidate)
	{
		{ toValidate.Validate() } -> std::convertible_to<bool>;
	};

	struct SComponentView
	{
	public:

		//TValidate must be a type with a method with the name and signature -> bool Valiate() const
		template<IsValidatable TValidate>
		explicit SComponentView(const TValidate& toValidate)
			: Component(&toValidate)
			, ValidateImpl	{ 
								[](const void *obj)
								{
									return static_cast<const TValidate*>(obj)->Validate();
								}
							}
		{ }

		bool Validate() const { return ValidateImpl(Component); }

	private:
		const void* Component;
		bool (*ValidateImpl)(const void*);

	};


	struct ENGINE_API SComponent
	{
		SComponent() = default;
		SComponent(const SEntity& entity);
		virtual ~SComponent() noexcept {};

		virtual void IsDeleted(CScene* fromScene);

		static bool IsValid(const SComponent* component);

		SEntity Owner = SEntity::Null;
	};
}
