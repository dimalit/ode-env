/*
 * model_e4mm.h
 *
 *  Created on: Jan 2, 2017
 *      Author: dimalit
 */

#ifndef MODELS_E4MM_MODEL_E4MM_H_
#define MODELS_E4MM_MODEL_E4MM_H_

#include <common_components.h>

#include <core_interfaces.h>
#include <core_factories.h>

#include <../from_protoc/model_e4mm.pb.h>

class E4mmConfig: public pb::E4mmConfig, public OdeConfig{
public:
	E4mmConfig();
	virtual OdeConfig* clone() const {
		E4mmConfig* ret = new E4mmConfig();
		ret->MergeFrom(*this);
		return ret;
	}
	static std::string getDisplayName(){
		return "model e4 multi-mode";
	}
};

class E4mmState: public pb::E4mmState, public OdeState{
public:
	E4mmState();
	E4mmState(const E4mmConfig*);
	virtual OdeState* clone() const {
		return new E4mmState(*this);
	}
};

class E4mmPetscSolver: public EXPetscSolver{
public:
	typedef EXPetscSolverConfig SConfig;
	typedef E4mmConfig PConfig;
	typedef E4mmState State;

	E4mmPetscSolver(const SConfig* sc, const PConfig* pc, const State* s):
		EXPetscSolver(sc, pc, s){}
};

REGISTER_INSTANCE_CLASS(E4mmConfig, E4mmState)
REGISTER_SOLVER_CLASS(E4mmPetscSolver)

#endif /* MODELS_E4MM_MODEL_E4MM_H_ */
