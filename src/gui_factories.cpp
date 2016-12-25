/*
 * gui_interfaces.cpp
 *
 *  Created on: Jul 5, 2014
 *      Author: dimalit
 */

#include "gui_factories.h"

OdeProblemWidgetType::OdeProblemWidgetType(OdeProblem* corresponding_instance_factory){
	this->corresponding_instance_factory = corresponding_instance_factory;
	OdeInstanceWidgetFactoryManager::getInstance()->add(this);
}
OdeProblemWidgetType::~OdeProblemWidgetType(){
	OdeInstanceWidgetFactoryManager::getInstance()->remove(this);
}

OdeSolverConfigWidgetType::OdeSolverConfigWidgetType(OdeSolverFactory* corresponding_solver_factory){
	this->corresponding_solver_factory = corresponding_solver_factory;
	OdeSolverConfigWidgetFactoryManager::getInstance()->add(this);
}
OdeSolverConfigWidgetType::~OdeSolverConfigWidgetType(){
	OdeSolverConfigWidgetFactoryManager::getInstance()->remove(this);
}

OdeAnalyzerWidgetType::OdeAnalyzerWidgetType(OdeProblem* corresponding_instance_factory){
	this->corresponding_instance_factory = corresponding_instance_factory;
	OdeAnalyzerWidgetFactoryManager::getInstance()->add(this);
}
OdeAnalyzerWidgetType::~OdeAnalyzerWidgetType(){
	OdeAnalyzerWidgetFactoryManager::getInstance()->remove(this);
}

