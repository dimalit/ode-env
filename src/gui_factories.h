/*
 * gui_factories.h
 *
 *  Created on: Dec 14, 2013
 *      Author: dimalit
 */

#ifndef GUI_FACTORIES_H_
#define GUI_FACTORIES_H_

#include "gui_interfaces.h"
#include "core_factories.h"

#include <iostream>

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
template class AuxFactoryManager<OdeInstanceWidgetFactory, OdeInstanceFactory>;
typedef AuxFactoryManager<OdeSolverConfigWidgetFactory, OdeSolverFactory> OdeSolverConfigWidgetFactoryManager;
template class AuxFactoryManager<OdeSolverConfigWidgetFactory, OdeSolverFactory>;

///////////////////// DEMPLATES FOR DERIVED CLASSES /////////////////////////

template<class IF, class CW, class SW, class C, class S>
class TemplateInstanceWidgetFactory: public OdeInstanceWidgetFactory{
public:
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
		:OdeInstanceWidgetFactory(IF::getInstance())
	{
	}
};

template<class IF, class CW, class SW, class C, class S>
TemplateInstanceWidgetFactory<IF, CW, SW, C, S> TemplateInstanceWidgetFactory<IF, CW, SW, C, S>::instance;

template<class SF, class SC, class SCW>
class TemplateSolverConfigWidgetFactory: public OdeSolverConfigWidgetFactory{
public:
	virtual OdeSolverConfigWidget* createConfigWidget(const OdeSolverConfig* cfg = NULL) const {
		const SC* ecfg = dynamic_cast<const SC*>(cfg);
			assert(ecfg || !cfg);
		return new SCW(ecfg);
	}

	virtual std::string getDisplayName() const {
		return SCW::getDisplayName();
	}

private:
	static TemplateSolverConfigWidgetFactory instance;
	TemplateSolverConfigWidgetFactory()
		:OdeSolverConfigWidgetFactory(SF::getInstance())
	{
	}
	virtual ~TemplateSolverConfigWidgetFactory()
	{
	}
};

template<class SF, class SC, class SCW>
TemplateSolverConfigWidgetFactory<SF, SC, SCW> TemplateSolverConfigWidgetFactory<SF, SC, SCW>::instance;

#define REGISTER_INSTANCE_WIDGET_FACTORY(NAME, T1, T2, T3, T4, T5) \
typedef TemplateInstanceWidgetFactory<T1, T2, T3, T4, T5> NAME;    \
template class TemplateInstanceWidgetFactory<T1, T2, T3, T4, T5>;

#define REGISTER_SOLVER_CONFIG_WIDGET_FACTORY(NAME, T1, T2, T3) \
typedef TemplateSolverConfigWidgetFactory<T1, T2, T3> NAME;     \
template class TemplateSolverConfigWidgetFactory<T1, T2, T3>;

#endif /* GUI_FACTORIES_H_ */
