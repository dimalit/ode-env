/*
 * RunThread.h
 *
 *  Created on: May 19, 2016
 *      Author: dimalit
 */

#ifndef RUNTHREAD_H_
#define RUNTHREAD_H_

#include <glibmm/main.h>
#include <glibmm/threads.h>

#include "core_interfaces.h"

// TODO: how to use sigc::trackable right?
class RunThread: public sigc::trackable{
private:
	OdeSolver* solver;
	Glib::Threads::Thread* thread;
	Glib::Threads::Mutex mutex;			// protect solver from multithreaded access!!
	Glib::Threads::Cond  cond;			// for synchronization
	sigc::signal<void> m_signal_finished;
	sigc::signal<void> m_signal_step;

public:
	RunThread(OdeSolver*);
	virtual ~RunThread();
	void run(int steps, double time, bool use_step = false);
	void finish();
	sigc::signal<void> getSignalFinished() const {
		return m_signal_finished;
	}
	sigc::signal<void> getSignalStepped() const {
		return m_signal_step;
	}

private:
	int steps;
	double time;
	bool use_step;

	int fd[2];
	Glib::RefPtr<Glib::IOSource> iosource;

	void thread_func();
	bool on_event(Glib::IOCondition);
};

#endif /* RUNTHREAD_H_ */
