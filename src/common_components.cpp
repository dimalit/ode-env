/*
 * common_components.cpp
 *
 *  Created on: Dec 26, 2016
 *      Author: dimalit
 */

#include "common_components.h"

#include <sstream>
#include <iostream>

#define UI_FILE_CHARTS_ADD "charts_add.glade"
#define UI_FILE_EX_PETSC_SOLVER "ex_petsc_solver.glade"

std::string trim(const std::string& str,
                 const std::string& whitespace = " \t")
{
    const auto strBegin = str.find_first_not_of(whitespace);
    if (strBegin == std::string::npos)
        return ""; // no content

    const auto strEnd = str.find_last_not_of(whitespace);
    const auto strRange = strEnd - strBegin + 1;

    return str.substr(strBegin, strRange);
}

void parse_with_prefix(google::protobuf::Message& msg, FILE* fp){
	int size;
	int ok = fread(&size, sizeof(size), 1, fp);
	assert(ok == 1);

	//TODO:without buffer cannot read later bytes
	char *buf = (char*)malloc(size);
	ok = fread(buf, 1, size, fp);
		assert(ok==size);
	msg.ParseFromArray(buf, size);
	free(buf);
}

EXPetscSolverConfig::EXPetscSolverConfig(){
	set_atol(1e-6);
	set_rtol(1e-6);
	set_init_step(0.01);
	set_n_cores(1);
}

AbstractConfigWidget::AbstractConfigWidget(Message *msg){
	this->data = msg;

	grid.set_column_spacing(4);
	grid.set_row_spacing(4);
	grid.set_margin_left(4);
	grid.set_margin_top(4);
	grid.set_margin_right(4);
	grid.set_margin_bottom(4);
	this->add(grid);

	button_apply.set_label("Apply");
	button_apply.set_hexpand(true);
	button_apply.signal_clicked().connect(sigc::mem_fun(*this, &AbstractConfigWidget::on_apply_cb));

	construct_ui();
	config_to_widget();
}

void AbstractConfigWidget::construct_ui(){

	// 1 clear
	std::vector<Widget*> v = grid.get_children();
	for(auto it=v.begin(); it!=v.end(); ++it){
		Gtk::Widget* w = *it;
		grid.remove(*w);
		if(w != &button_apply)
			delete w;
	}// for

	// 2 add
	grid.attach(button_apply, 0, 0, 2, 1);

	if(!data)
		return;

	const Descriptor* d = data->GetDescriptor();

	for(int i=0; i<d->field_count(); i++){
		const FieldDescriptor* fd = d->field(d->field_count()-i-1);
		const string& fname = fd->name();

		Gtk::Label *label = Gtk::manage(new Gtk::Label);
		label->set_text(fname);

		Gtk::Entry *entry = Gtk::manage(new Gtk::Entry);
		entry->set_hexpand(true);

		grid.insert_row(0);
		grid.attach(*label, 0, 0, 1, 1);
		grid.attach(*entry, 1, 0, 1, 1);

		entry_map[fname] = entry;
		entry->signal_changed().connect(sigc::mem_fun(*this, &AbstractConfigWidget::edit_anything_cb));
	}// for fields
}

void AbstractConfigWidget::setData(Message* msg){
	data = msg;
	construct_ui();
	config_to_widget();
}

const Message* AbstractConfigWidget::getData() const{
	widget_to_config();
	return data;
}

