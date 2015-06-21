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

#define UI_FILE_RUN "sim_params.glade"

HelloWorld::HelloWorld()
:forever_button("Forever"), cancel_button("Cancel"), step_button("Step"), reset_button("Reset")
{
  problem_name = "model e3";

  computing = false;
  run_thread = NULL;
  solver = NULL;

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
  	  entry_time->set_text("0.1");
  b->get_widget("entry_steps", entry_steps);
  	  entry_steps->set_text("1");
  b->get_widget("label_time", label_time);
  b->get_widget("label_steps", label_steps);

  set_steps_and_time(0, 0.0);

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
  run_computing();

  // show that iteration is running
  forever_button.set_label("Stop");
}

void HelloWorld::on_step_clicked()
{
	if(computing)
		return;
	run_computing();
	stop_computing();
}

void HelloWorld::on_reset_clicked()
{
	assert(!computing);

	if(saved_state.get())
		this->state_widget->loadState(saved_state.get(), saved_dstate.get());

	set_steps_and_time(0, 0.0);
	this->chart_analyzer->reset();
}

void HelloWorld::on_cancel_clicked()
{
  Gtk::Main::quit();
}

void HelloWorld::set_steps_and_time(int steps, double time){
	total_steps = steps;
	total_time = time;

	std::ostringstream buf;
	buf << "sim time: " << total_time;
	label_time->set_text(buf.str());

	buf.str("");
	buf << "sim steps: " << total_steps;
	label_steps->set_text(buf.str());
}
void HelloWorld::add_steps_and_time(int steps, double time){
	set_steps_and_time(total_steps + steps, total_time + time);
}

void HelloWorld::run_computing(){
  assert(!computing);
  assert(!run_thread);
  assert(!solver);

  const OdeConfig* config = extract_config();
  const OdeState* init_state = extract_state();
  const OdeSolverConfig* solver_config = extract_solver_config();

  OdeInstanceFactory* inst_fact = OdeInstanceFactoryManager::getInstance()->getFactory(problem_name);

  bool use_steps = radio_steps->get_active();
  steps = atoi(entry_steps->get_text().c_str());
  time = atof(entry_time->get_text().c_str());

  if(use_steps)
	  time = 1000000000.0;	// XXX: Why doesn't work 1e+6 or even 1000?
  else
	  steps = 1000000000;

  solver = OdeSolverFactoryManager::getInstance()->getFactoriesFor(inst_fact).first->createSolver(solver_config, config, init_state);

  run_thread = new RunThread(solver);
  run_thread->getSignalFinished().connect(sigc::mem_fun(*this, &HelloWorld::one_run_completed_cb));

  computing = true;
  step_button.set_sensitive(false);
  reset_button.set_sensitive(false);
  run_thread->run(steps, time);
}

void HelloWorld::one_run_completed_cb(const OdeState* final_state, const OdeState* final_d_state){

	if(total_steps == 0){			// save original state before
		this->saved_state = std::auto_ptr<OdeState>(state_widget->getState()->clone());
		this->saved_dstate = std::auto_ptr<OdeState>(state_widget->getDState()->clone());
	}
	add_steps_and_time(solver->getSteps(), solver->getTime());

	// Написать: упражнение с запуском счета параллельно GUI для студентов
	state_widget->loadState(final_state, final_d_state);

	// one more iteration if needed
	if(computing){
	  run_thread->run(steps, time);
	}// if
	else{
		delete run_thread;	run_thread = NULL;
		delete solver;		solver = NULL;
		forever_button.set_sensitive(true);		// enable it back after last iteration
		step_button.set_sensitive(true);
		reset_button.set_sensitive(true);
	}// else
}

void HelloWorld::stop_computing(){
	assert(computing);
	computing = false;
	// all deletions will be done in one_run_completed_cb
}

///////////////////////////// RunThread ///////////////////////////////

RunThread::RunThread(OdeSolver* solver){
	assert(solver);
	this->solver = solver;
	thread = NULL;

	assert(pipe(fd) == 0);
	iosource = Glib::IOSource::create(fd[0], Glib::IO_IN);
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

void RunThread::run(int steps, double time){
	this->steps = steps;
	this->time = time;
	if(thread)
		thread->join();
	thread = Glib::Threads::Thread::create(sigc::mem_fun(*this, &RunThread::thread_func));
}

void RunThread::thread_func(){
	// 1 make long computing
	solver->run(steps, time);
	final_state = solver->getState();
	final_d_state = solver->getDState();
	// 2 after computing is finished - fire IOSource
	char c = 0;
	write(fd[1], &c, 1);
}

bool RunThread::on_event(Glib::IOCondition){
	char c;
	read(fd[0], &c, 1);

	m_signal_finished(final_state, final_d_state);
	return 	true;
}
