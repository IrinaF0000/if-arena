// Registers statically linked feature packs.

#pragma once

#include "Core/Registry/FeaturePackCatalog.hpp"

namespace battle_sim::app
{
	void registerBuiltInFeaturePacks(core::registry::FeaturePackCatalog& catalog);
}
