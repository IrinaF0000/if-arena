#include "ui/ArenaView.hpp"

#include <QApplication>

#include <stdexcept>
#include <string>
#include <utility>

namespace
{
	using if_arena::battle_qt_client::game::ArenaSnapshot;
	using if_arena::battle_qt_client::game::HazardSnapshot;
	using if_arena::battle_qt_client::game::ScenarioMetadata;
	using if_arena::battle_qt_client::ui::ArenaView;

	void require(bool condition, const std::string& message)
	{
		if (!condition)
		{
			throw std::runtime_error(message);
		}
	}

	HazardSnapshot hazard(QString visualId, int damage, bool causesDrop, double rangeRadius)
	{
		HazardSnapshot result;
		result.id = visualId + "_id";
		result.kind = visualId.contains("crow") ? "crow" : visualId.contains("tower") ? "tower" : "mine";
		result.visualId = std::move(visualId);
		result.damage = damage;
		result.causesDrop = causesDrop;
		result.rangeRadius = rangeRadius;
		result.icon = result.visualId;
		result.team = "neutral";
		return result;
	}

	void hazardLegendTextLivesOutsidePaintOverlay()
	{
		ArenaView view;
		require(view.hazardLegendText() == "Hazards: waiting for snapshot", "empty view reports waiting hazard legend");

		ArenaSnapshot snapshot;
		snapshot.scenario = ScenarioMetadata{"arena_small_objective_run", "objective_run", 1, "server_config"};
		snapshot.hazards.push_back(hazard("hazard_crow", 3, false, 0.5));
		snapshot.hazards.push_back(hazard("hazard_crow", 4, true, 1.0));
		snapshot.hazards.push_back(hazard("hazard_tower", 6, true, 2.2));
		view.setSnapshot(snapshot, "local");

		const auto text = view.hazardLegendText();
		require(text.contains("Hazards:"), "hazard legend is side-panel text");
		require(text.contains("crow: -3, r0.5"), "crow effect text is derived from snapshot metadata");
		require(text.contains("tower: -6 + drop, r2.2"), "tower drop effect text is derived from snapshot metadata");
		require(text.count("crow:") == 1, "duplicate visual hazard entries are collapsed");
	}
}

int main(int argc, char** argv)
{
	QApplication app(argc, argv);
	try
	{
		hazardLegendTextLivesOutsidePaintOverlay();
	}
	catch (const std::exception& ex)
	{
		qCritical("ArenaView overlay test failed: %s", ex.what());
		return 1;
	}
	return 0;
}
