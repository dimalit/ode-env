#ifndef MODEL_E42MC_H_
#define MODEL_E42MC_H_

#include <common_components.h>

#include <core_interfaces.h>
#include <core_factories.h>

#include <../from_protoc/model_e42mc.pb.h>

class E42mcConfig: public pb::E42mcConfig, public OdeConfig{
public:
	E42mcConfig();
	virtual OdeConfig* clone() const {
		E42mcConfig* ret = new E42mcConfig();
		ret->MergeFrom(*this);
		return ret;
	}
	static std::string getDisplayName(){
		return "model e42mc";
	}
};

class E42mcState: public pb::E42mcState, public OdeState{
public:
	E42mcState();
	E42mcState(const E42mcConfig*);
	virtual OdeState* clone() const {
		return new E42mcState(*this);
	}
};

class E42mcPetscSolver: public EXPetscSolver<pb::E42mcModel, pb::E42mcSolution>{
public:
	typedef EXPetscSolverConfig SConfig;
	typedef E42mcConfig PConfig;
	typedef E42mcState State;

	virtual const char* ts_path(){
		return " ../ts42mc/Debug/ts42mc";
	}

	E42mcPetscSolver(const SConfig* sc, const PConfig* pc, const State* s):
		EXPetscSolver(sc, pc, s){}
};

REGISTER_INSTANCE_CLASS(E42mcConfig, E42mcState)
REGISTER_SOLVER_CLASS(E42mcPetscSolver)

#endif /* MODEL_E42MC_H_ */
