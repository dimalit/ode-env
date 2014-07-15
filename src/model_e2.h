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
	}
	static std::string getDisplayName(){
		return "model e2";
	}
};

class E2State: public pb::E2State, public OdeState{
public:
	E2State(const E2Config*);
};
class E2PetscSolverConfig: public pb::E2PetscSolverConfig, public OdeSolverConfig{};

class E2PetscSolver: public OdeSolver{
public:
	E2PetscSolver(const E2PetscSolverConfig*, const E2Config*, const E2State*);
	virtual ~E2PetscSolver();
	virtual const OdeState* getCurrentState() const;
	virtual double getTime() const;
	virtual void run();

	static std::string getDisplayName(){
		return "PETSc RK solver through protobuf for e1";
	}
private:
	E2Config* pconfig;				// problem config
	E2PetscSolverConfig* sconfig;	// solver config
	E2State* state;
};

/////////////////// factories //////////////////

REGISTER_INSTANCE_FACTORY(E2InstanceFactory, E2Config, E2State)
REGISTER_SOLVER_FACTORY(E2SolverFactory, E2InstanceFactory, E2PetscSolver, E2PetscSolverConfig, E2Config, E2State)

#endif /* MODEL_E2_H_ */
