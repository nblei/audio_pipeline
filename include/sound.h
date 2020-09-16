#pragma once

#include <string>
#include <fstream>
#include <memory>
#include <spatialaudio/Ambisonics.h>

#define NUM_CHANNELS (OrderToComponents(NORDER, true))
constexpr size_t SAMPLERATE = 48000;
constexpr size_t BLOCK_SIZE = 1024;
constexpr size_t NORDER = 3;
constexpr size_t NUM_SRCS = 4;

namespace ILLIXR_AUDIO{
	class Sound{
	public:
		Sound(std::string srcFile, unsigned nOrder, bool b3D);

		// set sound src position
		void setSrcPos(PolarPoint pos);
		// set sound amplitude scale
		void setSrcAmp(float ampScale);
		// read sound samples from mono 16bit WAV file and encode into ambisonics format
		std::unique_ptr<CBFormat>& readInBFormat();
	private:
		// corresponding sound src file
		std::fstream srcFile;
		// sample buffer HARDCODE
		float sample[BLOCK_SIZE];
		// ambisonics format sound buffer
		std::unique_ptr<CBFormat> BFormat;
		// ambisonics encoder, containing format info, position info, etc.
		CAmbisonicEncoderDist BEncoder;
		// ambisonics position
		PolarPoint srcPos;
		// amplitude scale to avoid clipping
		float amp;
	};
}
