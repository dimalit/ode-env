/*
 * gui_e2.h
 *
 *  Created on: Jul 14, 2014
 *      Author: dimalit
 */

#ifndef GUI_E2_H_
#define GUI_E2_H_

#include "gui_factories.h"
#include "model_e2.h"

#include "gtkmm/entry.h"
#include "gtkmm/checkbutton.h"
#include "gtkmm/socket.h"

#include <cassert>
#include <iostream>

class E2ConfigWidget: public OdeConfigWidget{
public:
	typedef E2Config Config;
private:
	E2Config* config;

	Gtk::Entry *entry_n, *entry_a, *entry_delta, *entry_f;
	Gtk::Button *button_apply;

public:
	E2ConfigWidget(const E2Config* config = NULL);

	virtual const OdeConfig* getConfig();
	virtual void loadConfig(const OdeConfig* cfg);

	static std::string getDisplayName(){
		return "GTK+ widgets for E2";
	}

private:
	void widget_to_config();
	void config_to_widget();
	void edit_anything_cb();
	void on_apply_cb();
};

class E2StateWidget: public OdeStateWidget{
public:
	typedef E2State State;
private:
	E2State* state;
	E2State* d_state;
	E2Config* config;

	Gtk::Socket x11_socket;

	Gtk::Entry *entry_left, *entry_right;
	Gtk::CheckButton *radio_rand, *radio_linear;

	Gtk::Label *label_i_left, *label_phi_left, *label_e;
	Gtk::Button *button_apply;

	FILE* to_gnuplot;
public:
	E2StateWidget(const E2Config* config, const E2State* state = NULL);
	virtual ~E2StateWidget();
	virtual void loadState(const OdeState* state, const OdeState* d_state);
	virtual const OdeState* getState();
	virtual const OdeState* getDState();
	virtual void loadConfig(const OdeConfig* config);
	virtual const OdeConfig* getConfig();
	virtual void generateState(bool emit=true);

private:
	void widget_to_state();
	void state_to_widget();
	void edit_anything_cb();
	void on_apply_cb();
	void update_chart();
	void on_realize_cb();
};

class E2PetscSolverConfigWidget: public OdeSolverConfigWidget{
public:
	typedef E2PetscSolver Solver;
private:
	E2PetscSolverConfig* config;

	Gtk::Entry *entry_tol, *entry_step;

public:
	E2PetscSolverConfigWidget(const E2PetscSolverConfig* config = NULL);
	virtual const OdeSolverConfig* getConfig();
	virtual void loadConfig(const OdeSolverConfig* config);

	static std::string getDisplayName(){
		return "PETSc solver for E2 config widget";
	}
private:
	void widget_to_config();
	void config_to_widget();
};

/////////////////////////////////////////////////////////////////////

REGISTER_INSTANCE_WIDGET_CLASSES(E2ConfigWidget, E2StateWidget)
REGISTER_SOLVER_CONFIG_WIDGET_CLASS(E2PetscSolverConfigWidget)

#endif /* GUI_E2_H_ */
