/*
 * gui_interfaces.cpp
 *
 *  Created on: Jul 5, 2014
 *      Author: dimalit
 */

#include "gui_factories.h"

OdeProblemWidgetType::OdeProblemWidgetType(Problem* corresponding_problem){
	this->corresponding_problem = corresponding_problem;
	OdeInstanceWidgetManager::getInstance()->add(this);
}
OdeProblemWidgetType::~OdeProblemWidgetType(){
	OdeInstanceWidgetManager::getInstance()->remove(this);
}

OdeSolverConfigWidgetType::OdeSolverConfigWidgetType(SolverType* corresponding_solver_type){
	this->corresponding_solver_type = corresponding_solver_type;
	OdeSolverConfigWidgetManager::getInstance()->add(this);
}
OdeSolverConfigWidgetType::~OdeSolverConfigWidgetType(){
	OdeSolverConfigWidgetManager::getInstance()->remove(this);
}

OdeAnalyzerWidgetType::OdeAnalyzerWidgetType(Problem* corresponding_problem){
	this->corresponding_problem = corresponding_problem;
	OdeAnalyzerWidgetManager::getInstance()->add(this);
}
OdeAnalyzerWidgetType::~OdeAnalyzerWidgetType(){
	OdeAnalyzerWidgetManager::getInstance()->remove(this);
}

