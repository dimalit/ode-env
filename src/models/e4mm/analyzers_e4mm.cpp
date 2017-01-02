#include "analyzers_e4mm.h"

#include <cassert>

E4mmChartAnalyzer::E4mmChartAnalyzer(const OdeConfig* config):EXChartAnalyzer(config) {
	btn_add_special.set_label("Add special");
	vbox.pack_end(btn_add_special);
}

E4mmChartAnalyzer::~E4mmChartAnalyzer() {

}

void E4mmChartAnalyzer::processState(const OdeState* state, const OdeState* d_state, double time){
	const google::protobuf::Message* msg = dynamic_cast<const google::protobuf::Message*>(state);
		assert(msg);
	const google::protobuf::Message* d_msg = dynamic_cast<const google::protobuf::Message*>(d_state);
		assert(d_msg);

	for(int i=0; i<charts.size(); i++){
		if(!chart_special_flags[i])
			charts[i]->processMessage(msg, d_msg, time);
//		else
//			charts[i]->processMessage(&spec_msg, NULL, time);
	}
}

void E4mmChartAnalyzer::addChart(MessageChart* chart){
	EXChartAnalyzer::addChart(chart);
	chart_special_flags.push_back(false);
}
