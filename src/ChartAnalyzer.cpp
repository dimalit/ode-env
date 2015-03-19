/*
 * ChartAnalyzer.cpp
 *
 *  Created on: Jan 29, 2015
 *      Author: dimalit
 */

#include "ChartAnalyzer.h"
#include <google/protobuf/message.h>

#include <gtkmm/builder.h>
#include <gtkmm/window.h>
#include <gtkmm/treeview.h>
#include <gtkmm/liststore.h>
#include <gtkmm/button.h>
#include <gtkmm/messagedialog.h>

#include <iostream>
#include <cstdio>

using namespace google::protobuf;

#define UI_FILE "charts_add.glade"

class ChartAddDialog: public Gtk::Window{
private:
	Gtk::TreeView *treeview1, *treeview2;		// for vars and derivatives
	Glib::RefPtr<Gtk::ListStore> store1, store2;
	Gtk::Button *btn_ok, *btn_cancel;
	ChartAnalyzer* parent;

public:
	ChartAddDialog(ChartAnalyzer* parent, const OdeState* state){
		assert(parent && state);
		this->parent = parent;
		this->set_modal(true);

		Glib::RefPtr<Gtk::Builder> b = Gtk::Builder::create_from_file(UI_FILE);

		Gtk::Widget* root;
		b->get_widget("root", root);

		b->get_widget("treeview1", treeview1);
		b->get_widget("treeview2", treeview2);
		b->get_widget("btn_ok", btn_ok);
		b->get_widget("btn_cancel", btn_cancel);
		store1 = Glib::RefPtr<Gtk::ListStore>::cast_dynamic(b->get_object("liststore1"));
		store2 = Glib::RefPtr<Gtk::ListStore>::cast_dynamic(b->get_object("liststore2"));

		Gtk::CellRendererToggle *cr;
		cr = dynamic_cast<Gtk::CellRendererToggle*>(treeview1->get_column(0)->get_first_cell());
			cr->signal_toggled().connect(sigc::bind(sigc::mem_fun(*this, &ChartAddDialog::on_use_clicked), store1));
		cr = dynamic_cast<Gtk::CellRendererToggle*>(treeview1->get_column(2)->get_first_cell());
			cr->signal_toggled().connect(sigc::bind(sigc::mem_fun(*this, &ChartAddDialog::on_x_clicked), store1));
		cr = dynamic_cast<Gtk::CellRendererToggle*>(treeview2->get_column(0)->get_first_cell());
			cr->signal_toggled().connect(sigc::bind(sigc::mem_fun(*this, &ChartAddDialog::on_use_clicked), store2));
		cr = dynamic_cast<Gtk::CellRendererToggle*>(treeview2->get_column(2)->get_first_cell());
			cr->signal_toggled().connect(sigc::bind(sigc::mem_fun(*this, &ChartAddDialog::on_x_clicked), store2));

		btn_ok->signal_clicked().connect(sigc::mem_fun(this, &ChartAddDialog::on_ok_clicked));
		btn_cancel->signal_clicked().connect(sigc::mem_fun(this, &ChartAddDialog::on_cancel_clicked));

		this->add(*root);

		// now add state's variables to the table
		store1->clear();
		store2->clear();
		const google::protobuf::Message* msg = dynamic_cast<const google::protobuf::Message*>(state);
		const Descriptor* desc = msg->GetDescriptor();
		const Reflection* refl = msg->GetReflection();

		for(int i=0; i<desc->field_count(); i++){
			const FieldDescriptor* fd = desc->field(i);
			// add simple
			if(fd->type() == FieldDescriptor::TYPE_DOUBLE){
				Gtk::ListStore::iterator it = store1->append();
				it->set_value(0, false);
				it->set_value(1, Glib::ustring(fd->name()));
				it->set_value(2, false);

				it = store2->append();
				it->set_value(0, false);
				it->set_value(1, Glib::ustring(fd->name()));
				it->set_value(2, false);
			}
			// add message
			else if(fd->type() == FieldDescriptor::TYPE_MESSAGE || fd->type() == FieldDescriptor::TYPE_GROUP){
				const Message& m2 = fd->is_repeated() ? refl->GetRepeatedMessage(*msg, fd, 0) : refl->GetMessage(*msg, fd);

				const Descriptor* d2 = m2.GetDescriptor();
				for(int i=0; i<d2->field_count(); i++){
						const FieldDescriptor* fd2 = d2->field(i);
						if(fd2->type() != FieldDescriptor::TYPE_DOUBLE)
							continue;

						Gtk::ListStore::iterator it = store1->append();
						it->set_value(0, false);
						it->set_value(1, Glib::ustring(fd->name())+"."+Glib::ustring(fd2->name()));
						it->set_value(2, false);

						it = store2->append();
						it->set_value(0, false);
						it->set_value(1, Glib::ustring(fd->name())+"."+Glib::ustring(fd2->name()));
						it->set_value(2, false);
				}// for
			}// else
		}// for
	}

private:
	void on_ok_clicked(){
		std::vector<std::string> vars;
		std::string x_axis_var;

		// check all repeated or all non-repeated
		bool has_repeated = false, has_non_repeated = false;

		// list1
		Gtk::ListStore::Children children = store1->children();
		for(Gtk::ListStore::const_iterator i = children.begin(); i!=children.end(); ++i){
			// parse added vars
			bool use;
			i->get_value(0, use);
			if(use){
				Glib::ustring us;
				i->get_value(1, us);
				vars.push_back(us.raw());

				if(us.raw().find('.') == std::string::npos)
					has_non_repeated = true;
				else
					has_repeated = true;
			}
			// parse x var
			bool as_x;
			i->get_value(2, as_x);
			if(as_x){
				Glib::ustring us;
				i->get_value(1, us);
				x_axis_var = us.raw();
			}// if as x
		}// for

		// list2
		children = store2->children();
		for(Gtk::ListStore::const_iterator i = children.begin(); i!=children.end(); ++i){
			// parse added vars
			bool use;
			i->get_value(0, use);
			if(use){
				Glib::ustring us;
				i->get_value(1, us);
				vars.push_back(us.raw()+'\'');

				if(us.raw().find('.') == std::string::npos)
					has_non_repeated = true;
				else
					has_repeated = true;
			}
			// parse x var
			bool as_x;
			i->get_value(2, as_x);
			if(as_x){
				Glib::ustring us;
				i->get_value(1, us);
				x_axis_var = us.raw()+'\'';
			}// if as x
		}// for

		if(x_axis_var.find('.') != std::string::npos)
			has_repeated = true;
		else if(!x_axis_var.empty())
			has_non_repeated = true;

		// must have at least something
		if(vars.size() == 0){
			Gtk::MessageDialog dlg("ERROR: Please select vars to add to the chart!", false, Gtk::MessageType::MESSAGE_ERROR, Gtk::ButtonsType::BUTTONS_OK, true);
			dlg.run();
			return;
		}

		// all must be either repeated or non-repeated!
		if(has_repeated && has_non_repeated){
			Gtk::MessageDialog dlg("ERROR: Vars must be either all repeated or all non-repeated!", false, Gtk::MessageType::MESSAGE_ERROR, Gtk::ButtonsType::BUTTONS_OK, true);
			dlg.run();
			return;
		}

		parent->addChart(vars, x_axis_var);

		this->hide();
		delete this;
	}
	void on_cancel_clicked(){
		this->hide();
		delete this;		// XXX: can we do so?
	}
	void on_use_clicked(const Glib::ustring& path, Glib::RefPtr<Gtk::ListStore> store){
		Gtk::ListStore::iterator it = store->get_iter(path);
		bool val;
		it->get_value(0, val);
		it->set_value(0, !val);
	}

