/*
 * model_e2.h
 *
 *  Created on: Jul 10, 2014
 *      Author: dimalit
 */

#ifndef MODEL_E2_H_
#define MODEL_E2_H_

#include "core_interfaces.h"
#include "core_factories.h"
#include "../from_protoc/model_e2.pb.h"

class E2Config: public pb::E2Config, public OdeConfig{
public:
	E2Config(){
		set_n(250);
		set_f(1);
	}
	virtual OdeConfig* clone() const {
		E2Config* ret = new E2Config();
		ret->MergeFrom(*this);
		return ret;
	}
	static std::string getDisplayName(){
		return "model e2";
	}
};

class E2State: public pb::E2State, public OdeState{
public:
	E2State(){
		set_simulated(false);
	}
	E2State(const E2Config*);
	virtual OdeState* clone() const {
		return new E2State(*this);
	}
};

class E2PetscSolverConfig: public pb::E2PetscSolverConfig, public OdeSolverConfig{
public:
	E2PetscSolverConfig(){
		this->set_tolerance(0.001);
		this->set_init_step(0.1);
	}
};

class E2PetscSolver: public OdeSolver{
public:
	typedef E2PetscSolverConfig SConfig;
	typedef E2Config PConfig;
	typedef E2State State;
public:
	E2PetscSolver(const E2PetscSolverConfig*, const E2Config*, const E2State*);
	virtual ~E2PetscSolver();
	virtual void run(int steps, double time, bool use_step = false);
	virtual bool step(){assert(false);return false;}
	virtual void finish(){assert(false);}
	virtual double getTime() const;
	virtual double getSteps() const;
	virtual const OdeState* getState() const {
		return state;
	}
	virtual const OdeState* getDState() const{
		return d_state;
	}

	static std::string getDisplayName(){
		return "PETSc RK solver through protobuf for e1";
	}
private:
	E2Config* pconfig;				// problem config
	E2PetscSolverConfig* sconfig;	// solver config
	E2State* state;
	E2State* d_state;
	double time_passed;
	int steps_passed;
};

/////////////////// factories //////////////////

REGISTER_INSTANCE_CLASS(E2Config, E2State)
REGISTER_SOLVER_CLASS(E2PetscSolver)

#endif /* MODEL_E2_H_ */
