/*
 * gui_e3.h
 *
 *  Created on: Aug 21, 2014
 *      Author: dimalit
 */

#ifndef GUI_E4_H_
#define GUI_E4_H_

#include "gui_factories.h"
#include "model_e4.h"

#include <google/protobuf/message.h>

#include "gtkmm/grid.h"
#include "gtkmm/entry.h"
#include "gtkmm/comboboxtext.h"
#include "gtkmm/adjustment.h"

#include <cassert>
#include <iostream>

using namespace google::protobuf;

class EXConfigWidget: public OdeConfigWidget{
//!!! TODO: remove this - in gui_interfaces
public:
	typedef E4Config Config;
private:
	OdeConfig* config;

	Gtk::Grid grid;
	Gtk::Button button_apply;

	std::map<string, Gtk::Entry*> entry_map;

public:
	EXConfigWidget(const OdeConfig* config = NULL);

	virtual const OdeConfig* getConfig();
	virtual void loadConfig(const OdeConfig* cfg);

	static std::string getDisplayName(){
		return "GTK+ widgets for E4";
	}

private:
	void widget_to_config();
	void config_to_widget();
	void edit_anything_cb();
	void on_apply_cb();
};

class E4StateGeneratorWidget: public OdeStateGeneratorWidget{
public:
	typedef E4State State;
private:
	E4State* state;
	E4Config* config;

	Gtk::Entry *entry_e, *entry_phi, *entry_a0;
	Gtk::Button *button_apply;

public:
	E4StateGeneratorWidget(const E4Config* config);
	virtual ~E4StateGeneratorWidget();
	virtual const OdeState* getState();
	virtual void loadConfig(const OdeConfig* config);
	virtual const OdeConfig* getConfig();
	virtual void newState(bool emit=true);

private:
	void edit_anything_cb();
	void on_apply_cb();
};

class E4PetscSolverConfigWidget: public OdeSolverConfigWidget{
public:
	typedef E4PetscSolver Solver;
private:
	E4PetscSolverConfig* config;

	Gtk::Entry *entry_atol, *entry_rtol, *entry_step;
	Glib::RefPtr<Gtk::Adjustment> adj_n_cores;

public:
	E4PetscSolverConfigWidget(const E4PetscSolverConfig* config = NULL);
	virtual const OdeSolverConfig* getConfig();
	virtual void loadConfig(const OdeSolverConfig* config);

	static std::string getDisplayName(){
		return "PETSc solver for E4 config widget";
	}
private:
	void widget_to_config();
	void config_to_widget();
};

/////////////////////////////////////////////////////////////////////

REGISTER_INSTANCE_WIDGET_CLASSES(EXConfigWidget, E4StateGeneratorWidget)
REGISTER_SOLVER_CONFIG_WIDGET_CLASS(E4PetscSolverConfigWidget)

#endif /* GUI_E4_H_ */
