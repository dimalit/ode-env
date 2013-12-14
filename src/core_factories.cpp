/*
 * core_factories.cpp
 *
 *  Created on: Dec 14, 2013
 *      Author: dimalit
 */

#include "core_factories.h"
#include "core_factory_managers.h"

OdeInstanceFactory::OdeInstanceFactory(){
	OdeInstanceFactoryManager::getInstance()->add(this);
}
OdeInstanceFactory::~OdeInstanceFactory(){
	OdeInstanceFactoryManager::getInstance()->remove(this);
}

OdeSolverFactory::OdeSolverFactory(){
	OdeSolverFactoryManager::getInstance()->add(this);
}
OdeSolverFactory::~OdeSolverFactory(){
	OdeSolverFactoryManager::getInstance()->remove(this);
}
