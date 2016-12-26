/*
 * gui_e3.h
 *
 *  Created on: Aug 21, 2014
 *      Author: dimalit
 */

#ifndef GUI_E4_H_
#define GUI_E4_H_

#include "model_e4.h"

#include <gui_factories.h>

#include <google/protobuf/message.h>

#include "gtkmm/grid.h"
#include "gtkmm/entry.h"
#include "gtkmm/comboboxtext.h"
#include "gtkmm/adjustment.h"

#include <cassert>
#include <iostream>

using namespace google::protobuf;

// monitors EXTERNAL Message - with no ownership!
class AbstractConfigWidget: public Gtk::Bin{
private:
	mutable Message* data;

	Gtk::Grid grid;
	mutable Gtk::Button button_apply;

	std::map<string, Gtk::Entry*> entry_map;

	sigc::signal<void> m_signal_changed;
public:
	AbstractConfigWidget(Message *msg = NULL);

	void setData(Message* msg);
	const Message* getData() const;
	void update(){
		config_to_widget();
	}

	// only fires when changed from GUI
	sigc::signal<void> signal_changed() const{
		return m_signal_changed;
	}

	virtual ~AbstractConfigWidget(){
		delete data;
	}

private:
	void construct_ui();
	void widget_to_config() const;
	void config_to_widget() const;
	void edit_anything_cb();
	void on_apply_cb();
};

template<class C>
class EXConfigWidget: public OdeConfigWidget{
//!!! TODO: remove this - in gui_interfaces
public:
	typedef C Config;
private:
	OdeConfig* config;

	AbstractConfigWidget cfg_widget;

public:
	EXConfigWidget(const OdeConfig* config = NULL);

	virtual const OdeConfig* getConfig() const;
	virtual void loadConfig(const OdeConfig* cfg);

	static std::string getDisplayName(){
		return "GTK+ widgets for EX";
	}

private:
	void on_changed();
};

class E4StateGeneratorWidget: public OdeStateGeneratorWidget{
public:
	typedef E4State State;
private:
	E4State* state;
	E4Config* config;

	AbstractConfigWidget cfg_widget;

public:
	E4StateGeneratorWidget(const E4Config* config);
	virtual ~E4StateGeneratorWidget();
	virtual const OdeState* getState();
	virtual void loadConfig(const OdeConfig* config);
	virtual const OdeConfig* getConfig();
	virtual void newState(bool emit=true);

private:
	void on_changed();
};

class EXPetscSolverConfigWidget: public OdeSolverConfigWidget{
public:
	typedef E4PetscSolver Solver;
private:
	EXPetscSolverConfig* config;

	Gtk::Entry *entry_atol, *entry_rtol, *entry_step;
	Glib::RefPtr<Gtk::Adjustment> adj_n_cores;

public:
	EXPetscSolverConfigWidget(const EXPetscSolverConfig* config = NULL);
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
typedef EXConfigWidget<E4Config> E4ConfigWidget;
REGISTER_INSTANCE_WIDGET_CLASSES(E4ConfigWidget, E4StateGeneratorWidget)
REGISTER_SOLVER_CONFIG_WIDGET_CLASS(EXPetscSolverConfigWidget)

#endif /* GUI_E4_H_ */
