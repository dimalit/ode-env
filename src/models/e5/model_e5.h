#ifndef MODEL_E5_H_
#define MODEL_E5_H_

#include <common_components.h>

#include <core_interfaces.h>
#include <core_factories.h>

#include <../from_protoc/model_e5.pb.h>

class E5Config: public pb::E5Config, public OdeConfig{
public:
	E5Config();
	virtual OdeConfig* clone() const {
		E5Config* ret = new E5Config();
		ret->MergeFrom(*this);
		return ret;
	}
	static std::string getDisplayName(){
		return "model e5";
	}
};

class E5State: public pb::E5State, public OdeState{
public:
	E5State();
	E5State(const E5Config*);
	virtual OdeState* clone() const {
		return new E5State(*this);
	}
};

class E5PetscSolver: public EXPetscSolver<pb::E5Model, pb::E5Solution>{
public:
	typedef EXPetscSolverConfig SConfig;
	typedef E5Config PConfig;
	typedef E5State State;

	virtual const char* ts_path(){
		return " ../ts5/Debug/ts5";
	}

	E5PetscSolver(const SConfig* sc, const PConfig* pc, const State* s):
		EXPetscSolver(sc, pc, s){}
};

REGISTER_INSTANCE_CLASS(E5Config, E5State)
REGISTER_SOLVER_CLASS(E5PetscSolver)

#endif /* MODEL_E5_H_ */
