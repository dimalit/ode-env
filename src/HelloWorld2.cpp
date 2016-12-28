/*
 ============================================================================
 Name        : ode-env.cpp
 Author      : dimalit
 Version     :
 Copyright   : 
 Description : Hello World in gtkmm
 ============================================================================
 */

#include "HelloWorld2.h"

#include <gtkmm/main.h>
#include <gtkmm/builder.h>
#include <gtkmm/socket.h>
#include <gtkmm/alignment.h>
//#include <gdk/x11/gdkx11window.h>

#include <iostream>

#include <cstdlib>

#include <fcntl.h>

#define UI_FILE_RUN "sim_params.glade"

HelloWorld2::HelloWorld2()
:forever_button("Forever"), cancel_button("Cancel"), step_button("Step"), reset_button("Reset")
{
  problem_name = "model e4";

  computing = false;
  run_thread = NULL;
  solver = NULL;
  total_steps = run_steps = 0;
  total_time = run_time = 0.0;
  last_refresh_time = 0.0;

  this->set_title(problem_name);

  set_border_width(10);

  button_box.pack_end(forever_button, false, false, 0);
  button_box.pack_end(step_button, false, false, 0);
  button_box.pack_end(reset_button, false, false, 0);
  button_box.pack_end(cancel_button, false, false, 0);

  vbox.pack_end(button_box, false, false);

  Problem* inst_fact = OdeProblemManager::getInstance()->getProblem( problem_name );

  OdeProblemWidgetType* inst_widget_fact = *OdeInstanceWidgetManager::getInstance()->getTypesFor(inst_fact).first;
  this->config_widget = inst_widget_fact->createConfigWidget();
  vbox.pack_start(*this->config_widget, false, false, 0);

  // win state //
  this->generator_widget = inst_widget_fact->createStateGeneratorWidget(this->config_widget->getConfig());
  vbox.pack_start(*this->generator_widget, false, false);

  SolverType* solver_fact = *OdeSolverTypeManager::getInstance()->getTypesFor(inst_fact).first;
  this->solver_config_widget = OdeSolverConfigWidgetManager::getInstance()->getTypesFor(solver_fact).first->createConfigWidget();
  vbox.pack_start(*this->solver_config_widget, false, false, 0);

  state = this->generator_widget->getState();
  d_state = state->clone();//new E4State(dynamic_cast<const E4Config*>(this->config_widget->getConfig()));

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

  // this is table
  OdeAnalyzerWidgetType* analyzer_fact = *OdeAnalyzerWidgetManager::getInstance()->getTypesFor(inst_fact).first;
  this->analyzer_widget = analyzer_fact->createAnalyzerWidget(config_widget->getConfig());
  this->analyzer_widget->processState(state, d_state, 0.0);
  vb->pack_start(*analyzer_widget, false, false);

  // these are charts with "add" button
  chart_analyzer = new E4ChartAnalyzer(config_widget->getConfig());
  chart_analyzer->processState(state,  d_state, 0.0);
  vb->pack_start(*chart_analyzer, false, false);

  // signals //

  config_widget->signal_changed().connect(sigc::mem_fun(*this,
              &HelloWorld2::on_config_changed));
  generator_widget->signal_changed().connect(sigc::mem_fun(*this,
              &HelloWorld2::on_state_changed));
  forever_button.signal_clicked().connect(sigc::mem_fun(*this,
              &HelloWorld2::on_forever_clicked));
  step_button.signal_clicked().connect(sigc::mem_fun(*this,
              &HelloWorld2::on_step_clicked));
  reset_button.signal_clicked().connect(sigc::mem_fun(*this,
              &HelloWorld2::on_reset_clicked));
  cancel_button.signal_clicked().connect(sigc::mem_fun(*this,
              &HelloWorld2::on_cancel_clicked));

  this->show_all();
  win_analyzers.show_all();

  ///////// diagnostics ///////////

  Gtk::Window* win_diag = new Gtk::Window();
  vb = Gtk::manage(new Gtk::VBox());
  win_diag->add(*vb);

  Gtk::Container* s1 = Gtk::manage(new Gtk::Alignment());
  Gtk::Container* s2 = Gtk::manage(new Gtk::Alignment());
  vb->pack_start(*s1, true, true, 5);
  vb->pack_start(*s2, true, true, 5);

  win_diag->set_title("Diagnostics");
  win_diag->show_all();

  MessageChart *c1 = new MessageChart(std::vector<std::string>({"Wa","Wb", "aver_a_2", "e_2"}), s1);
  c1->setYRange(0, std::numeric_limits<double>::infinity());
  chart_analyzer->addSpecial(c1);

  MessageChart *c2 = new MessageChart(std::vector<std::string>({"Na","Nb", "M"}), s2);
  chart_analyzer->addSpecial(c2);

  Gnuplot::title_translation_map["Wa_aver"] = "W_{v,aver}";
  Gnuplot::title_translation_map["Wb_aver"] = "W_{n,aver}";
  Gnuplot::title_translation_map["Wa"] = "W_v";
  Gnuplot::title_translation_map["Wb"] = "W_n";
  Gnuplot::title_translation_map["aver_a_2"] = "a@^2_{aver}";
  Gnuplot::title_translation_map["Na"] = "N_{v}/N";
  Gnuplot::title_translation_map["Nb"] = "N_{n}/N";
  Gnuplot::title_translation_map["N"] = "Nv/N+Nn/N";
  Gnuplot::title_translation_map["M"] = "Nv/N-Nn/N";
  Gnuplot::title_translation_map["e_2"] = "E^2";
}

