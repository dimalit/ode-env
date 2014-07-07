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

	OdeInstanceFactory* getCorrespondingInstanceFactory() const {
		return corresponding_instance_factory;
	}
private:
	OdeInstanceFactory* corresponding_instance_factory;
};


class OdeSolverConfigWidgetFactory{

};

typedef AuxFactoryManager<OdeInstanceWidgetFactory> OdeInstanceWidgetFactoryManager;
//typedef AuxFactoryManager<OdeSolverConfigWidgetFactory> OdeSolverConfigWidgetFactoryManager;

#endif /* GUI_FACTORIES_H_ */
