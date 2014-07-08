/*
 * core_factories.h
 *
 *  Created on: Dec 14, 2013
 *      Author: dimalit
 */

#ifndef CORE_FACTORIES_H_
#define CORE_FACTORIES_H_

#include <string>

class OdeConfig;
class OdeState;
class OdeInstance;
class OdeSolverConfig;
class OdeSolver;

class OdeInstanceFactory{
public:
	OdeInstance* createInstance() const;
	virtual OdeConfig* createConfig() const = 0;
	virtual OdeState* createState(const OdeConfig*) const = 0;

	virtual std::string getDisplayName() const = 0;
protected:
	OdeInstanceFactory();
	virtual ~OdeInstanceFactory();
};

class OdeSolverFactory{
public:
	virtual OdeSolver* createSolver(const OdeSolverConfig* scfg, const OdeConfig* ocfg, const OdeState* initial_state) const = 0;
	virtual OdeSolverConfig* createSolverConfg() const = 0;

	virtual std::string getDisplayName() const = 0;
	OdeInstanceFactory* getBaseFactory() const {
		return corresponding_instance_factory;
	}

protected:
	OdeSolverFactory(OdeInstanceFactory* corresponding_instance_factory);
	virtual ~OdeSolverFactory();
	OdeInstanceFactory* corresponding_instance_factory;
};

#endif /* CORE_FACTORIES_H_ */
