/*
 * core_factory_managers.cpp
 *
 *  Created on: Dec 14, 2013
 *      Author: dimalit
 */

#include "core_factory_managers.h"

#include "core_factories.h"

#include <algorithm>
#include <vector>
#include <cassert>

std::vector<int> v;

OdeInstanceFactoryManager OdeInstanceFactoryManager::instance;
OdeSolverFactoryManager OdeSolverFactoryManager::instance;

void OdeInstanceFactoryManager::add(OdeInstanceFactory* f){
	assert(instance_factories.find(f)==instance_factories.end());
	instance_factories.insert(f);
}

void OdeInstanceFactoryManager::remove(OdeInstanceFactory* f){
	assert(instance_factories.find(f)!=instance_factories.end());

	// TODO: remove related solvers!

	// remove itself
	instance_factories.erase(f);
}

void OdeSolverFactoryManager::add(OdeSolverFactory* sfact, OdeInstanceFactory* ifact){
	// TODO: think about this const
	inst_to_solvers_map.insert(std::make_pair(ifact, sfact));
}

void OdeSolverFactoryManager::remove(OdeSolverFactory* f){
	OdeInstanceFactory* ifact = f->getCorrespondingInstanceFactory();
		assert(inst_to_solvers_map.find(ifact)!=inst_to_solvers_map.end());
		inst_to_solvers_map.erase(ifact);
}

std::pair<OdeSolverFactoryManager::SupportedSolversIterator, OdeSolverFactoryManager::SupportedSolversIterator>
OdeSolverFactoryManager::getSupportedSolvers(const OdeInstanceFactory* f) const {
	auto range_to_return = solvers_map.equal_range(f);
	SupportedSolversIterator begin( range_to_return.first );
	SupportedSolversIterator end( range_to_return.second );
	return std::make_pair(begin, end);
}
