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

class OdeConfigFactory{

};

class OdeStateFactory{

};

class OdeInstanceFactory{
public:
	virtual OdeInstance* create() const = 0;
	virtual std::string getDisplayName() const = 0;
private:
	OdeInstanceFactory();
	virtual ~OdeInstanceFactory();
};

class OdeSolverConfigFactory{

};

class OdeSolverFactory{
public:
	virtual OdeSolver* create() const = 0;
	virtual std::string getDisplayName() const = 0;
	virtual OdeInstanceFactory* getCorrespondingInstanceFactory() const = 0;
private:
	OdeSolverFactory();
	virtual ~OdeSolverFactory();
};

#endif /* CORE_FACTORIES_H_ */
