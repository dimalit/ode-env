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
#include <cassert>

class OdeInstanceFactory;
class OdeSolverFactory;

class OdeInstanceFactoryManager{
public:
	static OdeInstanceFactoryManager* getInstance(){
		return &instance;
	}
	void add(OdeInstanceFactory* f);
	void remove(OdeInstanceFactory* f);

private:
	static OdeInstanceFactoryManager instance;

	std::set<OdeInstanceFactory*> instance_factories;
};

template<class AuxFactory>
class AuxFactoryManager{
private:
	typedef std::multimap<const OdeInstanceFactory*, AuxFactory*> inst_to_aux_map;
public:
	static AuxFactoryManager* getInstance(){
		return &instance;
	}
	void add(AuxFactory* xfact){
		// TODO: think about this const
		aux_map.insert(std::make_pair(xfact->getCorrespondingInstanceFactory(), xfact));
	}
	void remove(AuxFactory* f){
		const OdeInstanceFactory* ifact = f->getCorrespondingInstanceFactory();
		typename inst_to_aux_map::iterator ilow = aux_map.lower_bound(ifact);
		typename inst_to_aux_map::iterator ihi = aux_map.upper_bound(ifact);
			assert(ilow!=aux_map.end());

		// TODO: Написать:
		// 1 Как искать через std::find в multimap'е
		// 2 вложенный класс внутри шаблонного (AuxFactory)
		// 3 статические поля в шаблонных классах (там же)
		typename inst_to_aux_map::iterator found = std::find(ilow, ihi, std::pair<const OdeInstanceFactory *const, AuxFactory*>(ifact, f));
			assert(found != aux_map.end());
		aux_map.erase(found);
	}

	class SupportedAuxIterator: public std::iterator<std::input_iterator_tag, OdeInstanceFactory*>{
		friend class AuxFactoryManager;
	public:
		AuxFactory* operator*() const {
			return iterator->second;
		}
		AuxFactory* operator->() const {
			return iterator->second;
		}
		SupportedAuxIterator& operator++(){				// prefix
			++iterator;
			return *this;
		}
		SupportedAuxIterator operator++(int){			// postfix
			SupportedAuxIterator tmp = *this;
			++iterator;
			return tmp;
		}
		bool operator==(const SupportedAuxIterator& rhs) const {
			return iterator == rhs.iterator;
		}
		bool operator!=(const SupportedAuxIterator& rhs) const {
			return iterator != rhs.iterator;
		}

	private:		// friend interface
		SupportedAuxIterator(typename AuxFactoryManager::inst_to_aux_map::const_iterator it)
			:iterator(it)
		{
		}

	private:		// implementation
		typename AuxFactoryManager::inst_to_aux_map::const_iterator iterator;
	};

	std::pair< SupportedAuxIterator, SupportedAuxIterator > getSupportedSolvers(const OdeInstanceFactory* f) const {
		auto range_to_return = aux_map.equal_range(f);
		SupportedAuxIterator begin( range_to_return.first );
		SupportedAuxIterator end( range_to_return.second );
		return std::make_pair(begin, end);
	}

private:
	static AuxFactoryManager instance;
	inst_to_aux_map aux_map;
};

template<class AuxFactory>
AuxFactoryManager<AuxFactory> AuxFactoryManager<AuxFactory>::instance;

typedef AuxFactoryManager<OdeSolverFactory> OdeSolverFactoryManager;

#endif /* CORE_FACTORY_MANAGERS_H_ */
