#ifndef GUI_E5_H_
#define GUI_E5_H_

#include <common_components.h>

#include <gui_factories.h>
#include <models/e5/model_e5.h>

#include <cassert>
#include <iostream>

class E5ConfigWidget: public EXConfigWidget{
public:
	typedef E5Config Config;
	E5ConfigWidget(const Config* cfg):EXConfigWidget(cfg?cfg:new E5Config()){}
};

class E5PetscSolverConfigWidget: public EXPetscSolverConfigWidget{
public:
	typedef E5PetscSolver Solver;
	E5PetscSolverConfigWidget(const EXPetscSolverConfig* config = NULL):
		EXPetscSolverConfigWidget(config){}
};

class E5StateGeneratorWidget: public OdeStateGeneratorWidget{
public:
	typedef E5State State;
private:
	E5State* state;
	E5Config* config;

	AbstractConfigWidget cfg_widget;

public:
	E5StateGeneratorWidget(const E5Config* config);
	virtual ~E5StateGeneratorWidget();
	virtual const OdeState* getState();
	virtual void loadConfig(const OdeConfig* config);
	virtual const OdeConfig* getConfig();
	virtual void newState(bool emit=true);

private:
	void on_changed();
	void center_masses();
};

/////////////////////////////////////////////////////////////////////
REGISTER_INSTANCE_WIDGET_CLASSES(E5ConfigWidget, E5StateGeneratorWidget)
REGISTER_SOLVER_CONFIG_WIDGET_CLASS(E5PetscSolverConfigWidget)

#endif /* GUI_E5_H_ */
