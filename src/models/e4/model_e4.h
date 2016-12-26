/*
 * model_e3.h
 *
 *  Created on: Aug 19, 2014
 *      Author: dimalit
 */

#ifndef MODEL_E4_H_
#define MODEL_E4_H_

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

class EXPetscSolverConfig: public pb::EXPetscSolverConfig, public OdeSolverConfig{
public:
	EXPetscSolverConfig();
};

template<class SC, class PC, class S>
class EXPetscSolver: public OdeSolver{
public:
	typedef SC SConfig;
	typedef PC PConfig;
	typedef S State;

	static const char* ts_path;

public:
	EXPetscSolver(const SConfig*, const PConfig*, const State*);
	virtual ~EXPetscSolver();
	virtual void run(int steps, double time, bool use_steps = false);
	virtual bool step();
	virtual void finish();
	virtual double getTime() const {
		return time_passed;
	}
	virtual double getSteps() const {
		return steps_passed;
	}

	virtual const OdeState* getState() const {
		return state;
	}
	virtual const OdeState* getDState() const{
		return d_state;
	}

	static std::string getDisplayName(){
		return "PETSc RK solver for eX";
	}

private:
	PConfig* pconfig;				// problem config
	SConfig* sconfig;	// solver config
	State* state;
	State* d_state;

	FILE  *rf, *wf;
	pid_t child;

	double time_passed;
	int steps_passed;

private:
//	int read_simulation(FILE* fp, int m);
	bool read_results();
};

REGISTER_INSTANCE_CLASS(E4Config, E4State)
typedef EXPetscSolver<EXPetscSolverConfig,E4Config,E4State> E4PetscSolver;
REGISTER_SOLVER_CLASS(E4PetscSolver)

#endif /* MODEL_E4_H_ */
