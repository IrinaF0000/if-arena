#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace if_arena::battle_transport_tcp
{
	struct TcpFrameLimits
	{
		std::uint32_t maxFrameBytes{64u * 1024u};
		std::size_t maxBufferedBytes{128u * 1024u};
	};

	enum class TcpFrameErrorCode
	{
		None,
		InvalidLength,
		FrameTooLarge,
		BufferLimitExceeded,
		DecoderClosed
	};

	struct TcpFrameError
	{
		TcpFrameErrorCode code{TcpFrameErrorCode::None};
		std::string message;
	};

	struct TcpFrameDecodeResult
	{
		std::vector<std::string> frames;
		std::optional<TcpFrameError> error;

		[[nodiscard]] bool ok() const;
	};

	struct TcpFrameEncodeResult
	{
		std::vector<std::uint8_t> bytes;
		std::optional<TcpFrameError> error;

		[[nodiscard]] bool ok() const;
	};

	class TcpFrameDecoder
	{
	public:
		explicit TcpFrameDecoder(TcpFrameLimits limits = {});

		TcpFrameDecodeResult feed(std::span<const std::uint8_t> bytes);
		void reset();

		[[nodiscard]] bool closed() const;
		[[nodiscard]] std::size_t bufferedBytes() const;

	private:
		TcpFrameLimits _limits;
		std::vector<std::uint8_t> _buffer;
		std::optional<TcpFrameError> _error;

		[[nodiscard]] std::uint32_t peekLength() const;
		TcpFrameError fail(TcpFrameErrorCode code, std::string message);
	};

	[[nodiscard]] TcpFrameEncodeResult encodeFrame(std::string_view payload, TcpFrameLimits limits = {});
}
