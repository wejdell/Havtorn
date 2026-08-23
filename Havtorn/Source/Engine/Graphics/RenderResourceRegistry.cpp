// Copyright 2026 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "RenderResourceRegistry.h"

namespace Havtorn
{
	SRenderResourceHandle::SRenderResourceHandle(const CHavtornStaticString<32>& /*name*/)
	{
	}

	SRenderResource::SRenderResource(const SRenderResourceDescription& /*description*/)
	{
	}

	void SRenderResource::Construct()
	{
	}

	void SRenderResource::Bind()
	{
	}

	void CRenderResourceRegistry::Allocate()
	{
	}

	void CRenderResourceRegistry::Bind(std::vector<const SRenderResourceHandle>& /*handles*/)
	{
	}
}
