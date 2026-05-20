#include "TcpFrameCodec.hpp"

#include <cstdint>
#include <exception>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace
{
	using namespace if_arena::battle_transport_tcp;

	void require(bool condition, const std::string& message)
	{
		if (!condition)
		{
			throw std::runtime_error(message);
		}
	}

	std::vector<std::uint8_t> prefix(std::uint32_t value)
	{
		return {
			static_cast<std::uint8_t>((value >> 24u) & 0xffu),
			static_cast<std::uint8_t>((value >> 16u) & 0xffu),
			static_cast<std::uint8_t>((value >> 8u) & 0xffu),
			static_cast<std::uint8_t>(value & 0xffu),
		};
	}

	void partialFrameWaitsForPayload()
	{
		const auto encoded = encodeFrame("{\"version\":1}");
		require(encoded.ok(), "frame encodes");

		TcpFrameDecoder decoder;
		const auto first = decoder.feed(std::span<const std::uint8_t>{encoded.bytes.data(), 2u});
		require(first.ok(), "partial prefix is accepted");
		require(first.frames.empty(), "partial prefix emits no frames");

		const auto second = decoder.feed(std::span<const std::uint8_t>{encoded.bytes.data() + 2u, encoded.bytes.size() - 2u});
		require(second.ok(), "remaining bytes accepted");
		require(second.frames.size() == 1, "completed frame emitted");
		require(second.frames.front() == "{\"version\":1}", "payload preserved");
	}

	void combinedFramesEmitInOrder()
	{
		const auto firstFrame = encodeFrame("one");
		const auto secondFrame = encodeFrame("two");
		require(firstFrame.ok() && secondFrame.ok(), "frames encode");

		std::vector<std::uint8_t> combined = firstFrame.bytes;
		combined.insert(combined.end(), secondFrame.bytes.begin(), secondFrame.bytes.end());

		TcpFrameDecoder decoder;
		const auto result = decoder.feed(combined);
		require(result.ok(), "combined frames accepted");
		require(result.frames.size() == 2, "two frames emitted");
		require(result.frames[0] == "one", "first payload preserved");
		require(result.frames[1] == "two", "second payload preserved");
	}

	void oversizedLengthRejectedBeforePayloadAllocation()
	{
		TcpFrameLimits limits;
		limits.maxFrameBytes = 8;
		TcpFrameDecoder decoder(limits);

		const auto result = decoder.feed(prefix(9));
		require(!result.ok(), "oversized frame length rejected");
		require(result.error->code == TcpFrameErrorCode::FrameTooLarge, "frame too large error");
		require(decoder.closed(), "decoder enters closed state");
		require(decoder.bufferedBytes() == 0, "decoder buffer cleared");
	}

	void zeroLengthRejected()
	{
		TcpFrameDecoder decoder;

		const auto result = decoder.feed(prefix(0));
		require(!result.ok(), "zero length rejected");
		require(result.error->code == TcpFrameErrorCode::InvalidLength, "invalid length error");
		require(decoder.closed(), "decoder enters closed state");
	}

	void encoderRejectsOversizedPayload()
	{
		TcpFrameLimits limits;
		limits.maxFrameBytes = 2;

		const auto result = encodeFrame("abc", limits);
		require(!result.ok(), "oversized payload rejected");
		require(result.error->code == TcpFrameErrorCode::FrameTooLarge, "frame too large encode error");
	}
}

int main()
{
	const std::vector<std::pair<const char*, void (*)()>> tests{
		{"partialFrameWaitsForPayload", partialFrameWaitsForPayload},
		{"combinedFramesEmitInOrder", combinedFramesEmitInOrder},
		{"oversizedLengthRejectedBeforePayloadAllocation", oversizedLengthRejectedBeforePayloadAllocation},
		{"zeroLengthRejected", zeroLengthRejected},
		{"encoderRejectsOversizedPayload", encoderRejectsOversizedPayload},
	};

	int failed = 0;
	for (const auto& [name, test] : tests)
	{
		try
		{
			test();
			std::cout << "[PASS] " << name << '\n';
		}
		catch (const std::exception& ex)
		{
			++failed;
			std::cerr << "[FAIL] " << name << ": " << ex.what() << '\n';
		}
	}

	if (failed != 0)
	{
		std::cerr << failed << " test(s) failed\n";
		return 1;
	}

	std::cout << tests.size() << " test(s) passed\n";
	return 0;
}
