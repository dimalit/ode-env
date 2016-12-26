/*
 * model_e3.h
 *
 *  Created on: Aug 19, 2014
 *      Author: dimalit
 */

#ifndef MODEL_E4_H_
#define MODEL_E4_H_

#include <common_components.h>

#include <core_interfaces.h>
#include <core_factories.h>

#include <../from_protoc/model_e4.pb.h>

class E4Config: public pb::E4Config, public OdeConfig{
public:
	E4Config();
	virtual OdeConfig* clone() const {
		E4Config* ret = new E4Config();
		ret->MergeFrom(*this);
		return ret;
	}
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

class E4PetscSolver: public EXPetscSolver{
public:
	typedef EXPetscSolverConfig SConfig;
	typedef E4Config PConfig;
	typedef E4State State;

	E4PetscSolver(const SConfig* sc, const PConfig* pc, const State* s):
		EXPetscSolver(sc, pc, s){}
};

REGISTER_INSTANCE_CLASS(E4Config, E4State)
REGISTER_SOLVER_CLASS(E4PetscSolver)

#endif /* MODEL_E4_H_ */
