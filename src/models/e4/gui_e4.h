/*
 * gui_e3.h
 *
 *  Created on: Aug 21, 2014
 *      Author: dimalit
 */

#ifndef GUI_E4_H_
#define GUI_E4_H_

#include "model_e4.h"

#include <common_components.h>

#include <gui_factories.h>

#include <cassert>
#include <iostream>

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

/////////////////////////////////////////////////////////////////////
typedef EXConfigWidget<E4Config> E4ConfigWidget;
REGISTER_INSTANCE_WIDGET_CLASSES(E4ConfigWidget, E4StateGeneratorWidget)
typedef EXPetscSolverConfigWidget<E4PetscSolver> E4PetscSolverConfigWidget;
REGISTER_SOLVER_CONFIG_WIDGET_CLASS(E4PetscSolverConfigWidget)

#endif /* GUI_E4_H_ */
