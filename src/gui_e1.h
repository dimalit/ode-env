/*
 * gui_e1.h
 *
 *  Created on: Jul 4, 2014
 *      Author: dimalit
 */

#ifndef GUI_E1_H_
#define GUI_E1_H_

#include "gui_factories.h"

#include "model_e1.h"

#include <gtkmm/entry.h>
#include <gtkmm/checkbutton.h>

#include <goffice/goffice.h>

#include <cassert>
#include <iostream>

class E1ConfigWidget: public OdeConfigWidget{
private:
	E1Config* config;

	Gtk::Entry *entry_m, *entry_ksi;
	Gtk::CheckButton *check_rand, *check_linear;

public:
	E1ConfigWidget(const E1Config* config = NULL);
	// TODO: maybe clone?
	virtual const OdeConfig* getConfig();
	virtual void loadConfig(const OdeConfig* cfg);

private:
	void widget_to_config();
	void config_to_widget();
	void check_linear_toggled_cb();
	void check_rand_toggled_cb();
};

class E1StateWidget: public OdeStateWidget{
private:
	E1State* state;
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
	double series_b_xs[2];
	double series_b_ys[2];

	GOData *series_phi_data_x;
	GOData *series_phi_data_y;
	double series_phi_xs[21];
	double series_phi_ys[21];

public:
	E1StateWidget(const E1Config* config, const E1State* state = NULL);
	virtual void loadState(const OdeState* state);
	virtual const OdeState* getState();
	virtual void loadConfig(const OdeConfig* config);
	virtual const OdeConfig* getConfig();

private:
	void widget_to_state();
	void state_to_widget();
	void draw(GogChart* chart);
	void adj_E_value_changed_cb();
	void adj_b_value_changed_cb();
	void adj_phi_value_changed_cb();
};

class E1PetscSolverConfigWidget: public OdeSolverConfigWidget{
private:
	E1PetscSolverConfig* config;

	Gtk::Entry *entry_tol, *entry_step;
public:
	E1PetscSolverConfigWidget(const E1PetscSolverConfig* config = NULL);
	virtual const OdeSolverConfig* getConfig();
	virtual void loadConfig(const OdeSolverConfig* config);
private:
	void widget_to_config();
	void config_to_widget();
};

/////////////////////////////////////////////////////////////////////

class E1InstanceWidgetFactory: public OdeInstanceWidgetFactory{
public:
	static E1InstanceWidgetFactory* getInstance(){
		return &instance;
	}

	virtual OdeConfigWidget* createConfigWidget(const OdeConfig* = NULL) const;
	virtual OdeStateWidget* createStateWidget(const OdeConfig* cfg, const OdeState* state = NULL) const;

	virtual std::string getDisplayName() const;

private:
	static E1InstanceWidgetFactory instance;
	E1InstanceWidgetFactory()
		:OdeInstanceWidgetFactory(E1InstanceFactory::getInstance()){}
};

class E1PetscSolverConfigWidgetFactory: public OdeSolverConfigWidgetFactory{
public:
	static E1PetscSolverConfigWidgetFactory* getInstance(){
		return &instance;
	}

	virtual OdeSolverConfigWidget* createConfigWidget(const OdeSolverConfig* = NULL) const;

	virtual std::string getDisplayName() const;

private:
	static E1PetscSolverConfigWidgetFactory instance;
	E1PetscSolverConfigWidgetFactory()
		:OdeSolverConfigWidgetFactory(E1SolverFactory::getInstance())
	{
		std::cout << "added " << this << std::endl;
	}
	virtual ~E1PetscSolverConfigWidgetFactory()
	{
		std::cout << "will remove " << this << std::endl;
	}
};

#endif /* GUI_E1_H_ */
