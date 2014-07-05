/*
 * gui_e1.cpp
 *
 *  Created on: Jul 4, 2014
 *      Author: dimalit
 */

#include "gui_e1.h"

#include <gtkmm/builder.h>
#include <gtkmm/frame.h>

#define UI_FILE_CONF "e1_config.glade"

E1ConfigWidget::E1ConfigWidget(const E1Config* cfg){
	if(cfg)
		this->config = new E1Config(*cfg);
	else
		this->config = new E1Config();

	Glib::RefPtr<Gtk::Builder> b = Gtk::Builder::create_from_file(UI_FILE_CONF);

	Gtk::Widget* root;
	b->get_widget("root", root);

	b->get_widget("entry_m", entry_m);
	b->get_widget("entry_ksi", entry_ksi);

	b->get_widget("check_rand", check_rand);
	b->get_widget("check_linear", check_linear);

	this->add(*root);

	config_to_widget();
}

void E1ConfigWidget::widget_to_config(){
	config->g_m = atoi(entry_m->get_text().c_str());
}
void E1ConfigWidget::config_to_widget(){
	char buf[10];
	sprintf(buf, "%d", config->g_m);
	entry_m->set_text(buf);
}

void E1ConfigWidget::loadConfig(const OdeConfig* cfg){
	const E1Config* ecfg = dynamic_cast<const E1Config*>(cfg);
		assert(ecfg);
	delete this->config;
	this->config = new E1Config(*ecfg);
	config_to_widget();
}

const OdeConfig* E1ConfigWidget::getConfig() {
	widget_to_config();
	return config;
}
