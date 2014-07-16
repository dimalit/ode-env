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

class OdeAnalyzerWidgetFactory{
public:
	OdeAnalyzerWidgetFactory(OdeInstanceFactory* corresponding_instance_factory);
	virtual ~OdeAnalyzerWidgetFactory();
	virtual OdeAnalyzerWidget* createAnalyzerWidget(const OdeConfig* = NULL) const = 0;

	virtual std::string getDisplayName() const = 0;

	OdeInstanceFactory* getBaseFactory() const {
		return corresponding_instance_factory;
	}
private:
	OdeInstanceFactory* corresponding_instance_factory;
};

typedef AuxFactoryManager<OdeInstanceWidgetFactory, OdeInstanceFactory> OdeInstanceWidgetFactoryManager;
template class AuxFactoryManager<OdeInstanceWidgetFactory, OdeInstanceFactory>;
typedef AuxFactoryManager<OdeSolverConfigWidgetFactory, OdeSolverFactory> OdeSolverConfigWidgetFactoryManager;
template class AuxFactoryManager<OdeSolverConfigWidgetFactory, OdeSolverFactory>;
typedef AuxFactoryManager<OdeAnalyzerWidgetFactory, OdeInstanceFactory> OdeAnalyzerWidgetFactoryManager;
template class AuxFactoryManager<OdeAnalyzerWidgetFactory, OdeInstanceFactory>;

///////////////////// DEMPLATES FOR DERIVED CLASSES /////////////////////////

template<class CW, class SW>
class TemplateInstanceWidgetFactory: public OdeInstanceWidgetFactory{
public:
	virtual OdeConfigWidget* createConfigWidget(const OdeConfig* cfg = NULL) const {
		const typename CW::Config* ecfg = dynamic_cast<const typename CW::Config*>(cfg);
			assert(ecfg || !cfg);		// it exists or other inexists
		return new CW(ecfg);
	}
	virtual OdeStateWidget* createStateWidget(const OdeConfig* cfg, const OdeState* state = NULL) const {
		const typename CW::Config* ecfg = dynamic_cast<const typename CW::Config*>(cfg);
			assert(ecfg);
		const typename SW::State* estate = dynamic_cast<const typename SW::State*>(state);
			assert(estate || !state);		// it exists or other inexists
		return new SW(ecfg, estate);
	}

	virtual std::string getDisplayName() const {
		return CW::getDisplayName();
	}

private:
	static TemplateInstanceWidgetFactory instance;
	TemplateInstanceWidgetFactory()
		:OdeInstanceWidgetFactory(TemplateInstanceFactory<typename CW::Config, typename SW::State>::getInstance())
	{
	}
};

template<class CW, class SW>
TemplateInstanceWidgetFactory<CW, SW> TemplateInstanceWidgetFactory<CW, SW>::instance;

template<class SCW>
class TemplateSolverConfigWidgetFactory: public OdeSolverConfigWidgetFactory{
public:
	virtual OdeSolverConfigWidget* createConfigWidget(const OdeSolverConfig* cfg = NULL) const {
		const typename SCW::Solver::SConfig* ecfg = dynamic_cast<const typename SCW::Solver::SConfig*>(cfg);
			assert(ecfg || !cfg);
		return new SCW(ecfg);
	}

	virtual std::string getDisplayName() const {
		return SCW::getDisplayName();
	}

private:
	static TemplateSolverConfigWidgetFactory instance;
	TemplateSolverConfigWidgetFactory()
		:OdeSolverConfigWidgetFactory(TemplateSolverFactory<TemplateInstanceFactory<typename SCW::Solver::PConfig, typename SCW::Solver::State>, typename SCW::Solver>::getInstance())
	{
	}
	virtual ~TemplateSolverConfigWidgetFactory()
	{
	}
};

template<class SCW>
TemplateSolverConfigWidgetFactory<SCW> TemplateSolverConfigWidgetFactory<SCW>::instance;

template<class AW>
class TemplateAnalyzerWidgetFactory: public OdeAnalyzerWidgetFactory{
public:
	virtual OdeAnalyzerWidget* createAnalyzerWidget(const OdeConfig* cfg = NULL) const {
		const typename AW::Config* ecfg = dynamic_cast<const typename AW::Config*>(cfg);
			assert(ecfg || !cfg);		// it exists or other inexists
		return new AW(ecfg);
	}
	virtual std::string getDisplayName() const {
		return AW::getDisplayName();
	}

private:
	static TemplateAnalyzerWidgetFactory instance;
	TemplateAnalyzerWidgetFactory()
		:OdeAnalyzerWidgetFactory(TemplateInstanceFactory<typename AW::Config, typename AW::State>::getInstance())
	{
	}
};

template<class AW>
TemplateAnalyzerWidgetFactory<AW> TemplateAnalyzerWidgetFactory<AW>::instance;

#define REGISTER_INSTANCE_WIDGET_CLASSES(CW, SW) \
template class TemplateInstanceWidgetFactory<CW, SW>;

#define REGISTER_SOLVER_CONFIG_WIDGET_CLASS(SCW) \
template class TemplateSolverConfigWidgetFactory<SCW>;

#define REGISTER_ANALYZER_WIDGET_CLASS(AW) \
template class TemplateAnalyzerWidgetFactory<AW>;

#endif /* GUI_FACTORIES_H_ */
