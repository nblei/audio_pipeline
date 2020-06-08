#include <chrono>
#include <future>
#include <fstream>
#include <thread>

#include "common/threadloop.hpp"
#include "common/switchboard.hpp"
#include "common/data_format.hpp"
#include "common/phonebook.hpp"
#include "common/logger.hpp"
#include "common/pose_prediction.hpp"

#include <audio.h>

using namespace ILLIXR;

#define AUDIO_EPOCH (1024.0/48000.0)

class audio_component : public threadloop
{
public:
	audio_component(phonebook *pb)
		: sb{pb->lookup_impl<switchboard>()}
		, pp{pb->lookup_impl<pose_prediction>()}
	{
		ILLIXR_AUDIO::ABAudio::ProcessType processDecode(ILLIXR_AUDIO::ABAudio::ProcessType::DECODE);	
		decoder = new ILLIXR_AUDIO::ABAudio("", processDecode);
		ILLIXR_AUDIO::ABAudio::ProcessType processEncode(ILLIXR_AUDIO::ABAudio::ProcessType::ENCODE);	
		encoder = new ILLIXR_AUDIO::ABAudio("", processEncode);

		decoder->loadSource();
		encoder->loadSource();

		logger = new start_end_logger("audio");
	}
	~audio_component(){
		delete encoder;
		delete decoder;
	}

	virtual void _p_one_iteration(){
		std::chrono::time_point<std::chrono::system_clock> blockStart = std::chrono::high_resolution_clock::now();
		if (blockStart < sync) {
			std::this_thread::yield(); // ←_←
			// continue;
			return;
		}
		// seconds in double
		double timespent = std::chrono::duration<double>(blockStart-sync).count();
		int num_epoch = ceil(timespent/AUDIO_EPOCH);
		sync += std::chrono::microseconds(num_epoch*((int)(AUDIO_EPOCH*1000000))); 

		logger->log_start(std::chrono::high_resolution_clock::now());
		auto most_recent_pose = pp->get_fast_pose();
		encoder->processBlock();
		decoder->processBlock();
		logger->log_end(std::chrono::high_resolution_clock::now());
	}

private:
	[[unused]] const std::shared_ptr<switchboard> sb;
	pose_prediction *pp;

	start_end_logger* logger;
	ILLIXR_AUDIO::ABAudio* decoder, *encoder;
	std::chrono::time_point<std::chrono::system_clock> sync;
};

PLUGIN_MAIN(audio_component)
