/*
 * core_factories.cpp
 *
 *  Created on: Dec 14, 2013
 *      Author: dimalit
 */

#include "core_factories.h"

#include "core_interfaces.h"

#include <cassert>

OdeProblem::OdeProblem(){
	//TODO: better it worked. but for now - moved to children...
	//OdeProblemManager::getInstance()->add(this);
}

OdeProblem::~OdeProblem(){
	//TODO: better it worked. but for now - moved to children...
	//OdeProblemManager::getInstance()->remove(this);
}

OdeInstance* OdeProblem::createInstance() const {
	OdeConfig* c = createConfig();
	OdeState*  s = createState(c);
	return new OdeInstance(c, s);
}

OdeSolverFactory::OdeSolverFactory(OdeProblem* corresponding_instance_factory){
	assert(corresponding_instance_factory);
	this->corresponding_instance_factory = corresponding_instance_factory;

	OdeSolverFactoryManager::getInstance()->add(this);
}
OdeSolverFactory::~OdeSolverFactory(){
	OdeSolverFactoryManager::getInstance()->remove(this);
}

///////////////////////////// FACTORY MANAGERS //////////////////////////////

void OdeProblemManager::add(OdeProblem* f){
	std::string key = f->getDisplayName();
	assert(instance_factories.find(key)==instance_factories.end());
	instance_factories[key] = f;
}

void OdeProblemManager::remove(OdeProblem* f){
	assert(instance_factories.find(f->getDisplayName())!=instance_factories.end());

	// TODO: remove related solvers!

	// remove itself
	instance_factories.erase(f->getDisplayName());
}

std::vector<std::string> OdeProblemManager::getProblemNames() const {
	std::vector<std::string> v;
	std::for_each(instance_factories.begin(), instance_factories.end(), [&](const name_to_inst_map::value_type& pr){v.push_back(pr.second->getDisplayName());});
	return v;
}
OdeProblem* OdeProblemManager::getProblem(const std::string& name){
	return instance_factories.at(name);
}
