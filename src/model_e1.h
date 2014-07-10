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
#include "core_factory_managers.h"

#include <petscts.h>

#include <vector>
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

	static std::string getDisplayName(){
		return "model e1";
	}
};

// TODO: Make child from this specific to PETSc-solver
class E1State: public OdeState{
	friend class E1PetscSolver;
	std::vector<double> ksi_array;	// approximate solution vector
	std::vector<double> b_array;	// approximate solution vector
	double E, phi;	// field
	double b;		// particles amplitude
	double ksi;		// particles phase if equal
	bool random_ksi, linear_ksi;

public:
	E1State(const E1Config* cfg):ksi_array(cfg->g_m), b_array(cfg->g_m) {
		random_ksi = cfg->random_ksi;
		linear_ksi = cfg->linear_ksi;

		E = 1.0;
		b = 0.5;
		phi = 0.0;
		ksi = 0.2499;

		generate_arrays();
	}

	void sort_arrays();

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
		generate_arrays();
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

	std::vector<double>& getKsiArray(){
		return ksi_array;
	}

	std::vector<double>& getBArray(){
		return b_array;
	}
private:
	void generate_arrays(){
		int m = ksi_array.size();
		for(int i=0; i<m; i++){
			if(linear_ksi)
				ksi_array[i] = 1.0 / m * i;
			else if(random_ksi)
				ksi_array[i] = rand() / (double)RAND_MAX;
			else
				assert(false);
			b_array[i] = getB();
		}
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
	virtual void run();
private:
	E1Config* pconfig;				// problem config
	E1PetscSolverConfig* sconfig;	// solver config
	E1State* state;

	double *b;
	double *ksi;

	double time_passed;
	int steps_passed;

private:
	int read_simulation(FILE* fp, int m);
};

/////////////////// factories //////////////////
template<class C, class S>
class TemplateInstanceFactory: public OdeInstanceFactory{
public:
	static TemplateInstanceFactory* getInstance(){
		return &instance;
	}

	virtual OdeConfig* createConfig() const {
		return new C();
	}
	virtual OdeState* createState(const OdeConfig* cfg) const {
		const C* ecfg = dynamic_cast<const C*>(cfg);
			assert(ecfg);
		return new S(ecfg);
	}

	virtual std::string getDisplayName() const {
		return C::getDisplayName();
	}

	virtual ~TemplateInstanceFactory(){}
private:
	// TODO: write: when this is created it calls parent ctor which calls add(this) - and tihis fails because getName() isn't here yet (object partially constructed!)
	TemplateInstanceFactory(){
		OdeInstanceFactoryManager::getInstance()->add(this);
	}
	static TemplateInstanceFactory instance;
};

template<class C, class S>
TemplateInstanceFactory<C, S> TemplateInstanceFactory<C, S>::instance;

typedef TemplateInstanceFactory<E1Config, E1State> E1InstanceFactory;

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
