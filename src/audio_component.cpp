#include <chrono>
#include <future>
#include <fstream>
#include <thread>

#include "common/threadloop.hpp"
#include "common/switchboard.hpp"
#include "common/data_format.hpp"
#include "common/phonebook.hpp"
#include "common/logger.hpp"

#include <audio.h>

using namespace ILLIXR;

#define AUDIO_EPOCH (1024.0/48000.0)

class audio_component : public threadloop
{
public:
	audio_component(std::string name_, phonebook *pb_)
		: threadloop{name_, pb_}
		, sb{pb->lookup_impl<switchboard>()}
		, _m_pose{sb->subscribe_latest<pose_type>("slow_pose")}
		, decoder{"", ILLIXR_AUDIO::ABAudio::ProcessType::DECODE}
		, encoder{"", ILLIXR_AUDIO::ABAudio::ProcessType::ENCODE}
		, last_iteration{std::chrono::high_resolution_clock::now()}
	{
		decoder.loadSource();
		encoder.loadSource();
	}

	virtual skip_option _p_should_skip() override {
		// Could just check time and go back to sleep
		// But actually blocking here is more efficient, because I wake up fewer times,
		// reliable_sleep guarantees responsiveness (to `stop()`) and accuracy
		last_iteration += std::chrono::milliseconds{21};
		std::this_thread::sleep_for(last_iteration - std::chrono::high_resolution_clock::now());
		return skip_option::run;
	}

	virtual void _p_one_iteration() override {
		[[maybe_unused]] auto most_recent_pose = _m_pose->get_latest_ro();
		encoder.processBlock();
		decoder.processBlock();
	}

private:
	const std::shared_ptr<switchboard> sb;
	std::unique_ptr<reader_latest<pose_type>> _m_pose;
	ILLIXR_AUDIO::ABAudio decoder, encoder;
	std::chrono::high_resolution_clock::time_point last_iteration;
};

PLUGIN_MAIN(audio_component)