void AbstractConfigWidget::widget_to_config() const{
	if(data == NULL)
		return;

	const Descriptor* desc = data->GetDescriptor();
	const Reflection* refl = data->GetReflection();

	for(auto i=entry_map.begin(); i!=entry_map.end(); ++i){
		const string& var = i->first;
		Gtk::Entry* entry = i->second;

		string val = entry->get_text();

		const FieldDescriptor* fd = desc->FindFieldByName(var);
		FieldDescriptor::Type type = fd->type();

		switch(type){
			case FieldDescriptor::Type::TYPE_DOUBLE:
				refl->SetDouble(data, fd, atof(val.c_str()));
				break;
			case FieldDescriptor::Type::TYPE_FLOAT:
				refl->SetFloat(data, fd, (float)atof(val.c_str()));
				break;
			case FieldDescriptor::Type::TYPE_INT32:
				refl->SetInt32(data, fd, atoi(val.c_str()));
				break;
			case FieldDescriptor::Type::TYPE_UINT32:
				refl->SetUInt32(data, fd, atoi(val.c_str()));
				break;
			case FieldDescriptor::Type::TYPE_STRING:
				refl->SetString(data, fd, val);
				break;
			default:
				assert(false);
		}

	}// for

	button_apply.set_sensitive(false);
}
void AbstractConfigWidget::config_to_widget() const {
	if(data == NULL)
		return;

	const Descriptor* desc = data->GetDescriptor();
	const Reflection* refl = data->GetReflection();

	for(auto i=entry_map.begin(); i!=entry_map.end(); ++i){
		const string& var = i->first;
		Gtk::Entry* entry = i->second;

		std::ostringstream res;

		const FieldDescriptor* fd = desc->FindFieldByName(var);
		FieldDescriptor::Type type = fd->type();

		switch(type){
			case FieldDescriptor::Type::TYPE_DOUBLE:
				res << refl->GetDouble(*data, fd);
				break;
			case FieldDescriptor::Type::TYPE_FLOAT:
				res << refl->GetFloat(*data, fd);
				break;
			case FieldDescriptor::Type::TYPE_INT32:
				res << refl->GetInt32(*data, fd);
				break;
			case FieldDescriptor::Type::TYPE_UINT32:
				res << refl->GetUInt32(*data, fd);
				break;
			case FieldDescriptor::Type::TYPE_STRING:
				res << refl->GetString(*data, fd);
				break;
			default:
				assert(false);
		}

		entry->set_text(res.str());
	}// for

	button_apply.set_sensitive(false);
}

void AbstractConfigWidget::edit_anything_cb(){
	button_apply.set_sensitive(true);
}

void AbstractConfigWidget::on_apply_cb(){
	widget_to_config();
	m_signal_changed.emit();
}

///////////////////////////////////////////////////////////////////////////////
EXConfigWidget::EXConfigWidget(const OdeConfig* cfg){
	this->add(cfg_widget);

	if(!cfg)
		this->config = NULL;
	else
		this->config = cfg->clone();

	cfg_widget.setData(dynamic_cast<Message*>(config));
	cfg_widget.signal_changed().connect(sigc::mem_fun(*this, &EXConfigWidget::on_changed));
}

void EXConfigWidget::on_changed(){
	m_signal_changed.emit();
}

void EXConfigWidget::loadConfig(const OdeConfig* cfg){
	if(!cfg)
		this->config = NULL;
	else
		this->config = cfg->clone();

	Message* msg = dynamic_cast<Message*>(config);
		assert(msg);
	cfg_widget.setData(msg);
}

const OdeConfig* EXConfigWidget::getConfig() const{
	return this->config;
}

///////////////////////////////////////////////////////////////////////////////


EXPetscSolverConfigWidget::EXPetscSolverConfigWidget(const EXPetscSolverConfig* config){
	if(config)
		this->config = new EXPetscSolverConfig(*config);
	else
		this->config = new EXPetscSolverConfig();

	Glib::RefPtr<Gtk::Builder> b = Gtk::Builder::create_from_file(UI_FILE_EX_PETSC_SOLVER);

	Gtk::Widget* root;
	b->get_widget("root", root);

	b->get_widget("entry_atol", entry_atol);
	b->get_widget("entry_rtol", entry_rtol);
	b->get_widget("entry_step", entry_step);
	adj_n_cores = Glib::RefPtr<Gtk::Adjustment>::cast_dynamic(b->get_object("adj_n_cores"));
	adj_n_cores->set_value(4);

	this->add(*root);

	config_to_widget();
}

const OdeSolverConfig* EXPetscSolverConfigWidget::getConfig(){
	widget_to_config();
	return config;
}

void EXPetscSolverConfigWidget::loadConfig(const OdeSolverConfig* config){
	const EXPetscSolverConfig* econfig = dynamic_cast<const EXPetscSolverConfig*>(config);
		assert(econfig);
	delete this->config;
	this->config = new EXPetscSolverConfig(*econfig);
	config_to_widget();
}

void EXPetscSolverConfigWidget::widget_to_config(){
	config->set_init_step(atof(entry_step->get_text().c_str()));
	config->set_atol(atof(entry_atol->get_text().c_str()));
	config->set_rtol(atof(entry_rtol->get_text().c_str()));
	config->set_n_cores(adj_n_cores->get_value());
}

void EXPetscSolverConfigWidget::config_to_widget(){
	std::ostringstream buf;
	buf << config->init_step();
	entry_step->set_text(buf.str());

	buf.str("");
	buf << config->atol();
	entry_atol->set_text(buf.str());

	buf.str("");
	buf << config->rtol();
	entry_rtol->set_text(buf.str());
	adj_n_cores->set_value(config->n_cores());
}

