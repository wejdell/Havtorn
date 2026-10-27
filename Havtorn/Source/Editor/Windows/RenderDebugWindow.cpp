// Copyright 2026 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "RenderDebugWindow.h"
#include "EditorManager.h"
#include "EditorResourceManager.h"

#include <Graphics/RenderManager.h>
#include <Graphics/RenderGraph.h>
#include <ECS/Systems/CameraSystem.h>
#include <ECS/Components/MetaDataComponent.h>

namespace Havtorn
{
	CRenderDebugWindow::CRenderDebugWindow(const char* displayName, CEditorManager* manager)
		: CWindow(displayName, manager, false)
	{
		RenderManager = manager->GetRenderManager();
	}

	void CRenderDebugWindow::OnEnable()
	{
	}

	void CRenderDebugWindow::OnInspectorGUI()
	{
		// TODO.NW: Make ON_SCOPE_EXIT equivalent?

		if (!GUI::Begin(Name(), &IsEnabled))
		{
			GUI::End();
			return;
		}

		IsHovered = IsEnabled && GUI::IsWindowHovered();

		const bool isWindowHovered = GUI::IsWindowHovered();
		if (!IsHoveringWindow && isWindowHovered)
			GEngine::GetWorld()->BlockSystem<CCameraSystem>(this);
		else if (IsHoveringWindow && !isWindowHovered)
			GEngine::GetWorld()->UnblockSystem<CCameraSystem>(this);

		IsHoveringWindow = isWindowHovered;

		if (!RenderManager->HasRenderView(CurrentViewID))
			CurrentViewID = 0;

		bool madeNewSelection = false;
		{ // Menu Bar
			GUI::BeginChild("RenderWindowMenuBar", SVector2<F32>(0.0f, 60.0f));
			if (GUI::BeginCombo("Render View", ResolveLabel(CurrentViewID).c_str(), {}))
			{
				std::map<U64, SRenderView>* renderViews = RenderManager->GetRenderViews();
				if (renderViews != nullptr)
				{
					for (auto& [id, view] : *renderViews)
					{
						const bool isSelected = id == CurrentViewID;

						if (GUI::Selectable(ResolveLabel(id).c_str(), isSelected))
						{
							madeNewSelection = CurrentViewID != id;
							CurrentViewID = id;
						}

						if (isSelected)
							GUI::SetItemDefaultFocus();
					}
				}
				GUI::EndCombo();
			}
			Filter.Draw("Search", 180);

			GUI::Separator();
			GUI::EndChild();
		}

		DrawRegistry();

		GUI::BeginScript("Render Graph View");
		
		DrawGraph();

		if (madeNewSelection)
		{
			PlaceNodes();
			GUI::NavigateToScriptContent();
		}
		
		GUI::EndScript();
		GUI::End();
	}

	void CRenderDebugWindow::OnDisable()
	{
		GEngine::GetWorld()->UnblockSystem<CCameraSystem>(this);
	}

	void CRenderDebugWindow::DrawRegistry() const
	{
		SRenderView* renderView = RenderManager->GetRenderView(CurrentViewID);
		if (renderView == nullptr)
			return;

		CRenderResourceRegistry& resourceRegistry = RenderManager->GetRenderResourceRegistry();

		GUI::BeginChild("ResourceRegistry", SVector2<F32>(150.0f, 0.0f), { EChildFlag::Borders, EChildFlag::ResizeX });
		GUI::Text("Render Resources");
		GUI::Separator();

		// TODO.NW: Set sizes of all render resources, including imported ones (so they show up correctly here).
		// TODO.NW: Add some sort of highlighting mechanism to highlight where the resources are used in the graph. Maybe also on hover?
		// TODO.NW: Write out referencing passes directly in the tooltip, if possible.

		const SVector2<F32> rectSize = SVector2<F32>(14.0f, 14.0f);
		for (const SRenderResource& resource : resourceRegistry.GetResources())
		{
			GUI::AddRectFilled(GUI::GetCursorScreenPos(), rectSize, GetResourceColor(&resource));
			GUI::OffsetCursorPos({ rectSize.X + 4.0f, 0.0f });
			GUI::Text(resource.Description.Name.AsString().c_str());

			if (GUI::IsItemHovered())
			{
				std::string tooltip = resource.Description.Name.AsString()
					+ "\n"
					+ "ID: " + std::to_string(resource.Description.ID)
					+ "\n"
					+ "Type: " + magic_enum::enum_name(resource.Description.Type).data()
					+ "\n"
					+ "Is Imported: " + (resource.Description.IsImported ? "True" : "False")
					+ "\n"
					+ "Size: " + (resource.IsTextureType() ? (std::to_string(resource.Description.Size2D.X) + "x" + std::to_string(resource.Description.Size2D.Y)) : std::to_string(resource.Description.Size));
				GUI::SetTooltip(tooltip.c_str());
			}
		}

		GUI::EndChild();
		GUI::SameLine();
	}

