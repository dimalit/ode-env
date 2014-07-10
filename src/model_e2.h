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

class E2Config: public pb::E2Config, public OdeConfig{};
class E2State: public pb::E2State, public OdeState{
public:
	E2State(const E2Config*);
};
class E2PetscSolverConfig: public pb::E2PetscSolverConfig, public OdeSolverConfig{};

class E2PetscSolver: public OdeSolver{

public:
	E2PetscSolver(const E2Config*, const E2State*, const E2PetscSolverConfig*);
	virtual ~E2PetscSolver();
	virtual const OdeState* getCurrentState() const;
	virtual double getTime() const;
	virtual void run();
private:
	E2Config* pconfig;				// problem config
	E2PetscSolverConfig* sconfig;	// solver config
	E2State* state;
};

/////////////////// factories //////////////////
class E2InstanceFactory: public OdeInstanceFactory{
public:
	static E2InstanceFactory* getInstance(){
		return &instance;
	}

	virtual OdeConfig* createConfig() const {
		return new E2Config();
	}
	virtual OdeState* createState(const OdeConfig* cfg) const {
		const E2Config* ecfg = dynamic_cast<const E2Config*>(cfg);
			assert(ecfg);
		return new E2State(ecfg);
	}

	virtual std::string getDisplayName() const {
		return "model e2";
	}

	virtual ~E2InstanceFactory(){}
private:
	// TODO: write: when this is created it calls parent ctor which calls add(this) - and tihis fails because getName() isn't here yet (object partially constructed!)
	E2InstanceFactory();
	static E2InstanceFactory instance;
};

class E2SolverFactory: public OdeSolverFactory{
public:
	static E2SolverFactory* getInstance(){
		return &instance;
	}

	virtual OdeSolver* createSolver(const OdeSolverConfig* scfg, const OdeConfig* pcfg, const OdeState* initial_state) const {
		const E2PetscSolverConfig* e1_scfg = dynamic_cast<const E2PetscSolverConfig*>(scfg);
			assert(e1_scfg);
		const E2Config* e1_pcfg = dynamic_cast<const E2Config*>(pcfg);
			assert(e1_pcfg);
		const E2State* e1_state = dynamic_cast<const E2State*>(initial_state);
			assert(e1_state);
		return new E2PetscSolver(e1_pcfg, e1_state, e1_scfg);
	}
	virtual OdeSolverConfig* createSolverConfg() const {
		return new E2PetscSolverConfig();
	}

	virtual std::string getDisplayName() const {
		return "PETSc RK solver through protobuf for e1";
	}
private:
	static E2SolverFactory instance;
	E2SolverFactory():OdeSolverFactory(E2InstanceFactory::getInstance()){
	}
};

#endif /* MODEL_E2_H_ */
