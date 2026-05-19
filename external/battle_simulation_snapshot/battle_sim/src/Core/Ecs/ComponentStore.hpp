// Sparse component storage keyed by EntityId.

#pragma once

#include "Core/Ecs/Entity.hpp"

#include <cstddef>
#include <memory>
#include <stdexcept>
#include <typeindex>
#include <typeinfo>
#include <unordered_map>
#include <utility>

namespace battle_sim::core::ecs
{
	class IComponentStore
	{
	public:
		virtual ~IComponentStore() = default;

		virtual void removeEntity(EntityId id) = 0;
	};

	template <class T>
	class ComponentStore final : public IComponentStore
	{
	public:
		bool has(EntityId id) const
		{
			return _components.contains(id);
		}

		T* get(EntityId id)
		{
			auto it = _components.find(id);
			if (it == _components.end())
			{
				return nullptr;
			}
			return &it->second;
		}

		const T* get(EntityId id) const
		{
			auto it = _components.find(id);
			if (it == _components.end())
			{
				return nullptr;
			}
			return &it->second;
		}

		void emplace(EntityId id, T value)
		{
			_components.insert_or_assign(id, std::move(value));
		}

		void remove(EntityId id)
		{
			_components.erase(id);
		}

		void removeEntity(EntityId id) override
		{
			remove(id);
		}

		std::size_t size() const
		{
			return _components.size();
		}

	private:
		std::unordered_map<EntityId, T> _components;
	};

	class ComponentStorage
	{
	public:
		template <class T>
		ComponentStore<T>& store()
		{
			const auto key = std::type_index(typeid(T));
			auto it = _stores.find(key);
			if (it == _stores.end())
			{
				auto store = std::make_unique<ComponentStore<T>>();
				auto* raw = store.get();
				_stores.emplace(key, std::move(store));
				return *raw;
			}
			return static_cast<ComponentStore<T>&>(*it->second);
		}

		template <class T>
		const ComponentStore<T>& store() const
		{
			const auto it = _stores.find(std::type_index(typeid(T)));
			if (it == _stores.end())
			{
				throw std::runtime_error("Component store not found");
			}
			return static_cast<const ComponentStore<T>&>(*it->second);
		}

		void removeEntity(EntityId id)
		{
			for (auto& [_, store] : _stores)
			{
				store->removeEntity(id);
			}
		}

	private:
		std::unordered_map<std::type_index, std::unique_ptr<IComponentStore>> _stores;
	};
}
