// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once

#include "Delegate.h"

namespace Havtorn
{
	template<typename... BroadcastTypes>
	class CMulticastDelegate
	{
	public:
		CMulticastDelegate()
			: Delegate(MulticastDelegate<BroadcastTypes...>())
		{}

		~CMulticastDelegate() = default;
		CMulticastDelegate(const CMulticastDelegate&) = default;
		CMulticastDelegate(CMulticastDelegate&&) = default;
		CMulticastDelegate& operator=(const CMulticastDelegate&) = default;
		CMulticastDelegate& operator=(CMulticastDelegate&&) = default;

		bool IsBoundTo(const DelegateHandle& handle)
		{
			return Delegate.IsBoundTo(handle);
		}

		template<typename LambdaType, typename... LambdaArgs>
		DelegateHandle AddLambda(LambdaType&& lambda, LambdaArgs&&... args)
		{
			return Delegate.AddLambda(std::forward<LambdaType>(lambda), std::forward<LambdaArgs>(args)...);
		}

		template<typename ObjectType, typename FunctionReturnType>
		DelegateHandle AddMember(const ObjectType* object, FunctionReturnType(ObjectType::* function)(BroadcastTypes...) const)
		{
			return Delegate.AddRaw(object, function);
		}

		template<typename ObjectType, typename FunctionReturnType>
		DelegateHandle AddMember(ObjectType* object, FunctionReturnType(ObjectType::* function)(BroadcastTypes...))
		{
			return Delegate.AddRaw(object, function);
		}

		//AS. Didnt quite work, need to figure out how to specialize so we correctly resolve T to DelegateBase (?) /2026-01-02
		//template<typename ObjectType>
		//void RemoveObject(ObjectType* object, BroadcastTypes...)
		//{
		//	Delegate.RemoveObject(object);
		//}

		void RemoveHandle(DelegateHandle& handle)
		{
			Delegate.Remove(handle);
		}

		template<typename FunctionType, typename... FunctionArgs>
		DelegateHandle AddStatic(FunctionType&& function, FunctionArgs&&... args)
		{
			return Delegate.AddStatic(std::forward<FunctionType>(function), std::forward<FunctionArgs>(args)...);
		}

		void Broadcast(BroadcastTypes... args)
		{
			Delegate.Broadcast(std::forward<BroadcastTypes>(args)...);
		}

	public:
		MulticastDelegate<BroadcastTypes...> Delegate;
	};
}