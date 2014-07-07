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

#include <iostream>

HelloWorld::HelloWorld()
:launch_button("Launch"), cancel_button("Cancel")
{
  set_border_width(10);

  button_box.pack_end(launch_button, false, false, 0);
  button_box.pack_end(cancel_button, false, false, 0);

  this->config_widget = new E1ConfigWidget();
  vbox.pack_start(*this->config_widget, false, false, 0);

  const E1Config* ecfg = dynamic_cast<const E1Config*>(this->config_widget->getConfig());
  this->state_widget = new E1StateWidget(ecfg);
  vbox.pack_start(*this->state_widget, false, false, 0);

  this->solver_config_widget = new E1PetscSolverConfigWidget();
  vbox.pack_start(*this->solver_config_widget, false, false, 0);

  vbox.pack_start(button_box, false, false, 0);

  add(vbox);


  launch_button.signal_clicked().connect(sigc::mem_fun(*this,
              &HelloWorld::on_launch_clicked));
  cancel_button.signal_clicked().connect(sigc::mem_fun(*this,
              &HelloWorld::on_cancel_clicked));

  this->show_all();
}

HelloWorld::~HelloWorld()
{
}

void HelloWorld::on_launch_clicked()
{
  const OdeConfig* config = config_widget->getConfig();
  const OdeState* init_state;// = state_widget->getState();
  const OdeSolverConfig* solver_config;// = solver_config_widget->getConfig();

  OdeSolver* solver = E1SolverFactory::getInstance()->createSolver(solver_config, config, init_state);
  solver->step();
  delete solver;
}

void HelloWorld::on_cancel_clicked()
{
  Gtk::Main::quit();
}
