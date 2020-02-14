#include <iostream>

#include "fasta_utils_lib/logging.h"
#include "fasta_lib/renderer_ipr.h"

namespace fst {

FstRendererIPR::FstRendererIPR(): _futureObj(_stopSignal.get_future()) {
	LOG_DBG << "Fasta IPR renderer constructor called\n";
	_renderer_initialized = false;
	_renderer = new FstRenderer();
    LOG_DBG << "Fasta IPR renderer constructed\n";
}

FstRendererIPR::~FstRendererIPR(){
	LOG_DBG << "Fasta IPR renderer destructor called\n";

	if(_render_thread->joinable())
		_render_thread->join();

	if(_renderer!=nullptr) 
		delete _renderer;

	LOG_DBG << "Fasta IPR renderer destructed\n";
}

void FstRendererIPR::_renderSamples() {
	for(uint i=0; i < _samples; i++){
		_state = State::RUNNING;
		if(_stop_ipr) {
			_state = State::STOPPED;
			LOG_DBG << "ipr stopped\n";
			return;
		}
  		while(_pause_ipr){
  			_state = State::PAUSED;
     		std::unique_lock<std::mutex> lk(_m);
     		_cv.wait(lk);
     		lk.unlock();
  		}
  	gl_mutex.lock();
  	_renderer->_renderSample();
  	gl_mutex.unlock();
  	//std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}

	_state = State::DONE;
	LOG_DBG << "ipr done\n";
	return;
}

bool FstRendererIPR::init(uint width, uint height, uint samples) {
	_renderer_initialized = false;
	gl_mutex.lock();
	if (_renderer->init(width, height, samples)) {
		_width = width;
		_height = height;
		_samples = samples;
		_renderer_initialized = true;
	}
	gl_mutex.unlock();
	return _renderer_initialized;
}

void FstRendererIPR::run() {
	if (!_renderer_initialized) {
		LOG_DBG << "Fasta IPR not initialized !!!";
		return;
	}

	LOG_DBG << "ipr run\n";
	_pause_ipr = false;
	_stop_ipr = false;
	_render_thread = std::make_unique<std::thread>(std::bind(&FstRendererIPR::_renderSamples, this));
}

bool FstRendererIPR::_isRunning() {
	std::lock_guard<std::mutex> lk(_m);
	if (_state == State::RUNNING )
		return true;

	return false;
}

void FstRendererIPR::stop() {
	std::lock_guard<std::mutex> lk(_m);
	if (_state == State::RUNNING) {
  		_stop_ipr=true;
	}
}

void FstRendererIPR::pause() {
	std::lock_guard<std::mutex> lk(_m);
  	_pause_ipr=true;
  	LOG_DBG << "ipr paused";
}

void FstRendererIPR::resume() {
	std::lock_guard<std::mutex> lk(_m);
  	_pause_ipr=false;
  	_cv.notify_one();
  	LOG_DBG << "ipr resumed";
}

void FstRendererIPR::togglePauseResume() {
	if (_state == State::PAUSED) {
		resume();
	} else if (_state == State::RUNNING) {
		pause();
	}
}

void FstRendererIPR::toggleStartStop() {
	if (_state == State::RUNNING) {
		stop();
	} else if (_state == State::STOPPED) {
		run();
	}
}

void FstRendererIPR::resize(uint width, uint height) {
	//std::lock_guard<std::mutex> lk(_m);
	if (!_renderer_initialized)
		return;

	if ((_width != width) || (_height != height)) {
		stop();
		if(_render_thread->joinable()) {
			LOG_DBG << "wait join\n";
			_render_thread->join();
			LOG_DBG << "joined\n";
		}
		if(init(width, height, _samples)) {
			run();
		}
	}
}

} // namespace