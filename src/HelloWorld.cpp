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
#include <gtkmm/socket.h>
#include <gtkmm/alignment.h>

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
  last_refresh_time = 0.0;

  sum_ia = 0.0; sum_ib = 0.0;
  sum_wa = 0.0; sum_wb = 0.0;

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
  this->generator_widget = inst_widget_fact->createStateGeneratorWidget(this->config_widget->getConfig());
  vbox.pack_start(*this->generator_widget, false, false);

  OdeSolverFactory* solver_fact = *OdeSolverFactoryManager::getInstance()->getFactoriesFor(inst_fact).first;
  this->solver_config_widget = OdeSolverConfigWidgetFactoryManager::getInstance()->getFactoriesFor(solver_fact).first->createConfigWidget();
  vbox.pack_start(*this->solver_config_widget, false, false, 0);

  state = this->generator_widget->getState();
  d_state = new E3State(dynamic_cast<const E3Config*>(this->config_widget->getConfig()));

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
  this->analyzer_widget->processState(state, d_state, 0.0);
  vb->pack_start(*analyzer_widget, false, false);

  chart_analyzer = new ChartAnalyzer(config_widget->getConfig());
  chart_analyzer->processState(state,  d_state, 0.0);
  pb::E3Special* spec_msg = new pb::E3Special();
  chart_analyzer->addSpecial(spec_msg);
  vb->pack_start(*chart_analyzer, false, false);

  // signals //

  config_widget->signal_changed().connect(sigc::mem_fun(*this,
              &HelloWorld::on_config_changed));
  generator_widget->signal_changed().connect(sigc::mem_fun(*this,
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

  ///////// diagnostics ///////////

  Gtk::Window* win_diag = new Gtk::Window();
  vb = Gtk::manage(new Gtk::VBox());

  Gtk::Widget* s1 = chart_analyzer->addChart(spec_msg,std::vector<std::string>({"Wa","Wb", "aver_a_2"}),"",false,std::numeric_limits<double>::infinity());
  Gtk::Widget* s2 = chart_analyzer->addChart(spec_msg,std::vector<std::string>({"Na","Nb", "N", "M"}),"",false);
  Gtk::Widget* s3 = chart_analyzer->addChart(spec_msg,std::vector<std::string>({"e_2", "aver_a_2"}),"",false);

  vb->pack_start(*s1, true, true, 5);
  vb->pack_start(*s2, true, true, 5);
  vb->pack_start(*s3, true, true, 5);

  win_diag->add(*vb);
  win_diag->set_title("Diagnostics");
  win_diag->show_all();

  Gnuplot::title_translation_map["Wa_aver"] = "W_{v,aver}";
  Gnuplot::title_translation_map["Wb_aver"] = "W_{n,aver}";
  Gnuplot::title_translation_map["Wa"] = "W_v";
  Gnuplot::title_translation_map["Wb"] = "W_n";
  Gnuplot::title_translation_map["aver_a_2"] = "a@^2_{aver}";
  Gnuplot::title_translation_map["Na"] = "N_{v}/N";
  Gnuplot::title_translation_map["Nb"] = "N_{n}/N";
  Gnuplot::title_translation_map["N"] = "Nv/N+Nn/N";
  Gnuplot::title_translation_map["M"] = "Nv/N-Nn/N";
  Gnuplot::title_translation_map["e_2"] = "e^2";
}

HelloWorld::~HelloWorld()
{
}

void HelloWorld::on_config_changed()
{
	d_state = new E3State(dynamic_cast<const E3Config*>(config_widget->getConfig()));
	generator_widget->loadConfig(config_widget->getConfig());
}
void HelloWorld::on_state_changed(){
	this->state = generator_widget->getState();
	show_new_state();
}
void HelloWorld::show_new_state()
{
	analyzer_widget->loadConfig(config_widget->getConfig());
	analyzer_widget->processState(state, d_state, this->total_time);
	chart_analyzer->processState(state, d_state, this->total_time);

	pb::E3Special spec_msg;
	fill_spec_msg(&spec_msg);
	chart_analyzer->processSpecial(&spec_msg, this->total_time);
}

void HelloWorld::fill_spec_msg(pb::E3Special* spec_msg){
	const E3State* estate = dynamic_cast<const E3State*>(state);
	const E3State* dstate = dynamic_cast<const E3State*>(this->d_state);
	const E3Config* config = dynamic_cast<const E3Config*>(config_widget->getConfig());

	int N = estate->particles_size();
	double T = this->total_time;
	double dT = this->total_time - this->last_refresh_time;

	double sum_a_2 = 0;
	double sum_eta = 0;
	double Na = 0, Nb = 0;			// da/dt>0 and <0
	double Ia = 0.0, Ib = 0.0;	// sum da/dt
	double Wa = 0.0, Wb = 0.0;	// sum a^2
	for(int i=0; i<N; i++){
		E3State::Particles p = estate->particles(i);
		E3State::Particles dp = dstate->particles(i);

		sum_a_2 += p.a()*p.a();
		sum_eta += p.eta();

		if(dp.a() > 0.0){
			Na++;
			Ia += dp.a();
			Wa += p.a()*p.a();
		}// Na
		else{
			Nb++;
			Ib += dp.a();
			Wb += p.a()*p.a();
		}// Nb
	}

	Wa/=N; Wb/=N;
	Na/=N; Nb/=N;

	spec_msg->set_e_2(estate->e()*estate->e());
	spec_msg->set_aver_a_2(sum_a_2/estate->particles_size());

	spec_msg->set_na(Na);
	spec_msg->set_nb(Nb);
	spec_msg->set_wa(Wa);
	spec_msg->set_wb(Wb);

	spec_msg->set_n(Na+Nb);
	spec_msg->set_m(Na-Nb);
}

const OdeConfig* HelloWorld::extract_config(){
	return config_widget->getConfig();
}
const OdeState* HelloWorld::extract_state(){
	return generator_widget->getState();
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

	this->generator_widget->newState();

	total_steps = 0;
	total_time = 0.0;
	last_refresh_time = 0.0;

	sum_ia = 0.0; sum_ib = 0.0;
	sum_wa = 0.0; sum_wb = 0.0;

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
  const OdeSolverConfig* solver_config = extract_solver_config();

  OdeInstanceFactory* inst_fact = OdeInstanceFactoryManager::getInstance()->getFactory(problem_name);

  bool use_max_steps = radio_steps->get_active();
  steps = atoi(entry_steps->get_text().c_str());
  time = atof(entry_time->get_text().c_str());

  if(use_max_steps)
	  time = 1000000000.0;	// XXX: Why doesn't work 1e+6 or even 1000?
  else
	  steps = 1000000000;

  solver = OdeSolverFactoryManager::getInstance()->getFactoriesFor(inst_fact).first->createSolver(solver_config, config, state);

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

void HelloWorld::run_finished_cb(){
	this->state = solver->getState()->clone();
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