///////////////////////////////////////////////////////////////////////////////

ChartAddDialog::ChartAddDialog(const google::protobuf::Message* msg, bool show_derivatives){
	assert(msg);
	this->msg = msg;
	this->set_modal(true);

	Glib::RefPtr<Gtk::Builder> b = Gtk::Builder::create_from_file(UI_FILE_CHARTS_ADD);

	Gtk::Widget* root;
	b->get_widget("root", root);

	b->get_widget("treeview1", treeview1);
	b->get_widget("treeview2", treeview2);
	b->get_widget("treeview3", treeview3);
	b->get_widget("btn_plus", btn_plus);
	b->get_widget("btn_minus", btn_minus);
	b->get_widget("btn_ok", btn_ok);
	b->get_widget("btn_cancel", btn_cancel);
	b->get_widget("check_polar", check_polar);
	b->get_widget("check_parametric", check_parametric);

	polar = check_polar->get_active();
	parametric = check_parametric->get_active();

	store1 = Glib::RefPtr<Gtk::ListStore>::cast_dynamic(b->get_object("liststore1"));
	store2 = Glib::RefPtr<Gtk::ListStore>::cast_dynamic(b->get_object("liststore2"));
	store3 = Glib::RefPtr<Gtk::ListStore>::cast_dynamic(b->get_object("liststore3"));

	Gtk::CellRendererToggle *cr;
	cr = dynamic_cast<Gtk::CellRendererToggle*>(treeview1->get_column(0)->get_first_cell());
		cr->signal_toggled().connect(sigc::bind(sigc::mem_fun(*this, &ChartAddDialog::on_use_clicked), store1));
	cr = dynamic_cast<Gtk::CellRendererToggle*>(treeview1->get_column(2)->get_first_cell());
		cr->signal_toggled().connect(sigc::bind(sigc::mem_fun(*this, &ChartAddDialog::on_x_clicked), store1));
	cr = dynamic_cast<Gtk::CellRendererToggle*>(treeview2->get_column(0)->get_first_cell());
		cr->signal_toggled().connect(sigc::bind(sigc::mem_fun(*this, &ChartAddDialog::on_use_clicked), store2));
	cr = dynamic_cast<Gtk::CellRendererToggle*>(treeview2->get_column(2)->get_first_cell());
		cr->signal_toggled().connect(sigc::bind(sigc::mem_fun(*this, &ChartAddDialog::on_x_clicked), store2));
	cr = dynamic_cast<Gtk::CellRendererToggle*>(treeview3->get_column(0)->get_first_cell());
		cr->signal_toggled().connect(sigc::bind(sigc::mem_fun(*this, &ChartAddDialog::on_use_clicked), store3));
	cr = dynamic_cast<Gtk::CellRendererToggle*>(treeview3->get_column(2)->get_first_cell());
		cr->signal_toggled().connect(sigc::bind(sigc::mem_fun(*this, &ChartAddDialog::on_x_clicked), store3));

	Gtk::CellRendererText *crt = dynamic_cast<Gtk::CellRendererText*>(treeview3->get_column(1)->get_first_cell());
		crt->signal_edited().connect(sigc::mem_fun(*this, &ChartAddDialog::on_expr_edited));

	btn_plus->signal_clicked().connect(sigc::mem_fun(this, &ChartAddDialog::on_plus_clicked));
	btn_minus->signal_clicked().connect(sigc::mem_fun(this, &ChartAddDialog::on_minus_clicked));

	btn_ok->signal_clicked().connect(sigc::mem_fun(this, &ChartAddDialog::on_ok_clicked));
	btn_cancel->signal_clicked().connect(sigc::mem_fun(this, &ChartAddDialog::on_cancel_clicked));

	this->add(*root);

	// now add state's variables to the table
	store1->clear();
	store2->clear();
	store3->clear();
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

			if(show_derivatives){
				it = store2->append();
				it->set_value(0, false);
				it->set_value(1, Glib::ustring(fd->name()));
				it->set_value(2, false);
			}// if
		}
		// add message
		else if(fd->type() == FieldDescriptor::TYPE_MESSAGE || fd->type() == FieldDescriptor::TYPE_GROUP){
			//const Message& m2 = fd->is_repeated() ? refl->GetRepeatedMessage(*msg, fd, 0) : refl->GetMessage(*msg, fd);
			//const Descriptor* d2 = m2.GetDescriptor();
			const Descriptor* d2 = fd->message_type();
			for(int i=0; i<d2->field_count(); i++){
					const FieldDescriptor* fd2 = d2->field(i);
					if(fd2->type() != FieldDescriptor::TYPE_DOUBLE)
						continue;

					Gtk::ListStore::iterator it = store1->append();
					it->set_value(0, false);
					it->set_value(1, Glib::ustring(fd->name())+"."+Glib::ustring(fd2->name()));
					it->set_value(2, false);

					if(show_derivatives){
						it = store2->append();
						it->set_value(0, false);
						it->set_value(1, Glib::ustring(fd->name())+"."+Glib::ustring(fd2->name()));
						it->set_value(2, false);
					}// if
			}// for
		}// else
	}// for
}
void ChartAddDialog::on_ok_clicked(){
	vars.clear();
	x_axis_var = "";
	polar = check_polar->get_active();
	parametric = check_parametric->get_active();

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

	// list3
	children = store3->children();
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

	//parent->addChart(msg, vars, x_axis_var, NULL, polar);
	//this->hide();
	//delete this;

	signal_ok.emit();
}
void ChartAddDialog::on_cancel_clicked(){
	//this->hide();
	//delete this;		// XXX: can we do so?
	signal_cancel.emit();
}
void ChartAddDialog::on_use_clicked(const Glib::ustring& path, Glib::RefPtr<Gtk::ListStore> store){
	Gtk::ListStore::iterator it = store->get_iter(path);
	bool val;
	it->get_value(0, val);
	it->set_value(0, !val);
}
void ChartAddDialog::on_x_clicked(const Glib::ustring& path, Glib::RefPtr<Gtk::ListStore> store){
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
	for(Gtk::ListStore::iterator it = store3->children().begin(); it != store3->children().end(); ++it){
		it->set_value(2, false);
	}

	// check
	cur->set_value(2, true);
}
void ChartAddDialog::on_expr_edited(const Glib::ustring& path,  const Glib::ustring& new_text){
	Gtk::ListStore::iterator it = store3->get_iter(path);
	it->set_value(1, new_text);
}
void ChartAddDialog::on_plus_clicked(){
	store3->append();
}
void ChartAddDialog::on_minus_clicked(){
	if(treeview3->get_selection()->count_selected_rows()==1){
		Gtk::ListStore::iterator it = treeview3->get_selection()->get_selected();
		store3->erase(it);
	}
}
///////////////////////////////////////////////////////////////////////////////

