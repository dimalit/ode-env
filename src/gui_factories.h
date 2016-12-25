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
	OdeProblemWidgetType(Problem* corresponding_problem);
	virtual ~OdeProblemWidgetType();
	virtual OdeConfigWidget* createConfigWidget(const OdeConfig* = NULL) const = 0;
	virtual OdeStateGeneratorWidget* createStateGeneratorWidget(const OdeConfig* cfg) const = 0;

	virtual std::string getDisplayName() const = 0;

	Problem* getBase() const {
		return corresponding_problem;
	}
private:
	Problem* corresponding_problem;
};

class OdeSolverConfigWidgetType{
public:
	OdeSolverConfigWidgetType(SolverType* corresponding_solver_type);
	virtual ~OdeSolverConfigWidgetType();
	virtual OdeSolverConfigWidget* createConfigWidget(const OdeSolverConfig* = NULL) const = 0;

	virtual std::string getDisplayName() const = 0;

	SolverType* getBase() const {
		return corresponding_solver_type;
	}
private:
	SolverType* corresponding_solver_type;
};

class OdeAnalyzerWidgetType{
public:
	OdeAnalyzerWidgetType(Problem* corresponding_problem);
	virtual ~OdeAnalyzerWidgetType();
	virtual OdeAnalyzerWidget* createAnalyzerWidget(const OdeConfig* = NULL) const = 0;

	virtual std::string getDisplayName() const = 0;

	Problem* getBase() const {
		return corresponding_problem;
	}
private:
	Problem* corresponding_problem;
};

typedef AuxTypeManager<OdeProblemWidgetType, Problem> OdeInstanceWidgetManager;
template class AuxTypeManager<OdeProblemWidgetType, Problem>;
typedef AuxTypeManager<OdeSolverConfigWidgetType, SolverType> OdeSolverConfigWidgetManager;
template class AuxTypeManager<OdeSolverConfigWidgetType, SolverType>;
typedef AuxTypeManager<OdeAnalyzerWidgetType, Problem> OdeAnalyzerWidgetManager;
template class AuxTypeManager<OdeAnalyzerWidgetType, Problem>;

///////////////////// DEMPLATES FOR DERIVED CLASSES /////////////////////////

template<class CW, class SW>
class TemplateInstanceWidgetType: public OdeProblemWidgetType{
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
	static TemplateInstanceWidgetType instance;
	TemplateInstanceWidgetType()
		:OdeProblemWidgetType(TemplateProblem<typename CW::Config, typename SW::State>::getInstance())
	{
	}
};

template<class CW, class SW>
TemplateInstanceWidgetType<CW, SW> TemplateInstanceWidgetType<CW, SW>::instance;

template<class SCW>
class TemplateSolverConfigWidgetType: public OdeSolverConfigWidgetType{
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
	static TemplateSolverConfigWidgetType instance;
	TemplateSolverConfigWidgetType()
		:OdeSolverConfigWidgetType(TemplateSolverType<TemplateProblem<typename SCW::Solver::PConfig, typename SCW::Solver::State>, typename SCW::Solver>::getInstance())
	{
	}
	virtual ~TemplateSolverConfigWidgetType()
	{
	}
};

template<class SCW>
TemplateSolverConfigWidgetType<SCW> TemplateSolverConfigWidgetType<SCW>::instance;

template<class AW>
class TemplateAnalyzerWidgetType: public OdeAnalyzerWidgetType{
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
	static TemplateAnalyzerWidgetType instance;
	TemplateAnalyzerWidgetType()
		:OdeAnalyzerWidgetType(TemplateProblem<typename AW::Config, typename AW::State>::getInstance())
	{
	}
};

template<class AW>
TemplateAnalyzerWidgetType<AW> TemplateAnalyzerWidgetType<AW>::instance;

#define REGISTER_INSTANCE_WIDGET_CLASSES(CW, SW) \
template class TemplateInstanceWidgetType<CW, SW>;

#define REGISTER_SOLVER_CONFIG_WIDGET_CLASS(SCW) \
template class TemplateSolverConfigWidgetType<SCW>;

#define REGISTER_ANALYZER_WIDGET_CLASS(AW) \
template class TemplateAnalyzerWidgetType<AW>;

#endif /* GUI_FACTORIES_H_ */
