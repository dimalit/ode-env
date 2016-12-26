/*
 * model_e1.h
 *
 *  Created on: Jan 4, 2014
 *      Author: dimalit
 */

#ifndef MODEL_E1_H_
#define MODEL_E1_H_

#include <core_interfaces.h>
#include <core_factories.h>

#include <cstdlib>
#include <vector>
#include <string>
#include <cassert>

class E1Config: public OdeConfig{
public:
	int  g_m;			// number of particles
	double ksi;		// particle phase if all the same
	bool random_ksi, linear_ksi;

	E1Config(){
		g_m = 250;
		ksi = 0.2499;
		random_ksi = true;
		linear_ksi = false;
	}
	virtual OdeConfig* clone() const {
		E1Config* ret = new E1Config();
		*ret = *this;
		return ret;
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
	E1State(){
		random_ksi = false;
		linear_ksi = true;

		E = 0.0;
		b = 0.0;
		phi = 0.0;
		ksi = 0.0;
	}
	E1State(const E1Config* cfg):ksi_array(cfg->g_m), b_array(cfg->g_m) {
		random_ksi = cfg->random_ksi;
		linear_ksi = cfg->linear_ksi;

		E = 1.0;
		b = 0.5;
		phi = 0.0;
		ksi = 0.2499;

		generate_arrays();
	}

	virtual OdeState* clone() const {
		assert(false && "Not implemented");
		return NULL;
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
	double g_tol;
	double g_step;
};

class E1PetscSolver: public OdeSolver{
public:
	typedef E1PetscSolverConfig SConfig;
	typedef E1Config PConfig;
	typedef E1State State;

public:
	E1PetscSolver(const E1PetscSolverConfig*, const E1Config*, const E1State*);
	virtual ~E1PetscSolver();
	virtual void run(int steps, double time, bool use_step = false);
	virtual bool step(){assert(false);return false;}
	virtual void finish(){assert(false);}
	virtual double getTime() const;
	virtual double getSteps() const;
	virtual const OdeState* getState() const {
		return state;
	}
	virtual const OdeState* getDState() const{
		return d_state;
	}

	static std::string getDisplayName(){
		return "PETSc RK solver for e1";
	}

private:
	E1Config* pconfig;				// problem config
	E1PetscSolverConfig* sconfig;	// solver config
	E1State* state;
	E1State* d_state;

	double *b;
	double *ksi;

	double time_passed;
	int steps_passed;

private:
	int read_simulation(FILE* fp, int m);
};

/////////////////// factories //////////////////

REGISTER_INSTANCE_CLASS(E1Config, E1State)
REGISTER_SOLVER_CLASS(E1PetscSolver)

#endif /* MODEL_E1_H_ */
