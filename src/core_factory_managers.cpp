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
