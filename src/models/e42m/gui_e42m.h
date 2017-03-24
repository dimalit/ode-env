/*
 * gui_e3.h
 *
 *  Created on: Aug 21, 2014
 *      Author: dimalit
 */

#ifndef GUI_E42M_H_
#define GUI_E42M_H_

#include "model_e42m.h"

#include <common_components.h>

#include <gui_factories.h>

#include <cassert>
#include <iostream>

class E42mConfigWidget: public EXConfigWidget{
public:
	typedef E42mConfig Config;
	E42mConfigWidget(const Config* cfg):EXConfigWidget(cfg?cfg:new E42mConfig()){}
};

class E42mPetscSolverConfigWidget: public EXPetscSolverConfigWidget{
public:
	typedef E42mPetscSolver Solver;
	E42mPetscSolverConfigWidget(const EXPetscSolverConfig* config = NULL):
		EXPetscSolverConfigWidget(config){}
};

class E42mStateGeneratorWidget: public OdeStateGeneratorWidget{
public:
	typedef E42mState State;
private:
	E42mState* state;
	E42mConfig* config;

	AbstractConfigWidget cfg_widget;

public:
	E42mStateGeneratorWidget(const E42mConfig* config);
	virtual ~E42mStateGeneratorWidget();
	virtual const OdeState* getState();
	virtual void loadConfig(const OdeConfig* config);
	virtual const OdeConfig* getConfig();
	virtual void newState(bool emit=true);

private:
	void on_changed();
	void center_masses();
};

/////////////////////////////////////////////////////////////////////
REGISTER_INSTANCE_WIDGET_CLASSES(E42mConfigWidget, E42mStateGeneratorWidget)
REGISTER_SOLVER_CONFIG_WIDGET_CLASS(E42mPetscSolverConfigWidget)

#endif /* GUI_E42M_H_ */
