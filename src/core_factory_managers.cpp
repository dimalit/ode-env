/*
 * core_factory_managers.cpp
 *
 *  Created on: Dec 14, 2013
 *      Author: dimalit
 */

#include "core_factory_managers.h"

#include <cassert>

OdeInstanceFactoryManager OdeInstanceFactoryManager::instance;
OdeSolverFactoryManager OdeSolverFactoryManager::instance;

void OdeInstanceFactoryManager::add(OdeInstanceFactory* f){
	assert(instance_factories.find(f)==instance_factories.end());
	instance_factories.insert(f);
}

void OdeInstanceFactoryManager::remove(OdeInstanceFactory* f){
	assert(instance_factories.find(f)!=instance_factories.end());

	// remove related solvers
	auto range_to_delete = solvers_map.equal_range(f);
	solvers_map.erase(range_to_delete.first, range_to_delete.second);

	// remove itself
	instance_factories.erase(f);
}

std::pair<OdeInstanceFactoryManager::SupportedSolversIterator, OdeInstanceFactoryManager::SupportedSolversIterator>
OdeInstanceFactoryManager::getSupportedSolvers(OdeInstanceFactory* f){
	auto range_to_return = solvers_map.equal_range(f);
	SupportedSolversIterator begin( range_to_return.first );
	SupportedSolversIterator end( range_to_return.second );
	return std::make_pair(begin, end);
}

void OdeSolverFactoryManager::add(OdeSolverFactory* f){
	assert(solver_factories.find(f)==solver_factories.end());
	solver_factories.insert(f);
}

void OdeSolverFactoryManager::remove(OdeSolverFactory* f){
	assert(solver_factories.find(f)!=solver_factories.end());
	solver_factories.erase(f);
}
