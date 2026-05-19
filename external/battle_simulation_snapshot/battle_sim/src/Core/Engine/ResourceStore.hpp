// Generic type-indexed singleton resource storage for engine-level state.

#pragma once

#include <memory>
#include <stdexcept>
#include <typeindex>
#include <typeinfo>
#include <unordered_map>
#include <utility>

namespace battle_sim::core::engine
{
	class ResourceStore
	{
	public:
		template <class T, class... Args>
		T& emplace(Args&&... args)
		{
			auto resource = std::make_unique<Resource<T>>(std::forward<Args>(args)...);
			auto& value = resource->value;
			_resources.insert_or_assign(std::type_index(typeid(T)), std::move(resource));
			return value;
		}

		template <class T>
		bool contains() const
		{
			return _resources.contains(std::type_index(typeid(T)));
		}

		template <class T>
		T& get()
		{
			return resource<T>().value;
		}

		template <class T>
		const T& get() const
		{
			return resource<T>().value;
		}

	private:
		struct IResource
		{
			virtual ~IResource() = default;
		};

		template <class T>
		struct Resource final : IResource
		{
			template <class... Args>
			explicit Resource(Args&&... args) : value(std::forward<Args>(args)...)
			{
			}

			T value;
		};

		template <class T>
		Resource<T>& resource()
		{
			const auto it = _resources.find(std::type_index(typeid(T)));
			if (it == _resources.end())
			{
				throw std::runtime_error("Resource not found");
			}
			return static_cast<Resource<T>&>(*it->second);
		}

		template <class T>
		const Resource<T>& resource() const
		{
			const auto it = _resources.find(std::type_index(typeid(T)));
			if (it == _resources.end())
			{
				throw std::runtime_error("Resource not found");
			}
			return static_cast<const Resource<T>&>(*it->second);
		}

		std::unordered_map<std::type_index, std::unique_ptr<IResource>> _resources;
	};
}
