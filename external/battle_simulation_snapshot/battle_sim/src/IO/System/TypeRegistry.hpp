#pragma once

#include <memory>
#include <typeindex>
#include <unordered_map>
#include <utility>

namespace battle_sim
{
	/// @brief Type-indexed registry with shared ownership and interface registration
	///
	/// Utility class that may help solve certain tasks. Feel free to modify it to fit
	/// your needs or discard it entirely if you find no use for it.
	///
	/// @note One instance per type. Adding overwrites existing.
	/// @note Instances outlive registry if external references exist.
	class TypeRegistry
	{
	private:
		std::unordered_map<std::type_index, std::shared_ptr<void>> _instances;

	public:
		TypeRegistry() = default;

		TypeRegistry(const TypeRegistry&) = delete;
		TypeRegistry& operator=(const TypeRegistry&) = delete;

		TypeRegistry(TypeRegistry&&) = default;
		TypeRegistry& operator=(TypeRegistry&&) = default;

		/// @brief Constructs instance in-place, registers under T and optional base types
		/// @note Overwrites existing. Interfaces verified at compile-time.
		/// @code
		/// registry.emplace<Position>(1.0f, 2.0f);  // Without interfaces
		/// registry.emplace<Circle, IShape, IDrawable>(radius, color);  // With interfaces
		/// @endcode
		template <typename T, typename... Interfaces, typename... Args>
		std::shared_ptr<T> emplace(Args&&... args)
		{
			static_assert((std::is_base_of_v<Interfaces, T> && ...), "T must inherit from all specified interfaces");

			auto instance = std::make_shared<T>(std::forward<Args>(args)...);
			_instances[std::type_index(typeid(T))] = instance;
			((_instances[std::type_index(typeid(Interfaces))] = instance), ...);
			return instance;
		}

		/// @brief Adds existing instance, registers under T and optional base types
		/// @note Overwrites existing. Interfaces verified at compile-time.
		/// @code
		/// registry.add(velocityPtr);  // Without interfaces
		/// registry.add<Dog, IAnimal>(std::move(dogPtr));  // With interfaces
		/// @endcode
		template <typename T, typename... Interfaces>
		void add(std::shared_ptr<T> instance)
		{
			static_assert((std::is_base_of_v<Interfaces, T> && ...), "T must inherit from all specified interfaces");

			_instances[std::type_index(typeid(T))] = instance;
			((_instances[std::type_index(typeid(Interfaces))] = instance), ...);
		}

		/// @brief Retrieves instance by type (concrete or registered base)
		/// @return nullptr if not found. Always check before use.
		/// @code
		/// if (auto pos = registry.get<Position>()) { }  // by concrete type
		/// if (auto shape = registry.get<IShape>()) { }  // by interface
		/// @endcode
		template <typename T>
		std::shared_ptr<T> get() const
		{
			auto it = _instances.find(std::type_index(typeid(T)));
			if (it != _instances.end())
			{
				return std::static_pointer_cast<T>(it->second);
			}
			return nullptr;
		}
	};
}
