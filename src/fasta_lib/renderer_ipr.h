#ifndef __FASTA_RENDERER_IPR_H__
#define __FASTA_RENDERER_IPR_H__

#include <stdio.h>
#include <stdlib.h>
#include <thread>
#include <mutex>
#include <chrono>
#include <future>
#include <condition_variable>
#include <functional>

#include <glad/glad.h>

#include "fasta_lib/renderer.h"

namespace fst {

class FstRendererIPR: public FstRenderer {
	enum class State { RUNNING, PAUSED, STOPPED, DONE };

	public:
		FstRendererIPR();
		~FstRendererIPR();
		
		void run(uint width, uint height, uint samples);
		void pause();
		void resume();
		void togglePause(); // pause/resume

		void resize(uint width, uint height); // restarts ipr with new resolution

	private:
		void renderSamples();
		bool stopRequested();

	private:
		std::unique_ptr<std::thread> _render_thread;
		FstRenderer *_renderer;

	private:
		uint _width;
		uint _height;
		uint _samples;
		std::atomic<State> _state; // ipr state

		std::atomic<bool>  _pause_ipr; // request state
		std::promise<void> _stopSignal;
		std::future<void>  _futureObj;

	private:
		std::condition_variable _cv;
		std::mutex _m;
};

} // namespace

#endif // __FASTA_RENDERER_IPR_H__