/*
 * gui_factories.h
 *
 *  Created on: Dec 14, 2013
 *      Author: dimalit
 */

#ifndef GUI_FACTORIES_H_
#define GUI_FACTORIES_H_

#include "gui_interfaces.h"
#include "core_factory_managers.h"

class OdeInstanceWidgetFactory{
public:
	OdeInstanceWidgetFactory(OdeInstanceFactory* corresponding_instance_factory);
	virtual ~OdeInstanceWidgetFactory();
	virtual OdeConfigWidget* createConfigWidget(const OdeConfig* = NULL) const = 0;
	virtual OdeStateWidget* createStateWidget(const OdeConfig* cfg, const OdeState* state = NULL) const = 0;

	virtual std::string getDisplayName() const = 0;

	OdeInstanceFactory* getBaseFactory() const {
		return corresponding_instance_factory;
	}
private:
	OdeInstanceFactory* corresponding_instance_factory;
};

class OdeSolverConfigWidgetFactory{
public:
	OdeSolverConfigWidgetFactory(OdeSolverFactory* corresponding_solver_factory);
	virtual ~OdeSolverConfigWidgetFactory();
	virtual OdeSolverConfigWidget* createConfigWidget(const OdeSolverConfig* = NULL) const = 0;

	virtual std::string getDisplayName() const = 0;

	OdeSolverFactory* getBaseFactory() const {
		return corresponding_solver_factory;
	}
private:
	OdeSolverFactory* corresponding_solver_factory;
};

typedef AuxFactoryManager<OdeInstanceWidgetFactory, OdeInstanceFactory> OdeInstanceWidgetFactoryManager;
typedef AuxFactoryManager<OdeSolverConfigWidgetFactory, OdeSolverFactory> OdeSolverConfigWidgetFactoryManager;

#endif /* GUI_FACTORIES_H_ */
