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

///////////////////// DEMPLATES FOR DERIVED CLASSES /////////////////////////

template<class CW, class SW, class C, class S>
class TemplateInstanceWidgetFactory: public OdeInstanceWidgetFactory{
public:
	static TemplateInstanceWidgetFactory* getInstance(){
		return &instance;
	}

	virtual OdeConfigWidget* createConfigWidget(const OdeConfig* cfg = NULL) const {
		const C* ecfg = dynamic_cast<const C*>(cfg);
			assert(ecfg || !cfg);		// it exists or other inexists
		return new CW(ecfg);
	}
	virtual OdeStateWidget* createStateWidget(const OdeConfig* cfg, const OdeState* state = NULL) const {
		const C* ecfg = dynamic_cast<const C*>(cfg);
			assert(ecfg);
		const S* estate = dynamic_cast<const S*>(state);
			assert(estate || !state);		// it exists or other inexists
		return new SW(ecfg, estate);
	}

	virtual std::string getDisplayName() const {
		return CW::getDisplayName();
	}

private:
	static TemplateInstanceWidgetFactory instance;
	TemplateInstanceWidgetFactory()
		:OdeInstanceWidgetFactory(TemplateInstanceWidgetFactory::getInstance()){}
};

template<class CW, class SW, class C, class S>
TemplateInstanceWidgetFactory<CW, SW, C, S> TemplateInstanceWidgetFactory<CW, SW, C, S>::instance;

template<class SF, class SC, class SCW>
class TemplateSolverConfigWidgetFactory: public OdeSolverConfigWidgetFactory{
public:
	static TemplateSolverConfigWidgetFactory* getInstance(){
		return &instance;
	}

	virtual OdeSolverConfigWidget* createConfigWidget(const OdeSolverConfig* cfg = NULL) const {
		const SC* ecfg = dynamic_cast<const SC*>(cfg);
			assert(ecfg || !cfg);
		return new SCW(ecfg);
	}

	virtual std::string getDisplayName() const {
		return SC::getDisplayName();
	}

private:
	static TemplateSolverConfigWidgetFactory instance;
	TemplateSolverConfigWidgetFactory()
		:TemplateSolverConfigWidgetFactory(SF::getInstance())
	{
	}
	virtual ~TemplateSolverConfigWidgetFactory()
	{
	}
};

template<class SF, class SC, class SCW>
TemplateSolverConfigWidgetFactory<SF, SC, SCW> TemplateSolverConfigWidgetFactory<SF, SC, SCW>::instance;

#endif /* GUI_FACTORIES_H_ */
