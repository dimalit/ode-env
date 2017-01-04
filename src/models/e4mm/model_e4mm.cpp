/*
 * model_e4mm.cpp
 *
 *  Created on: Jan 2, 2017
 *      Author: dimalit
 */

#include <models/e4mm/model_e4mm.h>

E4mmConfig::E4mmConfig(){
	set_n0(100);
	set_n(75*n0());
	set_beta(5);
	set_k(3);
	set_alpha(0);
	set_s(1.0);
	set_gamma_omega(0.0);			// TODO: not implemented!!
}

E4mmState::E4mmState(){
	// XXX: no fields, no particles
}

E4mmState::E4mmState(const E4mmConfig* config){
	int N = config->n();
	int k = config->k();

	for(int j=0; j<2*k+1; j++){
		this->add_fields();
	}

	for(int i=0; i<N; i++){
		this->add_particles();
	}
}

