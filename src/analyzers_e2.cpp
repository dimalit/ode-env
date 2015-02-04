/*
 * analyzers_e2.cpp
 *
 *  Created on: Jul 16, 2014
 *      Author: dimalit
 */

#include "analyzers_e2.h"

#include "rpc.h"

#include <cassert>
#include <cstdlib>

E2FieldAnalyzer::E2FieldAnalyzer(const E2Config* config){
	states_count = 0;

	this->config = new E2Config(*config);

	int rf, wf;
	rpc_call("gnuplot", &rf, &wf);
	to_gnuplot = fdopen(wf, "wb");

	if(!to_gnuplot){
		perror(NULL);
		assert(false);
	}

	fprintf(to_gnuplot, "set terminal x11 size 400, 300 title \"%s\"\n", getDisplayName().c_str());
	fflush(to_gnuplot);
}

void E2FieldAnalyzer::loadConfig(const OdeConfig* config){
	const E2Config* econfig = dynamic_cast<const E2Config*>(config);
		assert(econfig);

	this->config = new E2Config(*econfig);
	fprintf(to_gnuplot, "clear\n");
	fflush(to_gnuplot);
}

void E2FieldAnalyzer::reset(){
	states_count = 0;
}
void E2FieldAnalyzer::processState(const OdeState* state, const OdeState* d_state, double time){
	const E2State* estate = dynamic_cast<const E2State*>(state);
		assert(estate);

	fprintf(to_gnuplot, "plot '-' with lines\n");

	// find left and right
	double left = std::numeric_limits<double>::infinity();
	double right = -std::numeric_limits<double>::infinity();
	for(int i=0; i<estate->particles_size(); i++){
		double ksi = estate->particles(i).ksi();
		if(ksi < left)
			left = ksi;
		if(ksi > right)
			right = ksi;
	}

	double h = (right - left) / 100;
	double ksi = left - h;

	// split into 102 pieces
	for(int i=0; i<103; i++){
		double E = 0;
		for(int j=0; j<estate->particles_size(); j++){
			E += config->f()*cos(2*M_PI/(1+config->delta()*estate->particles(j).v())*(estate->particles(j).ksi()-ksi));
		}
		E *= -2/config->n();

		fprintf(to_gnuplot, "%lf %lf\n", ksi, E);

		ksi += h;
	}
	fprintf(to_gnuplot, "e\n");
	fflush(to_gnuplot);

	++states_count;
}
int E2FieldAnalyzer::getStatesCount(){
	return states_count;
}
E2FieldAnalyzer::~E2FieldAnalyzer(){
	fclose(to_gnuplot);
	delete config;
}
