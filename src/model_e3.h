/*
 * model_e3.h
 *
 *  Created on: Aug 19, 2014
 *      Author: dimalit
 */

#ifndef MODEL_E3_H_
#define MODEL_E3_H_

#include "core_interfaces.h"
#include "core_factories.h"
#include "../from_protoc/model_e3.pb.h"

class E3Config: public pb::E3Config, public OdeConfig{
public:
	E3Config(){
		set_m(100);
		set_n(1.0);
		set_theta_e(0);
		set_gamma_0_2(1);
		set_delta_e(0);
		set_r_e(1.0);
	}
	static std::string getDisplayName(){
		return "model e3";
	}
};

class E3State: public pb::E3State, public OdeState{
public:
	E3State(const E3Config*);
};

class E3PetscSolverConfig: public pb::E3PetscSolverConfig, public OdeSolverConfig{
public:
	E3PetscSolverConfig(){
		set_tolerance(0.001);
		set_init_step(0.1);
	}
};

class E3PetscSolver: public OdeSolver{
public:
	typedef E3PetscSolverConfig SConfig;
	typedef E3Config PConfig;
	typedef E3State State;

public:
	E3PetscSolver(const E3PetscSolverConfig*, const E3Config*, const E3State*);
	virtual ~E3PetscSolver();
	virtual const OdeState* run(int steps, double time);
	virtual double getTime() const;
	virtual double getSteps() const;

	static std::string getDisplayName(){
		return "PETSc RK solver for e3";
	}

private:
	E3Config* pconfig;				// problem config
	E3PetscSolverConfig* sconfig;	// solver config
	E3State* state;

	double time_passed;
	int steps_passed;

private:
	int read_simulation(FILE* fp, int m);
};

REGISTER_INSTANCE_CLASS(E3Config, E3State)
REGISTER_SOLVER_CLASS(E3PetscSolver)

#endif /* MODEL_E3_H_ */
