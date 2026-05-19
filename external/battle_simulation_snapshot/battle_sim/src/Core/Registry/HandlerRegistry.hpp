// Generic handler registry keyed by HandlerId.

#pragma once

#include "Core/Registry/HandlerId.hpp"

#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>

namespace battle_sim::core::registry
{
	template <class T>
	class HandlerRegistry
	{
	public:
		void add(HandlerId id, std::unique_ptr<T> handler)
		{
			if (!handler)
			{
				throw std::invalid_argument("Handler must not be null: " + id.str());
			}

			auto [_, inserted] = _handlers.emplace(std::move(id), std::move(handler));
			if (!inserted)
			{
				throw std::runtime_error("Handler already registered");
			}
		}

		void add(std::string id, std::unique_ptr<T> handler)
		{
			add(HandlerId::fromString(std::move(id)), std::move(handler));
		}

		bool contains(const HandlerId& id) const
		{
			return _handlers.contains(id);
		}

		bool contains(std::string id) const
		{
			return contains(HandlerId::fromString(std::move(id)));
		}

		T& get(const HandlerId& id)
		{
			return *find(id);
		}

		const T& get(const HandlerId& id) const
		{
			return *find(id);
		}

		T& get(std::string id)
		{
			return get(HandlerId::fromString(std::move(id)));
		}

		const T& get(std::string id) const
		{
			return get(HandlerId::fromString(std::move(id)));
		}

	private:
		T* find(const HandlerId& id)
		{
			const auto it = _handlers.find(id);
			if (it == _handlers.end())
			{
				throw std::runtime_error("Handler not found: " + id.str());
			}
			return it->second.get();
		}

		const T* find(const HandlerId& id) const
		{
			const auto it = _handlers.find(id);
			if (it == _handlers.end())
			{
				throw std::runtime_error("Handler not found: " + id.str());
			}
			return it->second.get();
		}

		std::unordered_map<HandlerId, std::unique_ptr<T>> _handlers;
	};
}