MessageChart::MessageChart(const std::vector<std::string>& vars, const std::string& x_var, Gtk::Container* parent){
	last_time = 0;
	last_msg = NULL;
	last_d_msg = NULL;

	this->vars = vars;

	int socket_id = 0;
	if(parent){
		Gtk::Socket* socket = Gtk::manage(new Gtk::Socket());
		parent->add(*socket);
		parent->show_all();
		socket_id = socket->get_id();
		std::cerr << "Socket: " << std::hex << socket_id << "\n";
	}
	gnuplot = new Gnuplot(socket_id);

	if(!x_var.empty())
		gnuplot->setXAxisVar(x_var);

	for(int i=0; i<vars.size(); i++)
		gnuplot->addVar(vars[i]);

	// set dots for arrays
	if(vars[0].find('.')!=std::string::npos)
		gnuplot->setStyle(Gnuplot::STYLE_POINTS);

	// add widgets //
	Gtk::Button* del = new Gtk::Button("del");
	Gtk::Button* writeback = new Gtk::Button("auto");
	Gtk::Button* restore = new Gtk::Button("fixed");
	Gtk::Button* save = new Gtk::Button("save");

	Gtk::HBox* hbox = new Gtk::HBox();
	hbox->pack_end(*Gtk::manage(del), false, false);
	hbox->pack_end(*Gtk::manage(writeback), false, false);
	hbox->pack_end(*Gtk::manage(restore), false, false);
	hbox->pack_end(*Gtk::manage(save), false, false);
	hbox->pack_start(label, true, true);

	del->signal_clicked().connect(sigc::mem_fun(*this, &MessageChart::on_del_chart_clicked));
	writeback->signal_clicked().connect(sigc::mem_fun(*this, &MessageChart::on_writeback_clicked));
	restore->signal_clicked().connect(sigc::mem_fun(*this, &MessageChart::on_restore_clicked));
	save->signal_clicked().connect(sigc::mem_fun(*this, &MessageChart::on_save_clicked));
	this->add(*Gtk::manage(hbox));

	update_title();
}

