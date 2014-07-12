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

void OdeInstanceFactoryManager::add(OdeInstanceFactory* f){
	std::string key = f->getDisplayName();
	assert(instance_factories.find(key)==instance_factories.end());
	instance_factories[key] = f;
}

void OdeInstanceFactoryManager::remove(OdeInstanceFactory* f){
	assert(instance_factories.find(f->getDisplayName())!=instance_factories.end());

	// TODO: remove related solvers!

	// remove itself
	instance_factories.erase(f->getDisplayName());
}

std::vector<std::string> OdeInstanceFactoryManager::getInstanceNames() const {
	std::vector<std::string> v;
	std::for_each(instance_factories.begin(), instance_factories.end(), [&](const name_to_inst_map::value_type& pr){v.push_back(pr.second->getDisplayName());});
	return v;
}
OdeInstanceFactory* OdeInstanceFactoryManager::getFactory(const std::string& name){
	return instance_factories.at(name);
}
