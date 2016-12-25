/*
 * core_factories.h
 *
 *  Created on: Dec 14, 2013
 *      Author: dimalit
 */

#ifndef CORE_FACTORIES_H_
#define CORE_FACTORIES_H_

#include <string>

#include <map>
#include <vector>
#include <algorithm>

#include <cassert>

class OdeConfig;
class OdeState;
class OdeInstance;
class OdeSolverConfig;
class OdeSolver;
class OdeProblemManager;

class OdeProblem{
public:
	OdeInstance* createInstance() const;
	virtual OdeConfig* createConfig() const = 0;
	virtual OdeState* createState(const OdeConfig*) const = 0;

	virtual std::string getDisplayName() const = 0;
protected:
	OdeProblem();
	virtual ~OdeProblem();
};

class OdeSolverFactory{
public:
	virtual OdeSolver* createSolver(const OdeSolverConfig* scfg, const OdeConfig* ocfg, const OdeState* initial_state) const = 0;
	virtual OdeSolverConfig* createSolverConfg() const = 0;

	virtual std::string getDisplayName() const = 0;
	OdeProblem* getBaseFactory() const {
		return corresponding_instance_factory;
	}

protected:
	OdeSolverFactory(OdeProblem* corresponding_instance_factory);
	virtual ~OdeSolverFactory();
	OdeProblem* corresponding_instance_factory;
};

////////////////////// TEMPLATES FOR DERIVED CLASSES ////////////////////////

template<class C, class S>
class TemplateInstanceFactory: public OdeProblem{
public:
	static TemplateInstanceFactory* getInstance(){
		static TemplateInstanceFactory instance;
		return &instance;
	}

	virtual OdeConfig* createConfig() const {
		return new C();
	}
	virtual OdeState* createState(const OdeConfig* cfg) const {
		const C* ecfg = dynamic_cast<const C*>(cfg);
			assert(ecfg);
		return new S(ecfg);
	}

	virtual std::string getDisplayName() const {
		return C::getDisplayName();
	}

	virtual ~TemplateInstanceFactory();
private:
	// TODO: write: when this is created it calls parent ctor which calls add(this) - and this fails because getName() isn't here yet (object partially constructed!)
	TemplateInstanceFactory();
};

template<class IF, class SOLVER>
class TemplateSolverFactory: public OdeSolverFactory{
public:
	static TemplateSolverFactory* getInstance(){
		return &instance;
	}

	virtual OdeSolver* createSolver(const OdeSolverConfig* scfg, const OdeConfig* pcfg, const OdeState* initial_state) const {
		const typename SOLVER::SConfig* e1_scfg = dynamic_cast<const typename SOLVER::SConfig*>(scfg);
			assert(e1_scfg);
		const typename SOLVER::PConfig* e1_pcfg = dynamic_cast<const typename SOLVER::PConfig*>(pcfg);
			assert(e1_pcfg);
		const typename SOLVER::State* e1_state = dynamic_cast<const typename SOLVER::State*>(initial_state);
			assert(e1_state);
		return new SOLVER(e1_scfg, e1_pcfg, e1_state);
	}
	virtual OdeSolverConfig* createSolverConfg() const {
		return new typename SOLVER::SConfig();
	}

	virtual std::string getDisplayName() const {
		return SOLVER::getDisplayName();
	}
private:
	static TemplateSolverFactory instance;
	TemplateSolverFactory():OdeSolverFactory(IF::getInstance()){
	}
};

template<class IF, class SOLVER>
TemplateSolverFactory<IF, SOLVER> TemplateSolverFactory<IF, SOLVER>::instance;

//////////////////////////// FACTORY MANAGERS ///////////////////////////////
class OdeProblemManager{
	typedef std::map<std::string, OdeProblem*> name_to_inst_map;
public:
	static OdeProblemManager* getInstance(){
		static OdeProblemManager instance;
		return &instance;
	}
	void add(OdeProblem* f);
	void remove(OdeProblem* f);
	std::vector<std::string> getProblemNames() const;
	OdeProblem* getProblem(const std::string& name);

private:

