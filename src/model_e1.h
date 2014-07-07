/*
 * model_e1.h
 *
 *  Created on: Jan 4, 2014
 *      Author: dimalit
 */

#ifndef MODEL_E1_H_
#define MODEL_E1_H_

#include "core_interfaces.h"
#include "core_factories.h"

#include <petscts.h>

#include <string>
#include <cassert>

class E1Config: public OdeConfig{
public:
	PetscInt  g_m;			// number of particles
	PetscScalar ksi;		// particle phase if all the same
	bool random_ksi, linear_ksi;

	E1Config(){
		g_m = 250;
		ksi = 0.2499;
		random_ksi = true;
		linear_ksi = false;
	}
};

// TODO: Make child from this specific to PETSc-solver
class E1State: public OdeState{
	friend class E1PetscSolver;
	Vec	u;			// approximate solution vector
	double E, phi;	// field
	double b;		// particles amplitude
	double ksi;		// particles phase if equal
	bool random_ksi, linear_ksi;

public:
	E1State(const E1Config* cfg) {
		VecCreateMPI(PETSC_COMM_WORLD, PETSC_DECIDE, 2*cfg->g_m + 2, &u);
		random_ksi = cfg->random_ksi;
		linear_ksi = cfg->linear_ksi;

		E = 1.0;
		b = 0.5;
		phi = 0.0;
		ksi = 0.2499;
	}

	double getE() const {
		return E;
	}
	double getPhi() const {
		return phi;
	}
	double getB() const {
		return b;
	}

	void setE(double E){
		this->E = E;
	}
	void setPhi(double phi){
		this->phi = phi;
	}
	void setB(double b){
		this->b = b;
	}

	bool getRandomKsi() const {
		return random_ksi;
	}
	bool getLinearKsi() const {
		return linear_ksi;
	}
	double getKsi() const {
		return ksi;
	}

private:
	PetscScalar get_elem(PetscInt i) const {
		PetscInt       mybase,myend;
		PetscScalar    *u_localptr;
		PetscErrorCode ierr;

		ierr = VecGetOwnershipRange(u,&mybase,&myend);assert(!ierr);
		assert(mybase==0);			// use this only on rank 0
		assert(i<myend);

		ierr = VecGetArray(u,&u_localptr);assert(!ierr);
		PetscScalar ret = u_localptr[i];
		ierr = VecRestoreArray(u,&u_localptr);assert(!ierr);

		return ret;
	}
};

class E1PetscSolverConfig: public OdeSolverConfig{
public:
	PetscReal g_tol;
	PetscReal g_step;
};

class E1PetscSolver: public OdeSolver{

friend PetscErrorCode RHSFunction(TS ts,PetscReal t,Vec global_in,Vec global_out,void *ctx);
friend PetscErrorCode PostStepFunction(TS ts);

public:
	E1PetscSolver(const E1PetscSolverConfig*, const E1Config*, const E1State*);
	virtual ~E1PetscSolver();
	virtual const OdeState* getCurrentState() const;
	virtual double getTime() const;
	virtual void step();
private:
	E1Config* pconfig;				// problem config
	E1PetscSolverConfig* sconfig;	// solver config

	TS             ts;			// solver
	Vec            u;			// solution

	PetscScalar max_integral, min_integral;
};

/////////////////// factories //////////////////
class E1InstanceFactory: public OdeInstanceFactory{
public:
	static E1InstanceFactory* getInstance(){
		return &instance;
	}

	virtual OdeConfig* createConfig() const {
		return new E1Config();
	}
	virtual OdeState* createState(const OdeConfig* cfg) const {
		const E1Config* ecfg = dynamic_cast<const E1Config*>(cfg);
			assert(ecfg);
		return new E1State(ecfg);
	}

	virtual std::string getDisplayName() const {
		return "model e1";
	}

	virtual ~E1InstanceFactory(){}
private:
	static E1InstanceFactory instance;
};

class E1SolverFactory: public OdeSolverFactory{
public:
	static E1SolverFactory* getInstance(){
		return &instance;
	}

	virtual OdeSolver* createSolver(const OdeSolverConfig* scfg, const OdeConfig* pcfg, const OdeState* initial_state) const {
		const E1PetscSolverConfig* e1_scfg = dynamic_cast<const E1PetscSolverConfig*>(scfg);
			assert(e1_scfg);
		const E1Config* e1_pcfg = dynamic_cast<const E1Config*>(pcfg);
			assert(e1_pcfg);
		const E1State* e1_state = dynamic_cast<const E1State*>(initial_state);
			assert(e1_state);
		return new E1PetscSolver(e1_scfg, e1_pcfg, e1_state);
	}
	virtual OdeSolverConfig* createSolverConfg() const {
		return new E1PetscSolverConfig();
	}

	virtual std::string getDisplayName() const {
		return "PETSc RK solver for e1";
	}
private:
	static E1SolverFactory instance;
	E1SolverFactory():OdeSolverFactory(E1InstanceFactory::getInstance()){
	}
};

#endif /* MODEL_E1_H_ */