	void on_x_clicked(const Glib::ustring& path, Glib::RefPtr<Gtk::ListStore> store){
		// uncheck if checked
		Gtk::ListStore::iterator cur = store->get_iter(path);
		bool checked; cur->get_value(2, checked);
		if(checked){
			cur->set_value(2, false);
			return;
		}

		// clear all
		for(Gtk::ListStore::iterator it = store1->children().begin(); it != store1->children().end(); ++it){
			it->set_value(2, false);
		}
		for(Gtk::ListStore::iterator it = store2->children().begin(); it != store2->children().end(); ++it){
			it->set_value(2, false);
		}

		// check
		cur->set_value(2, true);
	}
};

ChartAnalyzer::ChartAnalyzer(const OdeConfig* config) {
	states_count = 0;
	last_state = NULL;

	btn_add.set_label("Add chart");
	vbox.pack_end(btn_add);
	btn_add.signal_clicked().connect(sigc::mem_fun(*this, &ChartAnalyzer::on_add_clicked));

	btn_reset.set_label("Reset");
	vbox.pack_end(btn_reset);
	btn_reset.signal_clicked().connect(sigc::mem_fun(*this, &ChartAnalyzer::reset));

	this->add(vbox);
}

ChartAnalyzer::~ChartAnalyzer() {
	for(int i=0; i<plots.size(); i++)
		delete plots[i];
}

