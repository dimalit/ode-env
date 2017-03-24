/*
 * model_e3.h
 *
 *  Created on: Aug 19, 2014
 *      Author: dimalit
 */

#ifndef MODEL_E42M_H_
#define MODEL_E42M_H_

#include <common_components.h>

#include <core_interfaces.h>
#include <core_factories.h>

#include <../from_protoc/model_e42m.pb.h>

class E42mConfig: public pb::E42mConfig, public OdeConfig{
public:
	E42mConfig();
	virtual OdeConfig* clone() const {
		E42mConfig* ret = new E42mConfig();
		ret->MergeFrom(*this);
		return ret;
	}
	static std::string getDisplayName(){
		return "model e42m";
	}
};

class E42mState: public pb::E42mState, public OdeState{
public:
	E42mState();
	E42mState(const E42mConfig*);
	virtual OdeState* clone() const {
		return new E42mState(*this);
	}
};

class E42mPetscSolver: public EXPetscSolver<pb::E42mModel, pb::E42mSolution>{
public:
	typedef EXPetscSolverConfig SConfig;
	typedef E42mConfig PConfig;
	typedef E42mState State;

	virtual const char* ts_path(){
		return " ../ts42m/Debug/ts42m";
	}

	E42mPetscSolver(const SConfig* sc, const PConfig* pc, const State* s):
		EXPetscSolver(sc, pc, s){}
};

REGISTER_INSTANCE_CLASS(E42mConfig, E42mState)
REGISTER_SOLVER_CLASS(E42mPetscSolver)

#endif /* MODEL_E42M_H_ */
