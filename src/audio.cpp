#include <cassert>
#include "audio.h"

ILLIXR_AUDIO::ABAudio::ABAudio(std::string outputFilePath, ProcessType procTypeIn)
    : processType{procTypeIn}
    , outputFile{
                 processType == ILLIXR_AUDIO::ABAudio::ProcessType::FULL
                 ? std::make_optional<std::ofstream>(outputFilePath, std::ios_base::out| std::ios_base::binary)
                 : std::nullopt
    }
{
    if (processType == ILLIXR_AUDIO::ABAudio::ProcessType::FULL) {
        generateWAVHeader();
    }

    // binauralizer as ambisonics decoder
	unsigned tailLength;
    assert(decoder.Configure(NORDER, true, SAMPLERATE, BLOCK_SIZE, tailLength));

    // Processor to rotate
    assert(rotator.Configure(NORDER, true, BLOCK_SIZE, 0));

    // Processor to zoom
    assert(zoomer.Configure(NORDER, true, 0));
}

void ILLIXR_AUDIO::ABAudio::loadSource(){
    // Add a bunch of sound sources
    PolarPoint position;

    if (processType == ILLIXR_AUDIO::ABAudio::ProcessType::FULL) {
        soundSrcs.emplace_back("samples/lectureSample.wav", NORDER, true);
        soundSrcs[soundSrcs.size() - 1].setSrcPos({
            .fAzimuth = -0.1f,
            .fElevation = 3.14f/2,
            .fDistance = 1,
        });

        soundSrcs.emplace_back("samples/radioMusicSample.wav", NORDER, true);
        position.fAzimuth = 1.0;
        position.fElevation = 0;
        position.fDistance = 5;
        soundSrcs[soundSrcs.size() - 1].setSrcPos({
            .fAzimuth = 1.0,
            .fElevation = 3.14f/2,
            .fDistance = 1,
        });
    } else {
        for (unsigned i = 0; i < NUM_SRCS; i++) {
            soundSrcs.emplace_back("samples/lectureSample.wav", NORDER, true);
            soundSrcs[soundSrcs.size() - 1].setSrcPos({
                .fAzimuth = -0.1f * i,
                .fElevation = 3.14f/2 * i,
                .fDistance = 1.0f * i,
            });
        }
    }
}

void ILLIXR_AUDIO::ABAudio::processBlock(){
    float** resultSample = new float*[2];
    resultSample[0] = new float[BLOCK_SIZE];
    resultSample[1] = new float[BLOCK_SIZE];

    // temporary BFormat file to sum up ambisonics
    CBFormat sumBF;
    sumBF.Configure(NORDER, true, BLOCK_SIZE);

    if (processType != ILLIXR_AUDIO::ABAudio::ProcessType::DECODE){
        readNEncode(sumBF);
    }
    if (processType != ILLIXR_AUDIO::ABAudio::ProcessType::ENCODE){
        // processing garbage data if just decoding
        rotateNZoom(sumBF);
        decoder.Process(&sumBF, resultSample);
    }

    if (processType == ILLIXR_AUDIO::ABAudio::ProcessType::FULL){
        writeFile(resultSample);
    }

    delete[] resultSample[0];
    delete[] resultSample[1];
    delete[] resultSample;
}

// Read from WAV files and encode into ambisonics format
void ILLIXR_AUDIO::ABAudio::readNEncode(CBFormat& sumBF){
    for (unsigned int soundIdx = 0; soundIdx < soundSrcs.size(); ++soundIdx) {
        std::unique_ptr<CBFormat>& tempBF = soundSrcs[soundIdx].readInBFormat();
        if (soundIdx == 0) {
            sumBF = *tempBF;
        } else {
            sumBF += *tempBF;
        }
    }
}

// Simple rotation
void ILLIXR_AUDIO::ABAudio::updateRotation(){
	frame++;
	Orientation head(0,0,1.0*frame/1500*3.14*2);
	rotator.SetOrientation(head);
	rotator.Refresh();
}
// Simple zoom
void ILLIXR_AUDIO::ABAudio::updateZoom(){
	frame++;
	zoomer.SetZoom(sinf(frame/100));
	zoomer.Refresh();
}
// Process some rotation and zoom effects
void ILLIXR_AUDIO::ABAudio::rotateNZoom(CBFormat& sumBF){
	updateRotation();
	rotator.Process(&sumBF, BLOCK_SIZE);
	updateZoom();
	zoomer.Process(&sumBF, BLOCK_SIZE);
}

void ILLIXR_AUDIO::ABAudio::writeFile(float** resultSample){
	// Normalize(Clipping), then write into file
	for(std::size_t sampleIdx = 0; sampleIdx < BLOCK_SIZE; ++sampleIdx){
		resultSample[0][sampleIdx] = std::max(std::min(resultSample[0][sampleIdx], +1.0f), -1.0f);
		resultSample[1][sampleIdx] = std::max(std::min(resultSample[1][sampleIdx], +1.0f), -1.0f);
		int16_t tempSample0 = (int16_t)(resultSample[0][sampleIdx]/1.0 * 32767);
		int16_t tempSample1 = (int16_t)(resultSample[1][sampleIdx]/1.0 * 32767);
		outputFile->write((char*)&tempSample0,sizeof(short));
		outputFile->write((char*)&tempSample1,sizeof(short));
	}
}

namespace ILLIXR_AUDIO{
    // NOTE: WAV FILE SIZE is not correct
    typedef struct __attribute__ ((packed)) WAVHeader_t
    {
    	unsigned int sGroupID = 0x46464952;
    	unsigned int dwFileLength = 48000000;		// A large enough random number
    	unsigned int sRiffType = 0x45564157;
    	unsigned int subchunkID = 0x20746d66;
    	unsigned int subchunksize = 16;
    	unsigned short audioFormat = 1;
    	unsigned short NumChannels = 2;
    	unsigned int SampleRate = 48000;
    	unsigned int byteRate = 48000*2*2;
    	unsigned short BlockAlign = 2*2;
    	unsigned short BitsPerSample = 16;
    	unsigned int dataChunkID = 0x61746164;	
    	unsigned int dataChunkSize = 48000000;		// A large enough random number
    } WAVHeader;
}
void ILLIXR_AUDIO::ABAudio::generateWAVHeader(){
	// brute force wav header
	WAVHeader wavh;
	outputFile->write((char*)&wavh, sizeof(WAVHeader));
}