int ChartAnalyzer::getStatesCount(){
	return states_count;
}

void ChartAnalyzer::reset(){
	states_count = 0;
	for(int i=0; i<plots.size(); i++)
		plots[i]->reset();
}

void ChartAnalyzer::processState(const OdeState* state, const OdeState* d_state, double time){
	this->last_state = state;
	this->last_d_state = d_state;
	last_time = time;

	const google::protobuf::Message* msg = dynamic_cast<const google::protobuf::Message*>(state);
		assert(msg);
	const google::protobuf::Message* d_msg = dynamic_cast<const google::protobuf::Message*>(d_state);
		assert(d_msg);

	for(int i=0; i<plots.size(); i++)
		plots[i]->processState(msg, d_msg, time);
}

void ChartAnalyzer::on_save_clicked(Gnuplot* ptr){
	assert(last_state);
	assert(last_d_state);

	std::string file = ptr->getTitle()+".png";

	const google::protobuf::Message* msg = dynamic_cast<const google::protobuf::Message*>(last_state);
		assert(msg);
	const google::protobuf::Message* d_msg = dynamic_cast<const google::protobuf::Message*>(last_d_state);
		assert(d_msg);

	ptr->processToFile(file, msg, d_msg, last_time);
}

void ChartAnalyzer::on_add_clicked(){
	assert(last_state);
	(new ChartAddDialog(this, last_state))->show_all();
}

void ChartAnalyzer::addChart(std::vector<std::string> vars, std::string x_axis_var){
	std::ostringstream full_title;

	Gnuplot* p = new Gnuplot();
	for(int i=0; i<vars.size(); i++){
		p->addVar(vars[i]);
		full_title << vars[i] << ' ';
	}// for
	if(!x_axis_var.empty()){
		p->setXAxisVar(x_axis_var);
		full_title << "| " << x_axis_var << " ";
	}

	// set dots if plot is scatter
	if(!x_axis_var.empty())
		p->setStyle(Gnuplot::STYLE_POINTS);

	p->setTitle(full_title.str());
	plots.push_back(p);	// XXX: not very copyable - but with no copies it will work...

	// add widget for it //
	Gtk::Button* del = new Gtk::Button("del");
	Gtk::Button* writeback = new Gtk::Button("auto");
	Gtk::Button* restore = new Gtk::Button("fixed");
	Gtk::Button* save = new Gtk::Button("save");
	Gtk::Label* l = new Gtk::Label(full_title.str());

	Gtk::HBox* hbox = new Gtk::HBox();
	hbox->pack_end(*Gtk::manage(del), false, false);
	hbox->pack_end(*Gtk::manage(writeback), false, false);
	hbox->pack_end(*Gtk::manage(restore), false, false);
	hbox->pack_end(*Gtk::manage(save), false, false);
	hbox->pack_start(*Gtk::manage(l), true, true);

	del->signal_clicked().connect(sigc::bind(sigc::mem_fun(*this, &ChartAnalyzer::on_del_chart_clicked), (Gtk::Widget*)hbox, p));
	writeback->signal_clicked().connect(sigc::bind(sigc::mem_fun(*this, &ChartAnalyzer::on_writeback_clicked), p));
	restore->signal_clicked().connect(sigc::bind(sigc::mem_fun(*this, &ChartAnalyzer::on_restore_clicked), p));
	save->signal_clicked().connect(sigc::bind(sigc::mem_fun(*this, &ChartAnalyzer::on_save_clicked), p));
	vbox.pack_start(*Gtk::manage(hbox), false, false);
	hbox->show_all();

	p->processState(dynamic_cast<const Message*>(this->last_state));
}

void ChartAnalyzer::on_del_chart_clicked(Gtk::Widget* w, const Gnuplot* ptr){
	assert(ptr);
	for(int i=0; i<plots.size(); i++){
		if(plots[i] == ptr){
			delete plots[i];
			plots.erase(plots.begin()+i);
			vbox.remove(*w);
			return;
		}// if
	}// for
}

void ChartAnalyzer::on_writeback_clicked(const Gnuplot* ptr){
	ptr->writeback();
}

void ChartAnalyzer::on_restore_clicked(const Gnuplot* ptr){
	ptr->restore();
}

