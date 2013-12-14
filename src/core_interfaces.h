/*
 * interfaces.h
 *
 *  Created on: Dec 14, 2013
 *      Author: dimalit
 */

#ifndef CORE_INTERFACES_H_
#define CORE_INTERFACES_H_

class OdeConfig{

};

class OdeState{

};

class OdeInstance{
public:
	OdeState* getInitialState() const;
	OdeConfig* getConfig() const;
};

class OdeSolverConfig{

};

class OdeSolver{
public:
	OdeSolver(OdeInstance*);
};

#endif /* CORE_INTERFACES_H_ */
