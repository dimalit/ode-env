/*
 * core_factory_managers.h
 *
 *  Created on: Dec 14, 2013
 *      Author: dimalit
 */

#ifndef CORE_FACTORY_MANAGERS_H_
#define CORE_FACTORY_MANAGERS_H_

#include <set>
#include <map>
#include <algorithm>
#include <vector>
#include <cassert>

class OdeInstanceFactory;
class OdeSolverFactory;

class OdeInstanceFactoryManager{
	typedef std::map<std::string, OdeInstanceFactory*> name_to_inst_map;
public:
	static OdeInstanceFactoryManager* getInstance(){
		return &instance;
	}
	void add(OdeInstanceFactory* f);
	void remove(OdeInstanceFactory* f);
	std::vector<std::string> getInstanceNames() const;
	OdeInstanceFactory* getFactory(const std::string& name);

private:
	static OdeInstanceFactoryManager instance;

	// TODO: See in Josuttis how to store this with possibility to search by name!
	name_to_inst_map instance_factories;
};

template<class AuxFactory, class BaseFactory>
class AuxFactoryManager{
private:
	typedef std::multimap<const BaseFactory*, AuxFactory*> inst_to_aux_map;
public:
	static AuxFactoryManager* getInstance(){
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

	std::pair< FactoryIterator, FactoryIterator > getFactoriesFor(const OdeInstanceFactory* f) const {
		auto range_to_return = aux_map.equal_range(f);
		FactoryIterator begin( range_to_return.first );
		FactoryIterator end( range_to_return.second );
		return std::make_pair(begin, end);
	}

	FactoryIterator begin(){
		return aux_map.begin();
	}

	FactoryIterator end(){
		return aux_map.end();
	}

private:
	static AuxFactoryManager instance;
	inst_to_aux_map aux_map;
};

template<class AuxFactory, class BaseFactory>
AuxFactoryManager<AuxFactory, BaseFactory> AuxFactoryManager<AuxFactory, BaseFactory>::instance;

typedef AuxFactoryManager<OdeSolverFactory, OdeInstanceFactory> OdeSolverFactoryManager;

#endif /* CORE_FACTORY_MANAGERS_H_ */
