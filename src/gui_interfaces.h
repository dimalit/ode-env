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
public:
	virtual void loadConfig(const OdeConfig* cfg) = 0;
	virtual const OdeConfig* getConfig() = 0;
};

class OdeStateWidget: public Gtk::Frame{

};

class OdeSolverConfigWidget: public Gtk::Frame{

};

#endif /* GUI_INTERFACES_H_ */
