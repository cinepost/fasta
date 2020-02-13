#include <iostream>

#include "fasta_utils_lib/logging.h"
#include "fasta_lib/renderer_ipr.h"

namespace fst {

FstRendererIPR::FstRendererIPR(): _futureObj(_stopSignal.get_future()) {
	std::cout << "Fasta IPR renderer constructor called." << std::endl;
	_renderer = new FstRenderer();
    std::cout << "Fasta IPR renderer constructed." << std::endl;
}

FstRendererIPR::~FstRendererIPR(){
	std::cout << "Fasta IPR renderer destructor called." << std::endl;

	if(_renderer!=nullptr) 
		delete _renderer;

	std::cout << "Fasta IPR renderer destructed." << std::endl;
}

void FstRendererIPR::renderSamples() {
	while (stopRequested() == false) {
		for(uint i=0; i < _samples; i++){
			_state = State::RUNNING;
	  		while(_pause_ipr){
	  			_state = State::PAUSED;
	     		std::unique_lock<std::mutex> lk(_m);
	     		_cv.wait(lk);
	     		lk.unlock();
	  		}
	  	_renderer->_renderSample(i, _samples);
	  	std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		}
		_state = State::DONE;
		break;
	}

	_state = State::DONE;
	fprintf(stdout, "ipr done\n");
}

void FstRendererIPR::run(uint width, uint height, uint samples) {
	fprintf(stdout, "ipr run\n" );
	_width = width;
	_height = height;
	_samples = samples;
	_pause_ipr = false;
	_render_thread = std::make_unique<std::thread>(std::bind(&FstRendererIPR::renderSamples, this));
	_render_thread->detach();
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

void FstRendererIPR::togglePause() {
	if (_state == State::PAUSED) {
		resume();
	} else if (_state == State::RUNNING) {
		pause();
	}
}

bool FstRendererIPR::stopRequested() {
	// checks if value in future object is available
	if (_futureObj.wait_for(std::chrono::milliseconds(1)) == std::future_status::timeout) return false;
	return true;
}

void FstRendererIPR::resize(uint width, uint height) {
	//std::lock_guard<std::mutex> lk(_m);
	if ((_state == State::RUNNING) && ((_width != width) || (_height != height))){
		fprintf(stdout, "ipr resize\n" );
		//_stopSignal.set_value();
		//_render_thread->join();
		//run(width, height, _samples);
	}
}

} // namespace