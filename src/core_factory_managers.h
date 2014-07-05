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

class OdeSolverFactoryManager{
private:
	typedef std::multimap<const OdeInstanceFactory*, OdeSolverFactory*> inst_to_solvers_map;
public:
	static OdeSolverFactoryManager* getInstance(){
		return &instance;
	}
	void add(OdeSolverFactory* sfact, OdeInstanceFactory* ifact);
	void remove(OdeSolverFactory* f);

	class SupportedSolversIterator: public std::iterator<std::input_iterator_tag, OdeInstanceFactory*>{
		friend class OdeSolverFactoryManager;
	public:
		OdeSolverFactory* operator*() const {
			return iterator->second;
		}
		OdeSolverFactory* operator->() const {
			return iterator->second;
		}
		SupportedSolversIterator& operator++(){				// prefix
			++iterator;
			return *this;
		}
		SupportedSolversIterator operator++(int){			// postfix
			SupportedSolversIterator tmp = *this;
			++iterator;
			return tmp;
		}
		bool operator==(const SupportedSolversIterator& rhs) const {
			return iterator == rhs.iterator;
		}
		bool operator!=(const SupportedSolversIterator& rhs) const {
			return iterator != rhs.iterator;
		}

	private:		// friend interface
		SupportedSolversIterator(OdeSolverFactoryManager::inst_to_solvers_map::const_iterator it)
			:iterator(it)
		{
		}

	private:		// implementation
		OdeSolverFactoryManager::inst_to_solvers_map::const_iterator iterator;
	};

	std::pair<SupportedSolversIterator, SupportedSolversIterator> getSupportedSolvers(const OdeInstanceFactory* f) const;
	bool isSolverSupported(const OdeInstanceFactory* ifactory, const OdeSolverFactory* sfactory) const;

private:
	static OdeSolverFactoryManager instance;
	inst_to_solvers_map solvers_map;
};

#endif /* CORE_FACTORY_MANAGERS_H_ */
