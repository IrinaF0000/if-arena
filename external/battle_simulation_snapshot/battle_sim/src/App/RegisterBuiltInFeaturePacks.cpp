// Registers statically linked feature packs.

#include "App/RegisterBuiltInFeaturePacks.hpp"

#include "Features/Battle/BattleFeaturePack.hpp"
#include "Features/UnitsClassic/UnitsClassicFeaturePack.hpp"
#include "IO/LegacyCommands/LegacyIoFeaturePack.hpp"

#include <memory>

namespace battle_sim::app
{
	void registerBuiltInFeaturePacks(core::registry::FeaturePackCatalog& catalog)
	{
		catalog.add("battle.basic", [] { return std::make_unique<features::battle::BattleFeaturePack>(); });
		catalog.add("units.classic", [] { return std::make_unique<features::units_classic::UnitsClassicFeaturePack>(); });
		catalog.add("io.legacy", [] { return std::make_unique<io::legacy::LegacyIoFeaturePack>(); });
	}
}
