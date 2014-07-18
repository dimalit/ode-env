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
:launch_button("Launch"), cancel_button("Cancel")
{
  problem_name = "model e2";

  computing = false;
  run_thread = NULL;
  solver = NULL;

  this->set_title(problem_name);

  set_border_width(10);

  button_box.pack_end(launch_button, false, false, 0);
  button_box.pack_end(cancel_button, false, false, 0);

  OdeInstanceFactory* inst_fact = OdeInstanceFactoryManager::getInstance()->getFactory( problem_name );

  OdeInstanceWidgetFactory* inst_widget_fact = *OdeInstanceWidgetFactoryManager::getInstance()->getFactoriesFor(inst_fact).first;
  this->config_widget = inst_widget_fact->createConfigWidget();
  vbox.pack_start(*this->config_widget, false, false, 0);

  OdeSolverFactory* solver_fact = *OdeSolverFactoryManager::getInstance()->getFactoriesFor(inst_fact).first;
  this->solver_config_widget = OdeSolverConfigWidgetFactoryManager::getInstance()->getFactoriesFor(solver_fact).first->createConfigWidget();
  vbox.pack_start(*this->solver_config_widget, false, false, 0);

  // global config //
  Glib::RefPtr<Gtk::Builder> b = Gtk::Builder::create_from_file(UI_FILE_RUN);

  Gtk::Widget* root;
  b->get_widget("root", root);

  b->get_widget("radio_time", radio_time);
  b->get_widget("radio_steps", radio_steps);

  b->get_widget("entry_time", entry_time);
  	  entry_time->set_text("1");
  b->get_widget("entry_steps", entry_steps);
  	  entry_steps->set_text("100");
  b->get_widget("label_time", label_time);
  b->get_widget("label_steps", label_steps);
  label_time->set_text("sim time: 0");
  label_steps->set_text("sim steps: 0");

  vbox.pack_start(*root, false, false);

  vbox.pack_start(button_box, false, false, 0);

  add(vbox);

  // win state //
  this->state_widget = inst_widget_fact->createStateWidget(this->config_widget->getConfig());
  win_state.add(*this->state_widget);

  // analyzers //
  OdeAnalyzerWidgetFactory* analyzer_fact = *OdeAnalyzerWidgetFactoryManager::getInstance()->getFactoriesFor(inst_fact).first;
  this->analyzer_widget = analyzer_fact->createAnalyzerWidget(config_widget->getConfig());
  this->analyzer_widget->processState(state_widget->getState(), 0.0);

  // signals //

  config_widget->signal_changed().connect(sigc::mem_fun(*this,
              &HelloWorld::on_config_changed));
  state_widget->signal_changed().connect(sigc::mem_fun(*this,
              &HelloWorld::on_state_changed));

  launch_button.signal_clicked().connect(sigc::mem_fun(*this,
              &HelloWorld::on_launch_clicked));
  cancel_button.signal_clicked().connect(sigc::mem_fun(*this,
              &HelloWorld::on_cancel_clicked));

  this->show_all();
  win_state.show_all();
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
	analyzer_widget->processState(state_widget->getState(), 0.0);
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

void HelloWorld::on_launch_clicked()
{
  if(computing){
	  stop_computing();
	  launch_button.set_label("Launch");
	  launch_button.set_sensitive(false);		// user cannot press it until last iteration ends
	  return;
  }

  // run iteration asynchronously
  run_computing();

  // show that iteration is running
  launch_button.set_label("Stop");
}

void HelloWorld::on_cancel_clicked()
{
  Gtk::Main::quit();
}

void HelloWorld::run_computing(){
  assert(!computing);
  assert(!run_thread);
  assert(!solver);

  const OdeConfig* config = extract_config();
  const OdeState* init_state = extract_state();
  const OdeSolverConfig* solver_config = extract_solver_config();

  OdeInstanceFactory* inst_fact = OdeInstanceFactoryManager::getInstance()->getFactory(problem_name);

  as_steps = radio_steps->get_active();
  if(as_steps)
	  time_or_steps = atof(entry_steps->get_text().c_str());
  else
	  time_or_steps = atof(entry_time->get_text().c_str());

  solver = OdeSolverFactoryManager::getInstance()->getFactoriesFor(inst_fact).first->createSolver(solver_config, config, init_state);

  run_thread = new RunThread(solver);
  run_thread->getSignalFinished().connect(sigc::mem_fun(*this, &HelloWorld::one_run_completed_cb));

  computing = true;
  run_thread->run(time_or_steps, as_steps);
}

void HelloWorld::one_run_completed_cb(const OdeState* final_state){
	  state_widget->loadState(final_state);

	  // Ноаписать: упражнение с запуском счета параллельно GUI для студентов

	  std::ostringstream buf;
	  buf << "sim time: " << solver->getTime();
	  label_time->set_text(buf.str());

	  buf.str("");
	  buf << "sim steps: " << solver->getSteps();
	  label_steps->set_text(buf.str());

	  // one more iteration if needed
	  if(computing){
		  run_thread->run(time_or_steps, as_steps);
	  }// if
	  else{
			delete run_thread;	run_thread = NULL;
			delete solver;		solver = NULL;
			launch_button.set_sensitive(true);		// it was disabled in on_launch_clicked()
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
}

void RunThread::run(double time_or_steps, bool as_steps){
	this->time_or_steps = time_or_steps;
	this->as_steps = as_steps;
	if(thread)
		thread->join();
	thread = Glib::Threads::Thread::create(sigc::mem_fun(*this, &RunThread::thread_func));
}

void RunThread::thread_func(){
	// 1 make long computing
	final_state = solver->run(time_or_steps, as_steps);
	// 2 after computing is finished - fire IOSource
	char c = 0;
	write(fd[1], &c, 1);
}

bool RunThread::on_event(Glib::IOCondition){
	char c;
	read(fd[0], &c, 1);

	m_signal_finished(final_state);
	return 	true;
}
