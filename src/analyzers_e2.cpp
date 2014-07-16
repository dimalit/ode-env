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

	fprintf(to_gnuplot, "set terminal x11\n");
	fflush(to_gnuplot);
}

void E2FieldAnalyzer::reset(){
	states_count = 0;
}
void E2FieldAnalyzer::processState(const OdeState* state, double time){
	const E2State* estate = dynamic_cast<const E2State*>(state);
		assert(estate);

	fprintf(to_gnuplot, "plot '-' with points\n");

	for(int i=0; i<estate->particles_size(); i++){
		double ksi = estate->particles(i).ksi();

		double E = 0;
		for(int j=0; j<estate->particles_size(); j++){
			if(i==j)continue;
			E += config->f()*cos(2*M_PI/(1+config->delta()*estate->particles(j).v())*(estate->particles(j).ksi()-ksi));
		}
		E *= -2/config->n();

		fprintf(to_gnuplot, "%lf %lf\n", ksi, E);
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


