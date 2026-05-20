#include "TcpFrameCodec.hpp"

#include <algorithm>
#include <utility>

namespace if_arena::battle_transport_tcp
{
	namespace
	{
		TcpFrameError makeError(TcpFrameErrorCode code, std::string message)
		{
			return TcpFrameError{code, std::move(message)};
		}

		void appendUint32Be(std::vector<std::uint8_t>& output, std::uint32_t value)
		{
			output.push_back(static_cast<std::uint8_t>((value >> 24u) & 0xffu));
			output.push_back(static_cast<std::uint8_t>((value >> 16u) & 0xffu));
			output.push_back(static_cast<std::uint8_t>((value >> 8u) & 0xffu));
			output.push_back(static_cast<std::uint8_t>(value & 0xffu));
		}
	}

	bool TcpFrameDecodeResult::ok() const
	{
		return !error.has_value();
	}

	bool TcpFrameEncodeResult::ok() const
	{
		return !error.has_value();
	}

	TcpFrameDecoder::TcpFrameDecoder(TcpFrameLimits limits)
		: _limits(limits)
	{
	}

	TcpFrameDecodeResult TcpFrameDecoder::feed(std::span<const std::uint8_t> bytes)
	{
		if (_error.has_value())
		{
			return TcpFrameDecodeResult{{}, makeError(TcpFrameErrorCode::DecoderClosed, "decoder is closed")};
		}

		if (_buffer.size() + bytes.size() > _limits.maxBufferedBytes)
		{
			return TcpFrameDecodeResult{{}, fail(TcpFrameErrorCode::BufferLimitExceeded, "buffer limit exceeded")};
		}

		_buffer.insert(_buffer.end(), bytes.begin(), bytes.end());

		TcpFrameDecodeResult result;
		while (_buffer.size() >= 4u)
		{
			const std::uint32_t length = peekLength();
			if (length == 0u)
			{
				result.error = fail(TcpFrameErrorCode::InvalidLength, "frame length must be positive");
				return result;
			}
			if (length > _limits.maxFrameBytes)
			{
				result.error = fail(TcpFrameErrorCode::FrameTooLarge, "frame length exceeds limit");
				return result;
			}

			const auto frameBytes = static_cast<std::size_t>(length);
			const auto totalBytes = 4u + frameBytes;
			if (_buffer.size() < totalBytes)
			{
				break;
			}

			const auto payloadBegin = _buffer.begin() + 4;
			const auto payloadEnd = payloadBegin + static_cast<std::ptrdiff_t>(frameBytes);
			result.frames.emplace_back(payloadBegin, payloadEnd);
			_buffer.erase(_buffer.begin(), payloadEnd);
		}

		return result;
	}

	void TcpFrameDecoder::reset()
	{
		_buffer.clear();
		_error.reset();
	}

	bool TcpFrameDecoder::closed() const
	{
		return _error.has_value();
	}

	std::size_t TcpFrameDecoder::bufferedBytes() const
	{
		return _buffer.size();
	}

	std::uint32_t TcpFrameDecoder::peekLength() const
	{
		return (static_cast<std::uint32_t>(_buffer[0]) << 24u) |
			   (static_cast<std::uint32_t>(_buffer[1]) << 16u) |
			   (static_cast<std::uint32_t>(_buffer[2]) << 8u) |
			   static_cast<std::uint32_t>(_buffer[3]);
	}

	TcpFrameError TcpFrameDecoder::fail(TcpFrameErrorCode code, std::string message)
	{
		_error = makeError(code, std::move(message));
		_buffer.clear();
		return *_error;
	}

	TcpFrameEncodeResult encodeFrame(std::string_view payload, TcpFrameLimits limits)
	{
		if (payload.empty())
		{
			return TcpFrameEncodeResult{{}, makeError(TcpFrameErrorCode::InvalidLength, "payload must not be empty")};
		}
		if (payload.size() > limits.maxFrameBytes)
		{
			return TcpFrameEncodeResult{{}, makeError(TcpFrameErrorCode::FrameTooLarge, "payload exceeds frame limit")};
		}

		std::vector<std::uint8_t> bytes;
		bytes.reserve(payload.size() + 4u);
		appendUint32Be(bytes, static_cast<std::uint32_t>(payload.size()));
		std::transform(payload.begin(), payload.end(), std::back_inserter(bytes), [](char value) {
			return static_cast<std::uint8_t>(value);
		});
		return TcpFrameEncodeResult{std::move(bytes), std::nullopt};
	}
}