	void CRenderDebugWindow::DrawGraph() const
	{
		SRenderView* renderView = RenderManager->GetRenderView(CurrentViewID);
		if (renderView == nullptr)
			return;

		CRenderResourceRegistry& resourceRegistry = RenderManager->GetRenderResourceRegistry();

		GUI::PushScriptStyleColor(EScriptStyleColor::Background, SColor(60));
			
		SVector2<F32> positionTracker = SVector2<F32>::Zero;
		const std::vector<Ref<IRenderPass>>& allPasses = renderView->RenderGraph.GetAllPasses();
		const std::vector<Ref<IRenderPass>>& sortedPasses = renderView->RenderGraph.GetSortedPasses();
		std::vector<Ref<IRenderPass>> culledPasses;
		for (const Ref<IRenderPass>& pass : allPasses)
		{
			if (std::ranges::find_if(sortedPasses, [pass](const Ref<IRenderPass>& sortedPass) { return pass->PassParamHash == sortedPass->PassParamHash; }) == sortedPasses.end())
				culledPasses.push_back(pass);
		}
		for (const Ref<IRenderPass>& pass : sortedPasses)
		{
			DrawPass(pass, resourceRegistry, false);
		}
		for (const Ref<IRenderPass>& pass : culledPasses)
		{
			DrawPass(pass, resourceRegistry, true);
		}

		//for (const SLink& link : CurrentScript->Links)
		//{
		//	SPin* startPin = GetPinFromID(link.StartPinUID, CurrentScript->Nodes);
		//	GUI::Link(link.UID, link.StartPinUID, link.EndPinUID, startPin != nullptr ? GetPinTypeColor(startPin->Type) : SColor::White);
		//}
	}

