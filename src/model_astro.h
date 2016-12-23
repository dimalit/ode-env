/*
 * model_astro.h
 *
 *  Created on: Sep 18, 2016
 *      Author: dimalit
 */

#ifndef MODEL_ASTRO_H_
#define MODEL_ASTRO_H_

#include "core_interfaces.h"
#include "core_factories.h"
#include "../from_protoc/model_astro.pb.h"

#include <memory>

class AstroConfig: public pb::AstroConfig, public OdeConfig{
public:
	AstroConfig();
	virtual OdeConfig* clone() const {
		AstroConfig* ret = new AstroConfig();
		ret->MergeFrom(*this);
		return ret;
	}
	static std::string getDisplayName(){
		return "astro";
	}
};

class AstroState: public pb::AstroState, public OdeState{
public:
	AstroState(const AstroConfig*);
	virtual OdeState* clone() const{
		return new AstroState(*this);
	}

};

class AstroSolverConfig: public pb::AstroSolverConfig, public OdeSolverConfig{
public:
	AstroSolverConfig();
};

class AstroSolver: public OdeSolver{
public:
	typedef AstroSolverConfig SConfig;
	typedef AstroConfig PConfig;
	typedef AstroState State;

public:
	AstroSolver(const AstroSolverConfig*, const AstroConfig*, const AstroState*);
	virtual ~AstroSolver();
	virtual void run(int steps, double time, bool use_steps = false);
	virtual bool step();
	virtual void finish();
	virtual double getTime() const;
	virtual double getSteps() const;
	virtual const OdeState* getState() const {
		return state.get();
	}
	virtual const OdeState* getDState() const{
		return d_state.get();
	}

	static std::string getDisplayName(){
		return "Simple RK4 solver for astro model";
	}

private:
	std::unique_ptr<AstroConfig> pconfig;				// problem config
	std::unique_ptr<AstroSolverConfig> sconfig;			// solver config
	std::unique_ptr<AstroState> state;
	std::unique_ptr<AstroState> d_state;

	int steps;
	double time;
};

#endif /* MODEL_ASTRO_H_ */
