#include "Engine/World.h"
#include "Engine/GameObject.h"
#include "Components/Component.h"
#include "ECS/TransformECS.h"

using namespace Sailor;
using namespace Sailor::Tasks;

GameObject::GameObject(WorldPtr world, const std::string& name) : m_name(name), m_pWorld(world)
{
	m_transformHandle = m_pWorld->GetECS<TransformECS>()->RegisterComponent();
}

void GameObject::BeginPlay()
{
	GetTransformComponent().SetOwner(m_self);
}

TransformComponent& GameObject::GetTransformComponent()
{
	return m_pWorld->GetECS<TransformECS>()->GetComponentData(m_transformHandle);
}

bool GameObject::RemoveComponent(ComponentPtr component)
{
	check(component);

	if (m_components.RemoveFirst(component) || m_componentsToAdd.RemoveFirst(component))
	{
		component->EndPlay();
		component.DestroyObject(m_pWorld->GetAllocator());
		return true;
	}

	return false;
}

void GameObject::RemoveAllComponents()
{
	for (auto& el : m_components)
	{
		el->EndPlay();
		el.DestroyObject(m_pWorld->GetAllocator());
	}

	for (auto& el : m_componentsToAdd)
	{
		el->EndPlay();
		el.DestroyObject(m_pWorld->GetAllocator());
	}
	
	m_components.Clear(true);
}

void GameObject::EndPlay()
{
	m_pWorld->GetECS<TransformECS>()->UnregisterComponent(m_transformHandle);
}

void GameObject::Tick(float deltaTime)
{
	for (auto& el : m_components)
	{
		if (el->m_bBeginPlayCalled)
		{
			el->Tick(deltaTime);
		}
		else
		{
			el->BeginPlay();
			el->m_bBeginPlayCalled = true;
		}

#ifdef SAILOR_EDITOR
		el->OnGizmo();
#endif
	}

	m_components.AddRange(m_componentsToAdd);
	m_componentsToAdd.Clear();
}