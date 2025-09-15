#pragma once
#include "miniaudio.h"
#include "Result.hpp"
#include "Error.hpp"

struct AudioDevices {
	ma_device_info *playbackDeviceInfos;
	ma_uint32 playbackDeviceCount;

	ma_device_info *captureDeviceInfos;
	ma_uint32 captureDeviceCount;
};

using ResultVoid = Result<std::monostate, Error>;

namespace AudioRedirector {
	ResultVoid Initialize();
	ResultVoid Uninitialize();

	ResultVoid StartLoopbackRedirect(const ma_device_id *loopbackId, const ma_device_id *playbackId);
	ResultVoid StartDuplexRedirect(const ma_device_id *captureId, const ma_device_id *playbackId);

	ResultVoid StopLoopbackRedirect(); // Stop and uninitialize loopback and playback devices.
	ResultVoid StopDuplexRedirect();   // Stop and uninitialize duplex device.

	Result<AudioDevices, Error> GetAudioDevices();

	Result<float, Error> GetPlaybackVolume();
	ma_result SetPlaybackVolume(float volume);

	Result<float, Error> GetDuplexVolume();
	ma_result SetDuplexVolume(float volume);

	constexpr ma_format Formats[] = {
		ma_format_f32,
		ma_format_s32,
		ma_format_s24,
		ma_format_s16,
		ma_format_u8,
	};

	constexpr ma_uint32 SampleRates[] = {
		8000,
		11025,
		16000,
		22050,
		32000,
		44100,
		48000,
		96000,
		192000,
	};

	ma_format GetLoopbackFormat();
	ma_uint32 GetLoopbackSampleRate();

	void SetLoopbackFormat(ma_format format);
	void SetLoopbackSampleRate(ma_uint32 sampleRate);

	ma_format GetDuplexFormat();
	ma_uint32 GetDuplexSampleRate();

	void SetDuplexFormat(ma_format format);
	void SetDuplexSampleRate(ma_uint32 sampleRate);
}; // namespace AudioRedirector
