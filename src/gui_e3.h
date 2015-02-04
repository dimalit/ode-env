/*
 * gui_e3.h
 *
 *  Created on: Aug 21, 2014
 *      Author: dimalit
 */

#ifndef GUI_E3_H_
#define GUI_E3_H_

#include "gui_factories.h"
#include "model_e3.h"

#include "gtkmm/entry.h"

#include <cassert>
#include <iostream>

class E3ConfigWidget: public OdeConfigWidget{
public:
	typedef E3Config Config;
private:
	E3Config* config;

	Gtk::Entry *entry_m, *entry_n, *entry_theta, *entry_gamma, *entry_delta, *entry_r;
	Gtk::Button *button_apply;

public:
	E3ConfigWidget(const E3Config* config = NULL);

	virtual const OdeConfig* getConfig();
	virtual void loadConfig(const OdeConfig* cfg);

	static std::string getDisplayName(){
		return "GTK+ widgets for E3";
	}

private:
	void widget_to_config();
	void config_to_widget();
	void edit_anything_cb();
	void on_apply_cb();
};

class E3StateWidget: public OdeStateWidget{
public:
	typedef E3State State;
private:
	E3State* state;
	E3State* d_state;
	E3Config* config;

	Gtk::Entry *entry_e, *entry_phi, *entry_a;
	Gtk::Button *button_apply;

public:
	E3StateWidget(const E3Config* config, const E3State* state = NULL);
	virtual ~E3StateWidget();
	virtual void loadState(const OdeState* state, const OdeState* d_state);
	virtual const OdeState* getState();
	virtual const OdeState* getDState();
	virtual void loadConfig(const OdeConfig* config);
	virtual const OdeConfig* getConfig();
	virtual void generateState();

private:
	void widget_to_state();
	void state_to_widget();
	void edit_anything_cb();
	void on_apply_cb();
};

class E3PetscSolverConfigWidget: public OdeSolverConfigWidget{
public:
	typedef E3PetscSolver Solver;
private:
	E3PetscSolverConfig* config;

	Gtk::Entry *entry_tol, *entry_step;

public:
	E3PetscSolverConfigWidget(const E3PetscSolverConfig* config = NULL);
	virtual const OdeSolverConfig* getConfig();
	virtual void loadConfig(const OdeSolverConfig* config);

	static std::string getDisplayName(){
		return "PETSc solver for E3 config widget";
	}
private:
	void widget_to_config();
	void config_to_widget();
};

/////////////////////////////////////////////////////////////////////

REGISTER_INSTANCE_WIDGET_CLASSES(E3ConfigWidget, E3StateWidget)
REGISTER_SOLVER_CONFIG_WIDGET_CLASS(E3PetscSolverConfigWidget)

#endif /* GUI_E3_H_ */
