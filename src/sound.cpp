#include <cassert>
#include <algorithm>
#include "sound.h"

ILLIXR_AUDIO::Sound::Sound(std::string srcFilename, [[maybe_unused]] unsigned nOrder, [[maybe_unused]] bool b3D)
	: srcFile{srcFilename, std::fstream::in}
	, BFormat{new CBFormat}
	, amp{1.0}
{
    // NOTE: This is currently only accepts mono channel 16-bit depth WAV file
    // TODO: Change brutal read from wav file
	std::byte temp[44];
    srcFile.read(reinterpret_cast<char*>(temp), sizeof(temp));

    // BFormat file initialization
    assert(BFormat->Configure(nOrder, true, BLOCK_SIZE));
    BFormat->Refresh();

    // Encoder initialization
    assert(BEncoder.Configure(nOrder, true, SAMPLERATE));
    BEncoder.Refresh();
    srcPos.fAzimuth = 0;
    srcPos.fElevation = 0;
    srcPos.fDistance = 0;
    BEncoder.SetPosition(srcPos);
    BEncoder.Refresh();
}

void ILLIXR_AUDIO::Sound::setSrcPos(PolarPoint pos){
    srcPos.fAzimuth = pos.fAzimuth;
    srcPos.fElevation = pos.fElevation;
    srcPos.fDistance = pos.fDistance;
    BEncoder.SetPosition(srcPos);
    BEncoder.Refresh();
}

void ILLIXR_AUDIO::Sound::setSrcAmp(float ampScale){
    amp = ampScale;
}

//TODO: Change brutal read from wav file
std::unique_ptr<CBFormat>& ILLIXR_AUDIO::Sound::readInBFormat(){
    float sampleTemp[BLOCK_SIZE];
    srcFile.read(reinterpret_cast<char*>(sampleTemp), sizeof(sampleTemp));
    // normalize samples to -1 to 1 float, with amplitude scale
    for (std::size_t i = 0; i < BLOCK_SIZE; ++i){
        sample[i] = amp * (sampleTemp[i] / 32767.0);
    }
    BEncoder.Process(sample, BLOCK_SIZE, BFormat.get());
    return BFormat;
}
