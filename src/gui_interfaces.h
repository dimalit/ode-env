/*
 * abstract_gui.h
 *
 *  Created on: Dec 14, 2013
 *      Author: dimalit
 */

#ifndef GUI_INTERFACES_H_
#define GUI_INTERFACES_H_

#include "core_interfaces.h"
#include <gtkmm/frame.h>

class OdeConfigWidget: public Gtk::Frame{
protected:
	sigc::signal<void> m_signal_changed;
public:
	virtual void loadConfig(const OdeConfig* cfg) = 0;
	virtual const OdeConfig* getConfig() const = 0;
	sigc::signal<void> signal_changed(){
		return m_signal_changed;
	}
};

class OdeStateGeneratorWidget: public Gtk::Frame{
protected:
	sigc::signal<void> m_signal_changed;
public:
	virtual const OdeState* getState() = 0;
	virtual void loadConfig(const OdeConfig* config) = 0;
	virtual const OdeConfig* getConfig() = 0;
	virtual void newState(bool emit=true) = 0;
	sigc::signal<void> signal_changed() const {
		return m_signal_changed;
	}
};

class OdeSolverConfigWidget: public Gtk::Frame{
protected:
	sigc::signal<void> m_signal_changed;
public:
	virtual const OdeSolverConfig* getConfig() = 0;
	virtual void loadConfig(const OdeSolverConfig* config) = 0;
	sigc::signal<void> signal_changed(){
		return m_signal_changed;
	}
};

class OdeAnalyzerWidget: public Gtk::Frame{
public:
	virtual void loadConfig(const OdeConfig* config) = 0;
	virtual void reset() = 0;
	virtual void processState(const OdeState* state, const OdeState* d_state, double time) = 0;
	virtual int getStatesCount() = 0;
	virtual ~OdeAnalyzerWidget(){}
};

#endif /* GUI_INTERFACES_H_ */
