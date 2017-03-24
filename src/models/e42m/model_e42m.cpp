/*
 * model_e3.cpp
 *
 *  Created on: Aug 19, 2014
 *      Author: dimalit
 */

#include "model_e42m.h"

E42mConfig::E42mConfig(){
	set_n(1000);
	set_delta_0(0);
	set_alpha(0);
}

E42mState::E42mState(){
	set_e_p(0.001);
	set_phi_p(0.0);
	set_e_m(0.001);
	set_phi_m(0.0);
}

E42mState::E42mState(const E42mConfig* config){
	int N = config->n();
	for(int i=0; i<N; i++){
		this->add_particles();
		this->mutable_particles(i)->set_a(1);
		this->mutable_particles(i)->set_psi(0.0);
		this->mutable_particles(i)->set_z(0.0);
		this->mutable_particles(i)->set_delta(0.0);
	}
	set_e_p(0.001);
	set_phi_p(0.0);
	set_e_m(0.001);
	set_phi_m(0.0);
}