	void CRenderDebugWindow::DrawPass(const Ref<IRenderPass>& pass, CRenderResourceRegistry& registry, const bool isCulled) const
	{
		const SVector2<F32> requiredSize = GetNodeSize(pass, registry);
		GUI::PushScriptStyleVar(EScriptStyleVar::NodePadding, SVector4(8.0f, 4.0f, 8.0f, 8.0f));
		GUI::BeginNode(pass->PassParamHash);
		
		GUI::PushID(pass->PassParamHash);
		GUI::BeginVertical("node", requiredSize);

		GUI::BeginHorizontal("header", SVector2<F32>(requiredSize.X, HeaderHeight));
		SVector2<F32> nodeNameCursorStart = GUI::GetCursorPos();
		nodeNameCursorStart.Y += 2.0f;
		GUI::SetCursorPos(nodeNameCursorStart);
		GUI::TextUnformatted(pass->GetName().c_str());
		GUI::EndHorizontal();

		const SVector4 headerRect = GUI::GetLastRect();
		GUI::Spring(0, GUI::GetStyleVar(EStyleVar::ItemSpacing).Y * 3.0f);

		const U64 maxHandleColumnLength = UMath::Max(pass->Inputs.size(), pass->Outputs.size());
		for (U64 i = 0; i < maxHandleColumnLength; i++)
		{
			SRenderResourceHandle* inputResourceHandle = pass->Inputs.size() > i ? &pass->Inputs[i] : nullptr;
			SRenderResourceHandle* outputResourceHandle = pass->Outputs.size() > i ? &pass->Outputs[i] : nullptr;

			if (inputResourceHandle != nullptr)
			{
				GUI::PushScriptStyleVar(EScriptStyleVar::PivotAlignment, SVector2<F32>(0.1f, 0.5f));
				GUI::BeginPin(pass->PassParamHash + inputResourceHandle->ID, EGUIPinDirection::Input);

				SRenderResource* resource = registry.GetResource(*inputResourceHandle);
				const bool isPinLinked = /*IsPinLinked(inputResourceHandle->UID, CurrentScript->Links)*/true;
				GUI::DrawPinIcon(SVector2<F32>(24.0f), EGUIIconType::Circle, isPinLinked, isCulled ? SColor::Black : GetResourceColor(resource), false);

				GUI::SameLine(0, 0);
				const F32 cursorY = GUI::GetCursorPosY();
				GUI::SetCursorPosY(cursorY + PinNameOffset);
				GUI::Text(resource->Description.Name.Data());
				GUI::SetCursorPosY(cursorY - PinNameOffset);

				GUI::EndPin();
				GUI::PopScriptStyleVar();
			}

			if (outputResourceHandle != nullptr)
			{
				if (inputResourceHandle != nullptr)
					GUI::SameLine();

				constexpr F32 iconSize = 24.0f;

				SRenderResource* resource = registry.GetResource(*outputResourceHandle);
				const std::string resourceName = resource->Description.Name.AsString();
				const F32 nameWidth = GUI::CalculateTextSize(resourceName.c_str()).X;
				const F32 indent = requiredSize.X - nameWidth - iconSize;
				GUI::Indent(indent);

				GUI::PushScriptStyleVar(EScriptStyleVar::PivotAlignment, SVector2<F32>(0.9f, 0.5f));
				GUI::BeginPin(pass->PassParamHash + outputResourceHandle->ID, EGUIPinDirection::Output);

				const F32 cursorX = GUI::GetCursorPosX();
				const F32 cursorY = GUI::GetCursorPosY();
				GUI::SetCursorPosY(cursorY + PinNameOffset);
				GUI::Text(resourceName.c_str());
				GUI::SetCursorPos(SVector2<F32>(cursorX + nameWidth, cursorY));

				const bool isPinLinked = /*IsPinLinked(inputResourceHandle->UID, CurrentScript->Links)*/true;
				GUI::DrawPinIcon(SVector2<F32>(24.0f), EGUIIconType::Circle, isPinLinked, isCulled ? SColor::Black : GetResourceColor(resource), false);

				GUI::EndPin();
				GUI::PopScriptStyleVar();
				GUI::Unindent(indent);
			}
		}

		GUI::EndVertical();
		GUI::EndNode();

		if (GUI::IsItemVisible())
		{
			constexpr F32 passRounding = 2.5f;
			constexpr F32 passBorderWidth = 1.5f;
			const F32 halfBorderWidth = passBorderWidth * 0.5f;
			const SVector2<F32> uv = SVector2<F32>(
				(headerRect.Z - headerRect.X) / (F32)(4.0f * 64.0f),//width
				(headerRect.W - headerRect.Y) / (F32)(4.0f * 64.0f));//height

			const SVector2<F32> imagePadding = SVector2<F32>(8 - halfBorderWidth, 4 - halfBorderWidth);
			const SVector2<F32> imagePaddingMax = SVector2<F32>(8 - halfBorderWidth, 10 - halfBorderWidth);
			const SVector2<F32> imageMin = SVector2<F32>(headerRect.X - imagePadding.X, headerRect.Y - imagePadding.Y);
			const SVector2<F32> imageMax = SVector2<F32>(headerRect.Z + imagePaddingMax.X, headerRect.W + imagePaddingMax.Y);
			GUI::DrawNodeHeader(pass->PassParamHash, Manager->GetResourceManager()->GetStaticEditorTextureResource(EEditorTexture::NodeBackground), imageMin, imageMax, SVector2<F32>(0.0f), uv, isCulled ? SColor::Grey : SColor::Orange, passRounding);
		}
		GUI::PopID();
		GUI::PopScriptStyleVar();
	}

