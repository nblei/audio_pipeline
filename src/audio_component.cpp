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
	audio_component(std::string name_, phonebook *pb_)
		: threadloop{name_, pb_}
		, pp{pb->lookup_impl<pose_prediction>()}
		, decoder{"", ILLIXR_AUDIO::ABAudio::ProcessType::DECODE}
		, encoder{"", ILLIXR_AUDIO::ABAudio::ProcessType::ENCODE}
		, last_iteration{std::chrono::high_resolution_clock::now()}
		, logger{"audio"}
	{
		decoder.loadSource();
		encoder.loadSource();
	}

	virtual skip_option _p_should_skip() override {
		// Could just check time and go back to sleep
		// But actually blocking here is more efficient, because I wake up fewer times,
		// reliable_sleep guarantees responsiveness (to `stop()`) and accuracy
		reliable_sleep(last_iteration += std::chrono::milliseconds{21});
		return skip_option::run;
	}

	virtual void _p_one_iteration() override {
		logger.log_start(std::chrono::high_resolution_clock::now());
		[[maybe unused]] auto most_recent_pose = pp->get_fast_pose();
		encoder.processBlock();
		decoder.processBlock();
		logger.log_end(std::chrono::high_resolution_clock::now());
	}

private:
	std::shared_ptr<const pose_prediction> pp;
	ILLIXR_AUDIO::ABAudio decoder, encoder;
	std::chrono::high_resolution_clock::time_point last_iteration;
	start_end_logger logger;
};

PLUGIN_MAIN(audio_component)
