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
private:
	E2Config* config;

	Gtk::Entry *entry_n, *entry_a, *entry_delta, *entry_f;

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
};

class E2StateWidget: public OdeStateWidget{
private:
	E2State* state;
	E2Config* config;

	Gtk::Socket x11_socket;

	Gtk::Entry *entry_left, *entry_right;
	Gtk::CheckButton *radio_rand, *radio_linear;

	FILE* to_gnuplot;
public:
	E2StateWidget(const E2Config* config, const E2State* state = NULL);
	virtual ~E2StateWidget();
	virtual void loadState(const OdeState* state);
	virtual const OdeState* getState();
	virtual void loadConfig(const OdeConfig* config);
	virtual const OdeConfig* getConfig();
	virtual void generateState();

private:
	void widget_to_state();
	void state_to_widget();
	void update_chart();
	void on_realize_cb();
};

class E2PetscSolverConfigWidget: public OdeSolverConfigWidget{
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

REGISTER_INSTANCE_WIDGET_FACTORY(E2InstanceWidgetFactory, E2InstanceFactory, E2ConfigWidget, E2StateWidget, E2Config, E2State)
REGISTER_SOLVER_CONFIG_WIDGET_FACTORY(E2PetscSolverConfigWidgetFactory, E2SolverFactory, E2PetscSolverConfig, E2PetscSolverConfigWidget)

#endif /* GUI_E2_H_ */
