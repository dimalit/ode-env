/*
 * interfaces.h
 *
 *  Created on: Dec 14, 2013
 *      Author: dimalit
 */

#ifndef CORE_INTERFACES_H_
#define CORE_INTERFACES_H_

class OdeConfig{
public:
	virtual ~OdeConfig(){}	// make it polymorphic
};

class OdeState{
public:
	virtual ~OdeState(){}	// make it polymorphic
};

class OdeInstance{
private:
	const OdeConfig* config;
	const OdeState* initial_state;
public:
	OdeInstance(const OdeConfig* cfg, const OdeState* state){
		this->config = cfg;
		this->initial_state = state;
	}
	const OdeState* getInitialState() const {return initial_state;}
	const OdeConfig* getConfig() const {return config;}
	void setConfig(const OdeConfig* cfg) {this->config = cfg;}
	void setInitialState(const OdeState* state) {this->initial_state = state;}
};

class OdeSolverConfig{
public:
	virtual ~OdeSolverConfig(){}	// make it polymorphic
};

// TODO: Virtual destructor here and elsewhere!
class OdeSolver{
public:
	virtual const OdeState* run(int steps, double time) = 0;
	virtual ~OdeSolver(){}
	virtual double getTime() const = 0;
	virtual double getSteps() const = 0;
};

#endif /* CORE_INTERFACES_H_ */
