// Copyright 2025 Team Havtorn. All Rights Reserved.

#include "ECSNodes.h"

#include "Scene/Scene.h"
#include "ECS/ECSInclude.h"
#include "ECS/GUIDManager.h"

// TODO.NW: Route asset switching through AssetRegistry
#include "Graphics/RenderManager.h"

namespace Havtorn
{
	namespace HexRune
	{
		SEntityLoopNode::SEntityLoopNode(const U64 id, const U32 typeID, SScript* owningScript)
			: SNode::SNode(id, typeID, owningScript, ENodeType::Standard)
		{
			AddInput(UGUIDManager::Generate(), EPinType::Flow);
			AddInput(UGUIDManager::Generate(), EPinType::EntityList, "Array");

			AddOutput(UGUIDManager::Generate(), EPinType::Flow, "Loop Body");
			AddOutput(UGUIDManager::Generate(), EPinType::Entity, "Element");
			AddOutput(UGUIDManager::Generate(), EPinType::Int, "Index");
			AddOutput(UGUIDManager::Generate(), EPinType::Flow, "Completed");
		}

		I8 SEntityLoopNode::OnExecute()
		{
			std::vector<SEntity> entities;
			GetDataOnPin(EPinDirection::Input, 0, entities);

			for (I32 i = 0; i < STATIC_I32(entities.size()); i++)
			{
				SEntity& entity = entities[i];
				SetDataOnPin(EPinDirection::Output, 1, entity);
				SetDataOnPin(EPinDirection::Output, 2, i);

				if (Outputs[0].LinkedPin != nullptr)
					Outputs[0].LinkedPin->OwningNode->Execute();
			}

			return 3;
		}

		SComponentLoopNode::SComponentLoopNode(const U64 id, const U32 typeID, SScript* owningScript)
			: SNode::SNode(id, typeID, owningScript, ENodeType::Standard)
		{
			AddInput(UGUIDManager::Generate(), EPinType::Flow);
			AddInput(UGUIDManager::Generate(), EPinType::ComponentPtrList, "Array");

			AddOutput(UGUIDManager::Generate(), EPinType::Flow, "Loop Body");
			AddOutput(UGUIDManager::Generate(), EPinType::ComponentPtr, "Element");
			AddOutput(UGUIDManager::Generate(), EPinType::Int, "Index");
			AddOutput(UGUIDManager::Generate(), EPinType::Flow, "Completed");
		}

		I8 SComponentLoopNode::OnExecute()
		{
			std::vector<SComponent*> components;
			GetDataOnPin(EPinDirection::Input, 0, components);

			for (I32 i = 0; i < STATIC_I32(components.size()); i++)
			{
				SComponent* component = components[i];
				SetDataOnPin(EPinDirection::Output, 1, component);
				SetDataOnPin(EPinDirection::Output, 2, i);

				if (Outputs[0].LinkedPin != nullptr)
					Outputs[0].LinkedPin->OwningNode->Execute();
			}

			return 3;
		}

		SPrintEntityNameNode::SPrintEntityNameNode(const U64 id, const U32 typeID, SScript* owningScript)
			: SNode::SNode(id, typeID, owningScript, ENodeType::Standard)
		{
			AddInput(UGUIDManager::Generate(), EPinType::Flow);
			AddInput(UGUIDManager::Generate(), EPinType::Entity);
		}

		I8 SPrintEntityNameNode::OnExecute()
		{
			SEntity entity;
			GetDataOnPin(EPinDirection::Input, 1, entity);

			if (OwningScript == nullptr || OwningScript->Scene == nullptr)
				return 0;

			if (entity == SEntity::Null)
			{
				HV_LOG_INFO("Entity is Null");
				return 0;
			}

			SMetaDataComponent* metaData = OwningScript->Scene->GetComponent<SMetaDataComponent>(entity);

			HV_LOG_INFO(metaData->Name.Data());
			return 0;
		}

		SSetStaticMeshNode::SSetStaticMeshNode(const U64 id, const U32 typeID, SScript* owningScript)
			: SNode::SNode(id, typeID, owningScript, ENodeType::Standard)
		{
			AddInput(UGUIDManager::Generate(), EPinType::Flow);
			AddInput(UGUIDManager::Generate(), EPinType::Entity, "Entity");
			AddInput(UGUIDManager::Generate(), EPinType::Asset, "Mesh");

			AddOutput(UGUIDManager::Generate(), EPinType::Flow, "Success");
			AddOutput(UGUIDManager::Generate(), EPinType::Flow, "Failure");
		}

		I8 SSetStaticMeshNode::OnExecute()
		{
			if (OwningScript == nullptr || OwningScript->Scene == nullptr)
				return 1;

			SEntity entity;
			GetDataOnPin(EPinDirection::Input, 1, entity);
			if (!entity.IsValid())
				return 1;
			
			std::string assetPath;
			GetDataOnPin(EPinDirection::Input, 2, assetPath);
			if (assetPath == "")
				return 1;

			SStaticMeshComponent* meshComponent = OwningScript->Scene->GetComponent<SStaticMeshComponent>(entity);
			if (!meshComponent)
				return 1;

			meshComponent->AssetReference = SAssetReference(assetPath);
			return 0;
		}

		STogglePointLightNode::STogglePointLightNode(const U64 id, const U32 typeID, SScript* owningScript)
			: SNode::SNode(id, typeID, owningScript, ENodeType::Standard)
		{
			AddInput(UGUIDManager::Generate(), EPinType::Flow);
			AddInput(UGUIDManager::Generate(), EPinType::Entity, "Entity");
			AddInput(UGUIDManager::Generate(), EPinType::Bool, "Enable");

			AddOutput(UGUIDManager::Generate(), EPinType::Flow);
		}

		I8 STogglePointLightNode::OnExecute()
		{
			if (OwningScript == nullptr || OwningScript->Scene == nullptr)
				return 0;

			SEntity entity;
			GetDataOnPin(EPinDirection::Input, 1, entity);
			if (!entity.IsValid())
				return 0;

			bool enable = false;
			GetDataOnPin(EPinDirection::Input, 2, enable);

			SPointLightComponent* lightComponent = OwningScript->Scene->GetComponent<SPointLightComponent>(entity);
			if (!lightComponent)
				return 0;

			lightComponent->IsActive = enable;
			return 0;
		}

		SOnBeginOverlapNode::SOnBeginOverlapNode(const U64 id, const U32 typeID, SScript* owningScript)
			: SNode::SNode(id, typeID, owningScript, ENodeType::Standard)
		{
			AddOutput(UGUIDManager::Generate(), EPinType::Flow);
			AddOutput(UGUIDManager::Generate(), EPinType::Entity, "Triggering Entity");
			AddOutput(UGUIDManager::Generate(), EPinType::Entity, "Other Entity");
		}

		I8 SOnBeginOverlapNode::OnExecute()
		{
			return 0;
		}

		SOnEndOverlapNode::SOnEndOverlapNode(const U64 id, const U32 typeID, SScript* owningScript)
			: SNode::SNode(id, typeID, owningScript, ENodeType::Standard)
		{
			AddOutput(UGUIDManager::Generate(), EPinType::Flow);
			AddOutput(UGUIDManager::Generate(), EPinType::Entity, "Triggering Entity");
			AddOutput(UGUIDManager::Generate(), EPinType::Entity, "Other Entity");
		}

		I8 SOnEndOverlapNode::OnExecute()
		{
			return 0;
		}
}
}