	// TODO: See in Josuttis how to store this with possibility to search by name!
	name_to_inst_map instance_factories;
};

template<class AuxType, class ProblemType>
class AuxTypeManager{
private:
	typedef std::multimap<const ProblemType*, AuxType*> inst_to_aux_map;
public:
	static AuxTypeManager* getInstance(){
		static AuxTypeManager instance;
		return &instance;
	}
	void add(AuxType* xfact){
		// TODO: think about this const
		aux_map.insert(std::make_pair(xfact->getBaseFactory(), xfact));
	}
	void remove(AuxType* f){
		const ProblemType* ifact = f->getBaseFactory();
		typename inst_to_aux_map::iterator ilow = aux_map.lower_bound(ifact);
		typename inst_to_aux_map::iterator ihi = aux_map.upper_bound(ifact);
			assert(ilow!=aux_map.end());

		// TODO: Написать:
		// 1 Как искать через std::find в multimap'е
		// 2 вложенный класс внутри шаблонного (AuxFactory)
		// 3 статические поля в шаблонных классах (там же)
		// 4 for cross-table: explicit template instantiation for static members (template class MyClass<type>;)
		// 5 Управление gnuplot через pipe'ы
		typename inst_to_aux_map::iterator found = std::find(ilow, ihi, std::pair<const ProblemType *const, AuxType*>(ifact, f));
			assert(found != aux_map.end());
		aux_map.erase(found);
	}

	class TypeIterator: public std::iterator<std::input_iterator_tag, ProblemType*>{
		friend class AuxTypeManager;
	public:
		AuxType* operator*() const {
			return iterator->second;
		}
		AuxType* operator->() const {
			return iterator->second;
		}
		TypeIterator& operator++(){				// prefix
			++iterator;
			return *this;
		}
		TypeIterator operator++(int){			// postfix
			TypeIterator tmp = *this;
			++iterator;
			return tmp;
		}
		bool operator==(const TypeIterator& rhs) const {
			return iterator == rhs.iterator;
		}
		bool operator!=(const TypeIterator& rhs) const {
			return iterator != rhs.iterator;
		}

	private:		// friend interface
		TypeIterator(typename AuxTypeManager::inst_to_aux_map::const_iterator it)
			:iterator(it)
		{
		}

	private:		// implementation
		typename AuxTypeManager::inst_to_aux_map::const_iterator iterator;
	};

	std::pair< TypeIterator, TypeIterator > getTypesFor(const ProblemType* f) const {
		auto range_to_return = aux_map.equal_range(f);
		TypeIterator begin( range_to_return.first );
		TypeIterator end( range_to_return.second );
		return std::make_pair(begin, end);
	}

	TypeIterator begin(){
		return TypeIterator(aux_map.begin());
	}

	TypeIterator end(){
		return TypeIterator(aux_map.end());
	}

private:
	inst_to_aux_map aux_map;
};

/////////////////////////////// IMPLEMENTATIONS /////////////////////////////

template<class C, class S>
TemplateInstanceFactory<C, S>::TemplateInstanceFactory(){
	OdeProblemManager::getInstance()->add(this);
}

template<class C, class S>
TemplateInstanceFactory<C, S>::~TemplateInstanceFactory(){
	OdeProblemManager::getInstance()->remove(this);
}

typedef AuxTypeManager<OdeSolverFactory, OdeProblem> OdeSolverFactoryManager;
template class AuxTypeManager<OdeSolverFactory, OdeProblem>;

// TODO: investigate if I could move ctor of AuxFactoryManaget into cpp-file

#define REGISTER_INSTANCE_CLASS(C, S) \
template class TemplateInstanceFactory<C, S>;

#define REGISTER_SOLVER_CLASS(SOLVER) \
template class TemplateSolverFactory<TemplateInstanceFactory<SOLVER::PConfig, SOLVER::State>, SOLVER>;

#endif /* CORE_FACTORIES_H_ */
