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

	// remove related solvers
	auto range_to_delete = solvers_map.equal_range(f);
	solvers_map.erase(range_to_delete.first, range_to_delete.second);

	// remove itself
	instance_factories.erase(f);
}

std::pair<OdeInstanceFactoryManager::SupportedSolversIterator, OdeInstanceFactoryManager::SupportedSolversIterator>
OdeInstanceFactoryManager::getSupportedSolvers(const OdeInstanceFactory* f) const {
	//std::pair<inst_to_solvers_map::iterator, inst_to_solvers_map::iterator>
	auto
		range_to_return = solvers_map.equal_range(f);
	SupportedSolversIterator begin( range_to_return.first );
	SupportedSolversIterator end( range_to_return.second );
	return std::make_pair(begin, end);
}

bool OdeInstanceFactoryManager::isSolverSupported(const OdeInstanceFactory* ifactory, const OdeSolverFactory* sfactory) const {
	auto range = solvers_map.equal_range(ifactory);
	return std::find_if(range.first, range.second,
			[=](const inst_to_solvers_map::value_type& pr){
				return pr.second == sfactory;
			}
	) != range.second;
}

void OdeInstanceFactoryManager::addSupportedSolver(const OdeInstanceFactory* ifactory, OdeSolverFactory* sfactory){
	// TODO: think about this const
	assert(instance_factories.find(const_cast<OdeInstanceFactory*>(ifactory)) != instance_factories.end());
	solvers_map.insert(std::make_pair(ifactory, sfactory));
}

void OdeSolverFactoryManager::add(OdeSolverFactory* f){
	assert(solver_factories.find(f)==solver_factories.end());
	solver_factories.insert(f);

	// tell our corresponding instance factory about us!
	OdeInstanceFactoryManager::getInstance()->addSupportedSolver(f->getCorrespondingInstanceFactory(), f);
}

void OdeSolverFactoryManager::remove(OdeSolverFactory* f){
	assert(solver_factories.find(f)!=solver_factories.end());
	solver_factories.erase(f);
}
