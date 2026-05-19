#pragma once

#include <iostream>

namespace battle_sim
{
	class CommandParserVisitor
	{
	private:
		std::istream& _stream;

	public:
		CommandParserVisitor(std::istream& stream) :
				_stream(stream)
		{}

		template <class TField>
		void visit(const char*, TField& field)
		{
			_stream >> field;
		}

		std::istream& stream()
		{
			return _stream;
		}
	};
}
