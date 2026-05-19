// Deterministic JSON trace writer for world events.

#pragma once

#include "Core/IWorldEventSink.hpp"

#include <iosfwd>

namespace battle_sim::io::trace
{
	class JsonTraceWriter final : public core::IWorldEventSink
	{
	public:
		explicit JsonTraceWriter(std::ostream& output);
		~JsonTraceWriter() override;

		void onEvent(const std::any& event) override;
		void finish();

	private:
		std::ostream& _output;
		bool _hasRecords{false};
		bool _finished{false};

		void beginRecord();
		void endRecord();
	};
}
