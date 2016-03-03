/*
 ============================================================================
 Name        : ode-env.cpp
 Author      : dimalit
 Version     :
 Copyright   : 
 Description : Hello World in gtkmm
 ============================================================================
 */

#include "HelloWorld.h"

#include <gtkmm/main.h>
#include <gtkmm/builder.h>

#include <iostream>

#include <cstdlib>

#include <fcntl.h>

#define UI_FILE_RUN "sim_params.glade"

HelloWorld::HelloWorld()
:forever_button("Forever"), cancel_button("Cancel"), step_button("Step"), reset_button("Reset")
{
  problem_name = "model e3";

  computing = false;
  run_thread = NULL;
  solver = NULL;
  total_steps = run_steps = 0;
  total_time = run_time = 0.0;

  this->set_title(problem_name);

  set_border_width(10);

  button_box.pack_end(forever_button, false, false, 0);
  button_box.pack_end(step_button, false, false, 0);
  button_box.pack_end(reset_button, false, false, 0);
  button_box.pack_end(cancel_button, false, false, 0);

  vbox.pack_end(button_box, false, false);

  OdeInstanceFactory* inst_fact = OdeInstanceFactoryManager::getInstance()->getFactory( problem_name );

  OdeInstanceWidgetFactory* inst_widget_fact = *OdeInstanceWidgetFactoryManager::getInstance()->getFactoriesFor(inst_fact).first;
  this->config_widget = inst_widget_fact->createConfigWidget();
  vbox.pack_start(*this->config_widget, false, false, 0);

  // win state //
  this->state_widget = inst_widget_fact->createStateWidget(this->config_widget->getConfig());
  vbox.pack_start(*this->state_widget, false, false);

  OdeSolverFactory* solver_fact = *OdeSolverFactoryManager::getInstance()->getFactoriesFor(inst_fact).first;
  this->solver_config_widget = OdeSolverConfigWidgetFactoryManager::getInstance()->getFactoriesFor(solver_fact).first->createConfigWidget();
  vbox.pack_start(*this->solver_config_widget, false, false, 0);

  // simulator config //
  Glib::RefPtr<Gtk::Builder> b = Gtk::Builder::create_from_file(UI_FILE_RUN);

  Gtk::Widget* root;
  b->get_widget("root", root);

  b->get_widget("radio_time", radio_time);
  b->get_widget("radio_steps", radio_steps);

  b->get_widget("entry_time", entry_time);
  	  entry_time->set_text("0.25");
  b->get_widget("entry_steps", entry_steps);
  	  entry_steps->set_text("1");
  b->get_widget("label_time", label_time);
  b->get_widget("label_steps", label_steps);

  show_steps_and_time();

  vbox.pack_start(*root, false, false);

  this->add(vbox);

  // analyzers //
  Gtk::VBox *vb = Gtk::manage(new Gtk::VBox());
  win_analyzers.add(*vb);
  win_analyzers.set_title(inst_widget_fact->getDisplayName() + " analyzers");

  OdeAnalyzerWidgetFactory* analyzer_fact = *OdeAnalyzerWidgetFactoryManager::getInstance()->getFactoriesFor(inst_fact).first;
  this->analyzer_widget = analyzer_fact->createAnalyzerWidget(config_widget->getConfig());
  this->analyzer_widget->processState(state_widget->getState(), state_widget->getDState(), 0.0);
  vb->pack_start(*analyzer_widget, false, false);

  chart_analyzer = new ChartAnalyzer(config_widget->getConfig());
  chart_analyzer->processState(state_widget->getState(), state_widget->getDState(), 0.0);
  pb::E3Special* spec_msg = new pb::E3Special();
  chart_analyzer->addSpecial(spec_msg);
  vb->pack_start(*chart_analyzer, false, false);

  // signals //

  config_widget->signal_changed().connect(sigc::mem_fun(*this,
              &HelloWorld::on_config_changed));
  state_widget->signal_changed().connect(sigc::mem_fun(*this,
              &HelloWorld::on_state_changed));

  forever_button.signal_clicked().connect(sigc::mem_fun(*this,
              &HelloWorld::on_forever_clicked));
  step_button.signal_clicked().connect(sigc::mem_fun(*this,
              &HelloWorld::on_step_clicked));
  reset_button.signal_clicked().connect(sigc::mem_fun(*this,
              &HelloWorld::on_reset_clicked));
  cancel_button.signal_clicked().connect(sigc::mem_fun(*this,
              &HelloWorld::on_cancel_clicked));

  this->show_all();
  win_analyzers.show_all();
}

HelloWorld::~HelloWorld()
{
}

