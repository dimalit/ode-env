/*
 * gui_e1.h
 *
 *  Created on: Jul 4, 2014
 *      Author: dimalit
 */

#ifndef GUI_E1_H_
#define GUI_E1_H_

#include "model_e1.h"

#include <gui_factories.h>

#include <gtkmm/entry.h>
#include <gtkmm/checkbutton.h>

#include <goffice/goffice.h>

#include <cassert>
#include <iostream>

class E1ConfigWidget: public OdeConfigWidget{
public:
	typedef E1Config Config;
private:
	mutable E1Config* config;

	Gtk::Entry *entry_m, *entry_ksi;
	Gtk::CheckButton *check_rand, *check_linear;

public:
	E1ConfigWidget(const E1Config* config = NULL);
	// TODO: maybe clone?
	virtual const OdeConfig* getConfig() const;
	virtual void loadConfig(const OdeConfig* cfg);

	static std::string getDisplayName(){
		return "GTK+ widgets for E1";
	}

private:
	void widget_to_config() const;
	void config_to_widget();
	void check_linear_toggled_cb();
	void check_rand_toggled_cb();
};

class E1StateGeneratorWidget: public OdeStateGeneratorWidget{
public:
	typedef E1State State;
private:
	E1State* state;
	E1State* d_state;
	E1Config* config;

	Glib::RefPtr<Gtk::Adjustment> adj_E, adj_phi, adj_b;
	Gtk::Label *label_phi;

	// chart:
	GtkWidget *ggw;
	GogSeries *series_E, *series_b, *series_phi;

	GOData *series_E_data;
	double series_E_xs[2];
	double series_E_ys[2];

	GOData *series_b_data;
	double* series_b_xs;
	double* series_b_ys;

	GOData *series_phi_data_x;
	GOData *series_phi_data_y;
	double series_phi_xs[21];
	double series_phi_ys[21];

public:
	E1StateGeneratorWidget(const E1Config* config);
	virtual const OdeState* getState();
	virtual void loadConfig(const OdeConfig* config);
	virtual const OdeConfig* getConfig();
	virtual void newState(bool emit=true);

private:
	void widget_to_state();
	void state_to_widget();
	void draw(GogChart* chart);
	void adj_E_value_changed_cb();
	void adj_b_value_changed_cb();
	void adj_phi_value_changed_cb();
};

class E1PetscSolverConfigWidget: public OdeSolverConfigWidget{
public:
	typedef E1PetscSolver Solver;
private:
	E1PetscSolverConfig* config;

	Gtk::Entry *entry_tol, *entry_step;
public:
	E1PetscSolverConfigWidget(const E1PetscSolverConfig* config = NULL);
	virtual const OdeSolverConfig* getConfig();
	virtual void loadConfig(const OdeSolverConfig* config);

	static std::string getDisplayName(){
		return "PETSc solver for E1 config widget";
	}
private:
	void widget_to_config();
	void config_to_widget();
};

/////////////////////////////////////////////////////////////////////

REGISTER_INSTANCE_WIDGET_CLASSES(E1ConfigWidget, E1StateGeneratorWidget)
REGISTER_SOLVER_CONFIG_WIDGET_CLASS(E1PetscSolverConfigWidget)

#endif /* GUI_E1_H_ */
