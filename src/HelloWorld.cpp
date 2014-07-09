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

  OdeInstanceFactory* inst_fact = OdeInstanceFactoryManager::getInstance()->getFactory("model e1");

  //this->config_widget = new E1ConfigWidget();
  OdeInstanceWidgetFactory* inst_widget_fact = *OdeInstanceWidgetFactoryManager::getInstance()->getFactoriesFor(inst_fact).first;
  this->config_widget = inst_widget_fact->createConfigWidget();
  vbox.pack_start(*this->config_widget, false, false, 0);

  //const E1Config* ecfg = dynamic_cast<const E1Config*>(this->config_widget->getConfig());
  //this->state_widget = new E1StateWidget(ecfg);
  this->state_widget = inst_widget_fact->createStateWidget(this->config_widget->getConfig());
  vbox.pack_start(*this->state_widget, false, false, 0);

  //this->solver_config_widget = new E1PetscSolverConfigWidget();
  OdeSolverFactory* solver_fact = *OdeSolverFactoryManager::getInstance()->getFactoriesFor(inst_fact).first;
  this->solver_config_widget = OdeSolverConfigWidgetFactoryManager::getInstance()->getFactoriesFor(solver_fact).first->createConfigWidget();
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
  const OdeState* init_state = state_widget->getState();
  const OdeSolverConfig* solver_config = solver_config_widget->getConfig();

  OdeInstanceFactory* inst_fact = OdeInstanceFactoryManager::getInstance()->getFactory("model e1");

  //OdeSolver* solver = E1SolverFactory::getInstance()->createSolver(solver_config, config, init_state);
  OdeSolver* solver = OdeSolverFactoryManager::getInstance()->getFactoriesFor(inst_fact).first->createSolver(solver_config, config, init_state);
  solver->run();
  state_widget->loadState(solver->getCurrentState());
  delete solver;
}

void HelloWorld::on_cancel_clicked()
{
  Gtk::Main::quit();
}