void MessageChart::update_title(){
	std::ostringstream full_title;

	for(int i=0; i<vars.size(); i++){
		full_title << vars[i] << ' ';
	}// for
	if(!gnuplot->getXAxisVar().empty()){
		full_title << "| " << gnuplot->getXAxisVar() << " ";
	}

	gnuplot->setTitle(trim(full_title.str()));
	label.set_text(full_title.str());
}

void MessageChart::processMessage(const google::protobuf::Message* msg, const google::protobuf::Message* d_msg, double time){
	assert(msg);

	gnuplot->processState(msg, d_msg, time);

	delete last_msg;
	last_msg = msg->New();
	last_msg->CopyFrom(*msg);
	if(d_msg){
		delete last_d_msg;
		last_d_msg = d_msg->New();
		last_d_msg->CopyFrom(*d_msg);
	}
}

void MessageChart::on_save_clicked(){
	assert(last_msg);
	std::string file = gnuplot->getTitle()+".csv";
	gnuplot->saveToCsv(file, last_msg, last_d_msg, last_time);
}

void MessageChart::on_del_chart_clicked(){
	this->signal_closed.emit();
}

void MessageChart::on_writeback_clicked(){
	gnuplot->writeback();
}

void MessageChart::on_restore_clicked(){
	gnuplot->restore();
}

///////////////////////////////////////////////////////////////////////////////

EXChartAnalyzer::EXChartAnalyzer(const OdeConfig* config) {
	this->config = config->clone();

	states_count = 0;

	btn_add.set_label("Add chart");
	vbox.pack_start(btn_add);
	btn_add.signal_clicked().connect(sigc::mem_fun(*this, &EXChartAnalyzer::on_add_clicked));

	btn_reset.set_label("Reset");
	vbox.pack_start(btn_reset);
	btn_reset.signal_clicked().connect(sigc::mem_fun(*this, &EXChartAnalyzer::reset));

	this->add(vbox);

	this->show_all();
}

EXChartAnalyzer::~EXChartAnalyzer() {
	for(int i=0; i<charts.size(); i++)
		delete charts[i];
	delete config;
}

int EXChartAnalyzer::getStatesCount(){
	return states_count;
}

void EXChartAnalyzer::reset(){
	states_count = 0;
	for(int i=0; i<charts.size(); i++)
		charts[i]->reset();
}

void EXChartAnalyzer::processState(const OdeState* state, const OdeState* d_state, double time){
	const google::protobuf::Message* msg = dynamic_cast<const google::protobuf::Message*>(state);
		assert(msg);
	const google::protobuf::Message* d_msg = dynamic_cast<const google::protobuf::Message*>(d_state);
		assert(d_msg);

	for(int i=0; i<charts.size(); i++)
		charts[i]->processMessage(msg, d_msg, time);
}

void EXChartAnalyzer::addChart(MessageChart* chart){
	charts.push_back(chart);
	vbox.pack_end(*chart, false, false, 1);
	this->show_all();
}

void EXChartAnalyzer::on_add_clicked(){
	const google::protobuf::Message* msg = new_state();
	ChartAddDialog* dialog = new ChartAddDialog(msg, true);
	dialog->signal_cancel.connect(
			sigc::bind(
					sigc::mem_fun(*this, &EXChartAnalyzer::on_dialog_cancel),
					dialog
			)
	);
	dialog->signal_ok.connect(
			sigc::bind(
					sigc::mem_fun(*this, &EXChartAnalyzer::on_dialog_add_ok),
					dialog
			)
	);

	dialog->show_all();
}

void EXChartAnalyzer::on_dialog_add_ok(ChartAddDialog* dialog){
	MessageChart* chart = new MessageChart(dialog->vars, dialog->x_axis_var, NULL);
	chart->setPolar(dialog->polar);
	chart->setParametric(dialog->parametric);
	addChart(chart);
	delete dialog;
}

void EXChartAnalyzer::on_dialog_cancel(ChartAddDialog* dialog){
	delete dialog;
}

void EXChartAnalyzer::on_del_chart_clicked(const MessageChart* chart){
	assert(chart);
	for(int i=0; i<charts.size(); i++){
		if(charts[i] == chart){
			delete charts[i];
			charts.erase(charts.begin()+i);
			return;
		}// if
	}// for
}
