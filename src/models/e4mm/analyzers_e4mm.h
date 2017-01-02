#ifndef ANALYZERS_E4MM_H_
#define ANALYZERS_E4MM_H_

#include "model_e4mm.h"

#include <gui_factories.h>

#include <gtkmm/entry.h>
#include <gtkmm/treeview.h>
#include <gtkmm/liststore.h>
#include <gtkmm/builder.h>

class E4mmChartAnalyzer: public EXChartAnalyzer {
public:
	typedef E4mmConfig Config;
	typedef E4mmState State;
private:
	std::vector<bool> chart_special_flags;
	Gtk::Button btn_add_special;

public:
	E4mmChartAnalyzer(const OdeConfig* config);
	virtual void processState(const OdeState* state, const OdeState* d_state, double time);
	virtual ~E4mmChartAnalyzer();

	static std::string getDisplayName(){
		return "customized for E4mm EXChartAnalyzer";
	}

	virtual void addChart(MessageChart* chart);

private:
	virtual ::google::protobuf::Message* new_state(){ return new E4mmState();}
};

REGISTER_ANALYZER_WIDGET_CLASS(E4mmChartAnalyzer)

#endif /* ANALYZERS_E4MM_H_ */
