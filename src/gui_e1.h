/*
 * gui_e1.h
 *
 *  Created on: Jul 4, 2014
 *      Author: dimalit
 */

#ifndef GUI_E1_H_
#define GUI_E1_H_

#include "gui_interfaces.h"

#include "model_e1.h"

#include <gtkmm/entry.h>
#include <gtkmm/checkbutton.h>

#include <cassert>

class E1ConfigWidget: public OdeConfigWidget{
private:
	E1Config* config;

	Gtk::Entry *entry_m, *entry_ksi;
	Gtk::CheckButton *check_rand, *check_linear;

public:
	E1ConfigWidget(const E1Config* config = NULL);
	// TODO: maybe clone?
	const OdeConfig* getConfig();
	void loadConfig(const OdeConfig* cfg);

private:
	void widget_to_config();
	void config_to_widget();
};

class E1PetscSolverConfigWidget: public OdeSolverConfigWidget{

};

#endif /* GUI_E1_H_ */