HelloWorld2::~HelloWorld2()
{
}

void HelloWorld2::on_config_changed()
{
	d_state = state->clone();//new E4State(dynamic_cast<const E4Config*>(config_widget->getConfig()));
	generator_widget->loadConfig(config_widget->getConfig());
}
void HelloWorld2::on_state_changed(){
	this->state = generator_widget->getState();
	show_new_state();
}
void HelloWorld2::show_new_state()
{
	analyzer_widget->loadConfig(config_widget->getConfig());
	analyzer_widget->processState(state, d_state, this->total_time);
	chart_analyzer->processState(state, d_state, this->total_time);
}

const OdeConfig* HelloWorld2::extract_config(){
	return config_widget->getConfig();
}
const OdeState* HelloWorld2::extract_state(){
	return generator_widget->getState();
}
const OdeSolverConfig* HelloWorld2::extract_solver_config(){
	return solver_config_widget->getConfig();
}

void HelloWorld2::on_forever_clicked()
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

void HelloWorld2::on_step_clicked()
{
	if(computing)
		return;
	run_computing(false);
	stop_computing();
}

void HelloWorld2::on_reset_clicked()
{
	assert(!computing);

	this->generator_widget->newState();

	total_steps = 0;
	total_time = 0.0;
	last_refresh_time = 0.0;

	show_steps_and_time();
	this->chart_analyzer->reset();
}

void HelloWorld2::on_cancel_clicked()
{
  Gtk::Main::quit();
}

void HelloWorld2::show_steps_and_time(){
	std::ostringstream buf;
	buf << "sim time: " << total_time;
	label_time->set_text(buf.str());

	buf.str("");
	buf << "sim steps: " << total_steps;
	label_steps->set_text(buf.str());
}

void HelloWorld2::run_computing(bool use_step){
  assert(!computing);
  assert(!run_thread);
  assert(!solver);

  const OdeConfig* config = extract_config();
  const OdeSolverConfig* solver_config = extract_solver_config();

  Problem* inst_fact = OdeProblemManager::getInstance()->getProblem(problem_name);

  bool use_max_steps = radio_steps->get_active();
  steps = atoi(entry_steps->get_text().c_str());
  time = atof(entry_time->get_text().c_str());

  if(use_max_steps)
	  time = 1000000000.0;	// XXX: Why doesn't work 1e+6 or even 1000?
  else
	  steps = 1000000000;

  solver = OdeSolverTypeManager::getInstance()->getTypesFor(inst_fact).first->createSolver(solver_config, config, state);

  run_thread = new RunThread(solver);
  run_thread->getSignalFinished().connect(sigc::mem_fun(*this, &HelloWorld2::run_finished_cb));
  run_thread->getSignalStepped().connect(sigc::mem_fun(*this, &HelloWorld2::run_stepped_cb));

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

void HelloWorld2::run_stepped_cb(){

	total_steps += solver->getSteps() - run_steps;
	total_time  += solver->getTime()  - run_time;
	run_steps = solver->getSteps();
	run_time  = solver->getTime();
	show_steps_and_time();

	// Написать: упражнение с запуском счета параллельно GUI для студентов
	const OdeState* final_state = solver->getState();
	const OdeState* final_d_state = solver->getDState();

	if(total_steps % steps == 0 || total_time-last_refresh_time >= time){
		this->state = final_state;
		this->d_state = final_d_state;
		this->show_new_state();
		last_refresh_time = total_time;
	}
}

void HelloWorld2::run_finished_cb(){
	this->state = solver->getState()->clone();
	delete run_thread;	run_thread = NULL;
	delete solver;		solver = NULL;
	computing = false;
	forever_button.set_label("Forever");
	forever_button.set_sensitive(true);		// enable it back after last iteration
	step_button.set_sensitive(true);
	reset_button.set_sensitive(true);
}

void HelloWorld2::stop_computing(){
	assert(computing);
	run_thread->finish();
	computing = false;
	// all deletions will be done in one_run_completed_cb
}

void HelloWorld2::on_plug_added(){
	std::cerr << "added!!\n";
	std::cerr.flush();
}
