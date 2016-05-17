/*
 * model_e3.h
 *
 *  Created on: Aug 19, 2014
 *      Author: dimalit
 */

#ifndef MODEL_E4_H_
#define MODEL_E4_H_

#include "core_interfaces.h"
#include "core_factories.h"
#include "../from_protoc/model_e4.pb.h"

class E4Config: public pb::E4Config, public OdeConfig{
public:
	E4Config();
	static std::string getDisplayName(){
		return "model e4";
	}
};

class E4State: public pb::E4State, public OdeState{
public:
	E4State();
	E4State(const E4Config*);
	virtual OdeState* clone() const {
		return new E4State(*this);
	}
};

class E4PetscSolverConfig: public pb::E4PetscSolverConfig, public OdeSolverConfig{
public:
	E4PetscSolverConfig();
};

class E4PetscSolver: public OdeSolver{
public:
	typedef E4PetscSolverConfig SConfig;
	typedef E4Config PConfig;
	typedef E4State State;

public:
	E4PetscSolver(const E4PetscSolverConfig*, const E4Config*, const E4State*);
	virtual ~E4PetscSolver();
	virtual void run(int steps, double time, bool use_steps = false);
	virtual bool step();
	virtual void finish();
	virtual double getTime() const;
	virtual double getSteps() const;
	virtual const OdeState* getState() const {
		return state;
	}
	virtual const OdeState* getDState() const{
		return d_state;
	}

	static std::string getDisplayName(){
		return "PETSc RK solver for e3";
	}

private:
	E4Config* pconfig;				// problem config
	E4PetscSolverConfig* sconfig;	// solver config
	E4State* state;
	E4State* d_state;

	FILE  *rf, *wf;
	pid_t child;

	double time_passed;
	int steps_passed;

private:
//	int read_simulation(FILE* fp, int m);
	bool read_results();
};

REGISTER_INSTANCE_CLASS(E4Config, E4State)
REGISTER_SOLVER_CLASS(E4PetscSolver)

#endif /* MODEL_E4_H_ */