void HelloWorld::on_config_changed()
{
	state_widget->loadConfig(config_widget->getConfig());
}
void HelloWorld::on_state_changed()
{
	analyzer_widget->loadConfig(config_widget->getConfig());
	analyzer_widget->processState(state_widget->getState(), state_widget->getDState(), this->total_time);
	chart_analyzer->processState(state_widget->getState(), state_widget->getDState(), this->total_time);

	const E3State* estate = dynamic_cast<const E3State*>(state_widget->getState());
	const E3Config* config = dynamic_cast<const E3Config*>(config_widget->getConfig());

	// draw energies
	double sum_a_2 = 0;
	double sum_eta = 0;
	for(int i=0; i<estate->particles_size(); i++){
		E3State::Particles p = estate->particles(i);
		sum_a_2 += p.a()*p.a();
		sum_eta += p.eta();
	}
	pb::E3Special spec_msg;
	spec_msg.set_e_2(estate->e()*estate->e());
	spec_msg.set_aver_a_2(sum_a_2/estate->particles_size());
	spec_msg.set_aver_eta(2.0/config->r_e()/config->m()*sum_eta);
	chart_analyzer->processSpecial(&spec_msg, this->total_time);
}

const OdeConfig* HelloWorld::extract_config(){
	return config_widget->getConfig();
}
const OdeState* HelloWorld::extract_state(){
	return state_widget->getState();
}
const OdeSolverConfig* HelloWorld::extract_solver_config(){
	return solver_config_widget->getConfig();
}

void HelloWorld::on_forever_clicked()
{
  if(computing){
	  stop_computing();
	  forever_button.set_label("Forever");
	  forever_button.set_sensitive(false);		// user cannot press it until last iteration ends
	  return;
  }

  // run iteration asynchronously
  run_computing(true);

  // show that iteration is running
  forever_button.set_label("Stop");
}

void HelloWorld::on_step_clicked()
{
	if(computing)
		return;
	run_computing(false);
	stop_computing();
}

void HelloWorld::on_reset_clicked()
{
	assert(!computing);

	if(saved_state.get())
		this->state_widget->loadState(saved_state.get(), saved_dstate.get());

	total_steps = 0;
	total_time = 0.0;
	show_steps_and_time();
	this->chart_analyzer->reset();
}

void HelloWorld::on_cancel_clicked()
{
  Gtk::Main::quit();
}

void HelloWorld::show_steps_and_time(){
	std::ostringstream buf;
	buf << "sim time: " << total_time;
	label_time->set_text(buf.str());

	buf.str("");
	buf << "sim steps: " << total_steps;
	label_steps->set_text(buf.str());
}

void HelloWorld::run_computing(bool use_step){
  assert(!computing);
  assert(!run_thread);
  assert(!solver);

  const OdeConfig* config = extract_config();
  const OdeState* init_state = extract_state();
  const OdeSolverConfig* solver_config = extract_solver_config();

  OdeInstanceFactory* inst_fact = OdeInstanceFactoryManager::getInstance()->getFactory(problem_name);

  bool use_max_steps = radio_steps->get_active();
  steps = atoi(entry_steps->get_text().c_str());
  time = atof(entry_time->get_text().c_str());

  if(use_max_steps)
	  time = 1000000000.0;	// XXX: Why doesn't work 1e+6 or even 1000?
  else
	  steps = 1000000000;

  solver = OdeSolverFactoryManager::getInstance()->getFactoriesFor(inst_fact).first->createSolver(solver_config, config, init_state);

  run_thread = new RunThread(solver);
  run_thread->getSignalFinished().connect(sigc::mem_fun(*this, &HelloWorld::run_finished_cb));
  run_thread->getSignalStepped().connect(sigc::mem_fun(*this, &HelloWorld::run_stepped_cb));

  computing = true;
  step_button.set_sensitive(false);
  reset_button.set_sensitive(false);

  run_steps = 0;
  run_time = 0.0;

  if(use_step)
	  run_thread->run(1000000000, 1000000000.0, use_step);
  else
	  run_thread->run(steps, time, use_step);
}

void HelloWorld::run_stepped_cb(){

	if(total_steps == 0){			// save original state before
		this->saved_state = std::auto_ptr<OdeState>(state_widget->getState()->clone());
		this->saved_dstate = std::auto_ptr<OdeState>(state_widget->getDState()->clone());
		last_refresh_time = 0.0;
	}
	total_steps += solver->getSteps() - run_steps;
	total_time  += solver->getTime()  - run_time;
	run_steps = solver->getSteps();
	run_time  = solver->getTime();
	show_steps_and_time();

	// Написать: упражнение с запуском счета параллельно GUI для студентов
	const OdeState* final_state = solver->getState();
	const OdeState* final_d_state = solver->getDState();

	if(total_steps % steps == 0 || total_time-last_refresh_time >= time){
		state_widget->loadState(final_state, final_d_state);
		last_refresh_time = total_time;
	}
}

void HelloWorld::run_finished_cb(){

	delete run_thread;	run_thread = NULL;
	delete solver;		solver = NULL;
	computing = false;
	forever_button.set_label("Forever");
	forever_button.set_sensitive(true);		// enable it back after last iteration
	step_button.set_sensitive(true);
	reset_button.set_sensitive(true);
}

void HelloWorld::stop_computing(){
	assert(computing);
	run_thread->finish();
	computing = false;
	// all deletions will be done in one_run_completed_cb
}

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