	void CRenderDebugWindow::PlaceNodes() const
	{
		SRenderView* renderView = RenderManager->GetRenderView(CurrentViewID);
		if (renderView == nullptr)
			return;
		
		CRenderResourceRegistry& resourceRegistry = RenderManager->GetRenderResourceRegistry();
		const std::vector<Ref<IRenderPass>>& allPasses = renderView->RenderGraph.GetAllPasses();
		SVector2<F32> positionTracker = SVector2<F32>::Zero;
		for (U64 i = 0; i < allPasses.size(); i++)
		{
			const Ref<IRenderPass>& pass = allPasses[i];
			const SVector2<F32> nodeSize = GetNodeSize(pass, resourceRegistry);
			SVector2<F32> newNodePosition = SVector2<F32>::Zero;
			SVector2<F32> newTrackedPosition = SVector2<F32>::Zero;

			if (i != 0)
			{
				const Ref<IRenderPass>& previousPass = allPasses[i - 1];
				//const SVector2<F32> scriptWindowSize = GUI::GetScriptWindowSize();
				//const SVector2<F32> aspectRatioDivider = { 1.0f / (scriptWindowSize.Y / scriptWindowSize.X), 1.0f / (scriptWindowSize.Y) };
				const SVector2<F32> aspectRatioDivider = { 9.0f / 16.0f, 1.0f };
				const SVector2<F32> previousNodeSize = GetNodeSize(previousPass, resourceRegistry) * aspectRatioDivider;
				if (!pass->HasDependency(previousPass))
				{
					newNodePosition = positionTracker - previousNodeSize;
					newTrackedPosition = positionTracker;
				}
				else
				{
					newNodePosition = positionTracker + SVector2<F32>(previousNodeSize.X, 0.0f);
					newTrackedPosition = newNodePosition;
				}
			}
				
			GUI::SetNodePosition(pass->PassParamHash, newNodePosition);
			positionTracker = newTrackedPosition;
		}
		
	}

	std::string CRenderDebugWindow::ResolveLabel(const U64& renderViewID) const
	{
		const CScene* currentWorkingScene = Manager->GetCurrentWorkingScene();

		std::string renderViewLabel;
		if (currentWorkingScene != nullptr)
		{
			if (SMetaDataComponent* metaDataComponent = currentWorkingScene->GetComponent<SMetaDataComponent>(SEntity(renderViewID)))
				renderViewLabel = metaDataComponent->Name.AsString();
		}

		if (renderViewLabel.empty())
			renderViewLabel = std::to_string(CurrentViewID);

		return renderViewLabel;
	}

	SVector2<F32> CRenderDebugWindow::GetNodeSize(const Ref<IRenderPass>& pass, CRenderResourceRegistry& registry) const
	{
		constexpr F32 iconSize = 24.0f;
		constexpr F32 iconNamePadding = 6.0f;
		constexpr F32 iconPadding = iconSize + iconNamePadding * 1.5f;

		const I64 maxHandleColumnLength = UMath::Max(pass->Inputs.size(), pass->Outputs.size());
		F32 inputMaxRequired = 0.0f;
		F32 outputMaxRequired = 0.0f;

		for (SRenderResourceHandle& handle : pass->Inputs)
		{
			const F32 nameWidth = GUI::CalculateTextSize(registry.GetResource(handle)->Description.Name.Data()).X;
			if (nameWidth > inputMaxRequired)
				inputMaxRequired = nameWidth + iconPadding;
		}
		for (SRenderResourceHandle& handle : pass->Outputs)
		{
			const F32 nameWidth = GUI::CalculateTextSize(registry.GetResource(handle)->Description.Name.Data()).X;
			if (nameWidth > outputMaxRequired)
				outputMaxRequired = nameWidth + iconPadding;
		}

		F32 requiredWidth = UMath::Max(GUI::CalculateTextSize(pass->GetName().c_str()).X, inputMaxRequired + outputMaxRequired);
		//if (pass->FlowType == EFlowType::Simple)
		//	requiredWidth += 50.0f;
		requiredWidth = UMath::Max(requiredWidth, 100.0f);
		return SVector2(requiredWidth, HeaderHeight + 1.5f * iconNamePadding + iconPadding * F32(maxHandleColumnLength));
	}

	SColor CRenderDebugWindow::GetResourceColor(const SRenderResource* renderResource) const
	{
		if (renderResource == nullptr)
			return SColor(255, 255, 255);

		switch (renderResource->Description.Type)
		{		
		case ERenderResourceType::RenderTargetView:     return SColor(220, 48, 48);
		case ERenderResourceType::DepthStencilView:     return SColor(124, 21, 153);
		case ERenderResourceType::ShaderResourceView:   return SColor(255, 206, 27);
		case ERenderResourceType::ConstantBufferView:   return SColor(51, 150, 215);
		case ERenderResourceType::VertexBufferView:    return SColor(147, 226, 74);
		case ERenderResourceType::IndexBufferView:   return SColor(68, 201, 156);

		default:
			return SColor(255, 255, 255);
		}
	}
}
