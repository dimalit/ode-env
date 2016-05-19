/*
 * RunThread.cpp
 *
 *  Created on: May 19, 2016
 *      Author: dimalit
 */
#include "RunThread.h"

#include <cassert>

#include <unistd.h>
#include <fcntl.h>

///////////////////////////// RunThread ///////////////////////////////

RunThread::RunThread(OdeSolver* solver){
	assert(solver);
	this->solver = solver;
	thread = NULL;

	assert(pipe2(fd, O_NONBLOCK) == 0);			// read will return 0 when no data available
	iosource = Glib::IOSource::create(fd[0], Glib::IO_IN);
	iosource->set_can_recurse(false);
	iosource->connect(sigc::mem_fun(*this, &RunThread::on_event));
	iosource->attach();
}

RunThread::~RunThread(){
	if(thread)
		thread->join();
	close(fd[0]);
	close(fd[1]);
	iosource->unreference();
}

void RunThread::run(int steps, double time, bool use_step){
	this->steps = steps;
	this->time = time;
	this->use_step = use_step;
	if(thread)
		thread->join();
	thread = Glib::Threads::Thread::create(sigc::mem_fun(*this, &RunThread::thread_func));
}

void RunThread::finish(){
	mutex.lock();
	if(this->use_step)
		solver->finish();
	mutex.unlock();
}

void RunThread::thread_func(){
	// 1 make long computing
	mutex.lock();
	solver->run(steps, time, use_step);
	mutex.unlock();

	if(use_step){
		for(;;){
			mutex.lock();
			bool step_ok = solver->step();
	//		mutex.unlock();
			if(!step_ok){
				mutex.unlock();
				break;
			}
			char c = 's';
			write(fd[1], &c, 1);		// fire 's'
			cond.wait(mutex);
			mutex.unlock();
		}
	}// use_step

	// 2 after computing is finished - fire 'f' (if use_step==false => step fires exactly once)
	char c = 'f';
	write(fd[1], &c, 1);			// see below:
}

bool RunThread::on_event(Glib::IOCondition){

	// read last bytes and ignore all previous
	char c=0;
	read(fd[0], &c, 1);								// buffer all 's' - but not all states!
	//while(read(fd[0], &c, 1) > 0);				// show what I can

//	fprintf(stderr, "%c\n", c);

	mutex.lock();
	if(c=='s'){
		m_signal_step();
		mutex.unlock();
		cond.signal();
	}
	else if(c=='f'){
		// XXX: signal handler DELETES this object! so we do unlock before. but how to implement this right?!
		mutex.unlock();
		m_signal_step();
		m_signal_finished();
	}
	return 	true;
}


