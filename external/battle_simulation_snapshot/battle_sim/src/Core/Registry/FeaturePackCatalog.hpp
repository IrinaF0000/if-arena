// Catalog of built-in feature-pack factories.

#pragma once

#include "Core/Registry/FeaturePack.hpp"
#include "Core/Registry/HandlerId.hpp"

#include <functional>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>

namespace battle_sim::core::registry
{
	class FeaturePackCatalog
	{
	public:
		using Factory = std::function<std::unique_ptr<IFeaturePack>()>;

		void add(HandlerId id, Factory factory)
		{
			if (!factory)
			{
				throw std::invalid_argument("Feature pack factory must not be empty: " + id.str());
			}

			auto [_, inserted] = _factories.emplace(std::move(id), std::move(factory));
			if (!inserted)
			{
				throw std::runtime_error("Feature pack already registered");
			}
		}

		void add(std::string id, Factory factory)
		{
			add(HandlerId::fromString(std::move(id)), std::move(factory));
		}

		bool contains(const HandlerId& id) const
		{
			return _factories.contains(id);
		}

		bool contains(std::string id) const
		{
			return contains(HandlerId::fromString(std::move(id)));
		}

		std::unique_ptr<IFeaturePack> create(const HandlerId& id) const
		{
			const auto it = _factories.find(id);
			if (it == _factories.end())
			{
				throw std::runtime_error("Feature pack not found: " + id.str());
			}

			auto pack = it->second();
			if (!pack)
			{
				throw std::runtime_error("Feature pack factory returned null: " + id.str());
			}
			return pack;
		}

		std::unique_ptr<IFeaturePack> create(std::string id) const
		{
			return create(HandlerId::fromString(std::move(id)));
		}

	private:
		std::unordered_map<HandlerId, Factory> _factories;
	};
}
