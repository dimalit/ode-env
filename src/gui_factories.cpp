/*
 * gui_interfaces.cpp
 *
 *  Created on: Jul 5, 2014
 *      Author: dimalit
 */

#include "gui_factories.h"

OdeInstanceWidgetFactory::OdeInstanceWidgetFactory(OdeInstanceFactory* corresponding_instance_factory){
	this->corresponding_instance_factory = corresponding_instance_factory;
	OdeInstanceWidgetFactoryManager::getInstance()->add(this);
}
OdeInstanceWidgetFactory::~OdeInstanceWidgetFactory(){
	OdeInstanceWidgetFactoryManager::getInstance()->remove(this);
}

OdeSolverConfigWidgetFactory::OdeSolverConfigWidgetFactory(OdeSolverFactory* corresponding_solver_factory){
	this->corresponding_solver_factory = corresponding_solver_factory;
	OdeSolverConfigWidgetFactoryManager::getInstance()->add(this);
}
OdeSolverConfigWidgetFactory::~OdeSolverConfigWidgetFactory(){
	OdeSolverConfigWidgetFactoryManager::getInstance()->remove(this);
}

OdeAnalyzerWidgetFactory::OdeAnalyzerWidgetFactory(OdeInstanceFactory* corresponding_instance_factory){
	this->corresponding_instance_factory = corresponding_instance_factory;
	OdeAnalyzerWidgetFactoryManager::getInstance()->add(this);
}
OdeAnalyzerWidgetFactory::~OdeAnalyzerWidgetFactory(){
	OdeAnalyzerWidgetFactoryManager::getInstance()->remove(this);
}

