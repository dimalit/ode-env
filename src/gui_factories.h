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

class OdeProblemWidgetType{
public:
	OdeProblemWidgetType(OdeProblem* corresponding_instance_factory);
	virtual ~OdeProblemWidgetType();
	virtual OdeConfigWidget* createConfigWidget(const OdeConfig* = NULL) const = 0;
	virtual OdeStateGeneratorWidget* createStateGeneratorWidget(const OdeConfig* cfg) const = 0;

	virtual std::string getDisplayName() const = 0;

	OdeProblem* getBaseFactory() const {
		return corresponding_instance_factory;
	}
private:
	OdeProblem* corresponding_instance_factory;
};

class OdeSolverConfigWidgetType{
public:
	OdeSolverConfigWidgetType(OdeSolverFactory* corresponding_solver_factory);
	virtual ~OdeSolverConfigWidgetType();
	virtual OdeSolverConfigWidget* createConfigWidget(const OdeSolverConfig* = NULL) const = 0;

	virtual std::string getDisplayName() const = 0;

	OdeSolverFactory* getBaseFactory() const {
		return corresponding_solver_factory;
	}
private:
	OdeSolverFactory* corresponding_solver_factory;
};

class OdeAnalyzerWidgetType{
public:
	OdeAnalyzerWidgetType(OdeProblem* corresponding_instance_factory);
	virtual ~OdeAnalyzerWidgetType();
	virtual OdeAnalyzerWidget* createAnalyzerWidget(const OdeConfig* = NULL) const = 0;

	virtual std::string getDisplayName() const = 0;

	OdeProblem* getBaseFactory() const {
		return corresponding_instance_factory;
	}
private:
	OdeProblem* corresponding_instance_factory;
};

typedef AuxTypeManager<OdeProblemWidgetType, OdeProblem> OdeInstanceWidgetFactoryManager;
template class AuxTypeManager<OdeProblemWidgetType, OdeProblem>;
typedef AuxTypeManager<OdeSolverConfigWidgetType, OdeSolverFactory> OdeSolverConfigWidgetFactoryManager;
template class AuxTypeManager<OdeSolverConfigWidgetType, OdeSolverFactory>;
typedef AuxTypeManager<OdeAnalyzerWidgetType, OdeProblem> OdeAnalyzerWidgetFactoryManager;
template class AuxTypeManager<OdeAnalyzerWidgetType, OdeProblem>;

///////////////////// DEMPLATES FOR DERIVED CLASSES /////////////////////////

template<class CW, class SW>
class TemplateInstanceWidgetFactory: public OdeProblemWidgetType{
public:
	virtual OdeConfigWidget* createConfigWidget(const OdeConfig* cfg = NULL) const {
		const typename CW::Config* ecfg = dynamic_cast<const typename CW::Config*>(cfg);
			assert(ecfg || !cfg);		// it exists or other inexists
		return new CW(ecfg);
	}
	virtual OdeStateGeneratorWidget* createStateGeneratorWidget(const OdeConfig* cfg) const {
		const typename CW::Config* ecfg = dynamic_cast<const typename CW::Config*>(cfg);
			assert(ecfg);
		return new SW(ecfg);
	}

	virtual std::string getDisplayName() const {
		return CW::getDisplayName();
	}

private:
	static TemplateInstanceWidgetFactory instance;
	TemplateInstanceWidgetFactory()
		:OdeProblemWidgetType(TemplateInstanceFactory<typename CW::Config, typename SW::State>::getInstance())
	{
	}
};

template<class CW, class SW>
TemplateInstanceWidgetFactory<CW, SW> TemplateInstanceWidgetFactory<CW, SW>::instance;

template<class SCW>
class TemplateSolverConfigWidgetFactory: public OdeSolverConfigWidgetType{
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
		:OdeSolverConfigWidgetType(TemplateSolverFactory<TemplateInstanceFactory<typename SCW::Solver::PConfig, typename SCW::Solver::State>, typename SCW::Solver>::getInstance())
	{
	}
	virtual ~TemplateSolverConfigWidgetFactory()
	{
	}
};

template<class SCW>
TemplateSolverConfigWidgetFactory<SCW> TemplateSolverConfigWidgetFactory<SCW>::instance;

template<class AW>
class TemplateAnalyzerWidgetFactory: public OdeAnalyzerWidgetType{
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
		:OdeAnalyzerWidgetType(TemplateInstanceFactory<typename AW::Config, typename AW::State>::getInstance())
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
