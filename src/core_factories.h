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
class OdeInstanceFactoryManager;

class OdeInstanceFactory{
public:
	OdeInstance* createInstance() const;
	virtual OdeConfig* createConfig() const = 0;
	virtual OdeState* createState(const OdeConfig*) const = 0;

	virtual std::string getDisplayName() const = 0;
protected:
	OdeInstanceFactory();
	virtual ~OdeInstanceFactory();
};

class OdeSolverFactory{
public:
	virtual OdeSolver* createSolver(const OdeSolverConfig* scfg, const OdeConfig* ocfg, const OdeState* initial_state) const = 0;
	virtual OdeSolverConfig* createSolverConfg() const = 0;

	virtual std::string getDisplayName() const = 0;
	OdeInstanceFactory* getBaseFactory() const {
		return corresponding_instance_factory;
	}

protected:
	OdeSolverFactory(OdeInstanceFactory* corresponding_instance_factory);
	virtual ~OdeSolverFactory();
	OdeInstanceFactory* corresponding_instance_factory;
};

////////////////////// TEMPLATES FOR DERIVED CLASSES ////////////////////////

template<class C, class S>
class TemplateInstanceFactory: public OdeInstanceFactory{
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

	virtual ~TemplateInstanceFactory(){}
private:
	// TODO: write: when this is created it calls parent ctor which calls add(this) - and this fails because getName() isn't here yet (object partially constructed!)
	TemplateInstanceFactory();
};

template<class IF, class SO, class SC, class PC, class ST>
class TemplateSolverFactory: public OdeSolverFactory{
public:
	static TemplateSolverFactory* getInstance(){
		return &instance;
	}

	virtual OdeSolver* createSolver(const OdeSolverConfig* scfg, const OdeConfig* pcfg, const OdeState* initial_state) const {
		const SC* e1_scfg = dynamic_cast<const SC*>(scfg);
			assert(e1_scfg);
		const PC* e1_pcfg = dynamic_cast<const PC*>(pcfg);
			assert(e1_pcfg);
		const ST* e1_state = dynamic_cast<const ST*>(initial_state);
			assert(e1_state);
		return new SO(e1_scfg, e1_pcfg, e1_state);
	}
	virtual OdeSolverConfig* createSolverConfg() const {
		return new SC();
	}

	virtual std::string getDisplayName() const {
		return "PETSc RK solver for e1";
	}
private:
	static TemplateSolverFactory instance;
	TemplateSolverFactory():OdeSolverFactory(IF::getInstance()){
	}
};

template<class IF, class SO, class SC, class PC, class ST>
TemplateSolverFactory<IF, SO, SC, PC, ST> TemplateSolverFactory<IF, SO, SC, PC, ST>::instance;

//////////////////////////// FACTORY MANAGERS ///////////////////////////////
class OdeInstanceFactoryManager{
	typedef std::map<std::string, OdeInstanceFactory*> name_to_inst_map;
public:
	static OdeInstanceFactoryManager* getInstance(){
		static OdeInstanceFactoryManager instance;
		return &instance;
	}
	void add(OdeInstanceFactory* f);
	void remove(OdeInstanceFactory* f);
	std::vector<std::string> getInstanceNames() const;
	OdeInstanceFactory* getFactory(const std::string& name);

private:

	// TODO: See in Josuttis how to store this with possibility to search by name!
	name_to_inst_map instance_factories;
};

template<class AuxFactory, class BaseFactory>
class AuxFactoryManager{
private:
	typedef std::multimap<const BaseFactory*, AuxFactory*> inst_to_aux_map;
public:
	static AuxFactoryManager* getInstance(){
		static AuxFactoryManager instance;
		return &instance;
	}
	void add(AuxFactory* xfact){
		// TODO: think about this const
		aux_map.insert(std::make_pair(xfact->getBaseFactory(), xfact));
	}
	void remove(AuxFactory* f){
		const BaseFactory* ifact = f->getBaseFactory();
		typename inst_to_aux_map::iterator ilow = aux_map.lower_bound(ifact);
		typename inst_to_aux_map::iterator ihi = aux_map.upper_bound(ifact);
			assert(ilow!=aux_map.end());

		// TODO: Написать:
		// 1 Как искать через std::find в multimap'е
		// 2 вложенный класс внутри шаблонного (AuxFactory)
		// 3 статические поля в шаблонных классах (там же)
		// 4: for cross-table: explicit template instantiation for static members (template class MyClass<type>;)
		typename inst_to_aux_map::iterator found = std::find(ilow, ihi, std::pair<const BaseFactory *const, AuxFactory*>(ifact, f));
			assert(found != aux_map.end());
		aux_map.erase(found);
	}

	class FactoryIterator: public std::iterator<std::input_iterator_tag, BaseFactory*>{
		friend class AuxFactoryManager;
	public:
		AuxFactory* operator*() const {
			return iterator->second;
		}
		AuxFactory* operator->() const {
			return iterator->second;
		}
		FactoryIterator& operator++(){				// prefix
			++iterator;
			return *this;
		}
		FactoryIterator operator++(int){			// postfix
			FactoryIterator tmp = *this;
			++iterator;
			return tmp;
		}
		bool operator==(const FactoryIterator& rhs) const {
			return iterator == rhs.iterator;
		}
		bool operator!=(const FactoryIterator& rhs) const {
			return iterator != rhs.iterator;
		}

	private:		// friend interface
		FactoryIterator(typename AuxFactoryManager::inst_to_aux_map::const_iterator it)
			:iterator(it)
		{
		}

	private:		// implementation
		typename AuxFactoryManager::inst_to_aux_map::const_iterator iterator;
	};

	std::pair< FactoryIterator, FactoryIterator > getFactoriesFor(const BaseFactory* f) const {
		auto range_to_return = aux_map.equal_range(f);
		FactoryIterator begin( range_to_return.first );
		FactoryIterator end( range_to_return.second );
		return std::make_pair(begin, end);
	}

	FactoryIterator begin(){
		return FactoryIterator(aux_map.begin());
	}

	FactoryIterator end(){
		return FactoryIterator(aux_map.end());
	}

private:
	inst_to_aux_map aux_map;
};

/////////////////////////////// IMPLEMENTATIONS /////////////////////////////

template<class C, class S>
TemplateInstanceFactory<C, S>::TemplateInstanceFactory(){
	OdeInstanceFactoryManager::getInstance()->add(this);
}

typedef AuxFactoryManager<OdeSolverFactory, OdeInstanceFactory> OdeSolverFactoryManager;
template class AuxFactoryManager<OdeSolverFactory, OdeInstanceFactory>;

// TODO: investigate if I could move ctor of AuxFactoryManaget into cpp-file

#define REGISTER_INSTANCE_FACTORY(NAME, T1, T2) \
typedef TemplateInstanceFactory<T1, T2> NAME;   \
template class TemplateInstanceFactory<T1, T2>;

#define REGISTER_SOLVER_FACTORY(NAME, T1, T2, T3, T4, T5) \
typedef TemplateSolverFactory<T1, T2, T3, T4, T5> NAME;   \
template class TemplateSolverFactory<T1, T2, T3, T4, T5>;

#endif /* CORE_FACTORIES_H_ */
