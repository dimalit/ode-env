/*
 * gui_e4.h
 *
 *  Created on: Jan 2, 2017
 *      Author: dimalit
 */

#ifndef GUI_E4MM_H_
#define GUI_E4MM_H_

#include "model_e4mm.h"

#include <common_components.h>

#include <gui_factories.h>

#include <cassert>
#include <iostream>

class E4mmConfigWidget: public EXConfigWidget{
public:
	typedef E4mmConfig Config;
	E4mmConfigWidget(const Config* cfg):EXConfigWidget(cfg?cfg:new E4mmConfig()){}
};

class E4mmPetscSolverConfigWidget: public EXPetscSolverConfigWidget{
public:
	typedef E4mmPetscSolver Solver;
	E4mmPetscSolverConfigWidget(const EXPetscSolverConfig* config = NULL):
		EXPetscSolverConfigWidget(config){}
};

class E4mmStateGeneratorWidget: public OdeStateGeneratorWidget{
public:
	typedef E4mmState State;
private:
	E4mmState* state;
	E4mmConfig* config;

	AbstractConfigWidget cfg_widget;

public:
	E4mmStateGeneratorWidget(const E4mmConfig* config);
	virtual ~E4mmStateGeneratorWidget();
	virtual const OdeState* getState();
	virtual void loadConfig(const OdeConfig* config);
	virtual const OdeConfig* getConfig();
	virtual void newState(bool emit=true);

private:
	void on_changed();
};

/////////////////////////////////////////////////////////////////////
REGISTER_INSTANCE_WIDGET_CLASSES(E4mmConfigWidget, E4mmStateGeneratorWidget)
REGISTER_SOLVER_CONFIG_WIDGET_CLASS(E4mmPetscSolverConfigWidget)

#endif /* GUI_E4MM_H_ */
