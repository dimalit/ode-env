#ifndef GUI_E42MC_H_
#define GUI_E42MC_H_

#include <common_components.h>

#include <gui_factories.h>
#include <models/e42mc/model_e42mc.h>

#include <cassert>
#include <iostream>

class E42mcConfigWidget: public EXConfigWidget{
public:
	typedef E42mcConfig Config;
	E42mcConfigWidget(const Config* cfg):EXConfigWidget(cfg?cfg:new E42mcConfig()){}
};

class E42mcPetscSolverConfigWidget: public EXPetscSolverConfigWidget{
public:
	typedef E42mcPetscSolver Solver;
	E42mcPetscSolverConfigWidget(const EXPetscSolverConfig* config = NULL):
		EXPetscSolverConfigWidget(config){}
};

class E42mcStateGeneratorWidget: public OdeStateGeneratorWidget{
public:
	typedef E42mcState State;
private:
	E42mcState* state;
	E42mcConfig* config;

	AbstractConfigWidget cfg_widget;

public:
	E42mcStateGeneratorWidget(const E42mcConfig* config);
	virtual ~E42mcStateGeneratorWidget();
	virtual const OdeState* getState();
	virtual void loadConfig(const OdeConfig* config);
	virtual const OdeConfig* getConfig();
	virtual void newState(bool emit=true);

private:
	void on_changed();
	void center_masses();
};

/////////////////////////////////////////////////////////////////////
REGISTER_INSTANCE_WIDGET_CLASSES(E42mcConfigWidget, E42mcStateGeneratorWidget)
REGISTER_SOLVER_CONFIG_WIDGET_CLASS(E42mcPetscSolverConfigWidget)

#endif /* GUI_E42MC_H_ */
