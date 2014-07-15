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
	virtual const OdeConfig* getConfig() = 0;
	sigc::signal<void> signal_changed(){
		return m_signal_changed;
	}
};

class OdeStateWidget: public Gtk::Frame{
protected:
	sigc::signal<void> m_signal_changed;
public:
	virtual void loadState(const OdeState* state) = 0;
	virtual const OdeState* getState() = 0;
	virtual void loadConfig(const OdeConfig* config) = 0;
	virtual const OdeConfig* getConfig() = 0;
	virtual void generateState() = 0;
	sigc::signal<void> signal_changed(){
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

#endif /* GUI_INTERFACES_H_ */
