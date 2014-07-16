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
:config_apply_button("Apply Config"), state_apply_button("Apply State"), launch_button("Launch"), cancel_button("Cancel")
{
  problem_name = "model e2";

  set_border_width(10);

  config_apply_button.set_sensitive(false);
  state_apply_button.set_sensitive(false);

  button_box.pack_end(launch_button, false, false, 0);
  button_box.pack_end(cancel_button, false, false, 0);

  OdeInstanceFactory* inst_fact = OdeInstanceFactoryManager::getInstance()->getFactory( problem_name );

  OdeInstanceWidgetFactory* inst_widget_fact = *OdeInstanceWidgetFactoryManager::getInstance()->getFactoriesFor(inst_fact).first;
  this->config_widget = inst_widget_fact->createConfigWidget();
  vbox.pack_start(*this->config_widget, false, false, 0);
  vbox.pack_start(config_apply_button, false, false, 0);

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
  win_state.add(win_state_vbox);

  this->state_widget = inst_widget_fact->createStateWidget(this->config_widget->getConfig());
  win_state_vbox.pack_start(*this->state_widget, false, false, 0);
  win_state_vbox.pack_start(state_apply_button, false, false, 0);

  // signals //

  config_widget->signal_changed().connect(sigc::mem_fun(*this,
              &HelloWorld::on_config_changed));
  state_widget->signal_changed().connect(sigc::mem_fun(*this,
              &HelloWorld::on_state_changed));

  config_apply_button.signal_clicked().connect(sigc::mem_fun(*this,
              &HelloWorld::on_config_apply));
  state_apply_button.signal_clicked().connect(sigc::mem_fun(*this,
              &HelloWorld::on_state_apply));

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
	config_apply_button.set_sensitive(true);
}
void HelloWorld::on_state_changed()
{
	state_apply_button.set_sensitive(true);
}

void HelloWorld::on_config_apply()
{
	state_widget->loadConfig(config_widget->getConfig());
	config_apply_button.set_sensitive(false);
}
void HelloWorld::on_state_apply()
{
	state_widget->generateState();
	state_apply_button.set_sensitive(false);
}

void HelloWorld::on_launch_clicked()
{
  const OdeConfig* config = config_widget->getConfig();
  const OdeState* init_state = state_widget->getState();
  const OdeSolverConfig* solver_config = solver_config_widget->getConfig();

  OdeInstanceFactory* inst_fact = OdeInstanceFactoryManager::getInstance()->getFactory(problem_name);

  // TODO: think where to create this - with config!
  OdeAnalyzerWidgetFactory* analyzer_fact = *OdeAnalyzerWidgetFactoryManager::getInstance()->getFactoriesFor(inst_fact).first;
  this->analyzer_widget = analyzer_fact->createAnalyzerWidget(config);
  this->analyzer_widget->processState(init_state, 0.0);

  bool as_steps = radio_steps->get_active();
  double time_or_steps;
  if(as_steps)
	  time_or_steps = atof(entry_steps->get_text().c_str());
  else
	  time_or_steps = atof(entry_time->get_text().c_str());

  OdeSolver* solver = OdeSolverFactoryManager::getInstance()->getFactoriesFor(inst_fact).first->createSolver(solver_config, config, init_state);
  const OdeState* final_state = solver->run(time_or_steps, as_steps);
  state_widget->loadState(final_state);

  std::ostringstream buf;
  buf << "sim time: " << solver->getTime();
  label_time->set_text(buf.str());

  buf.str("");
  buf << "sim steps: " << solver->getSteps();
  label_steps->set_text(buf.str());

  delete solver;
}

void HelloWorld::on_cancel_clicked()
{
  Gtk::Main::quit();
}
