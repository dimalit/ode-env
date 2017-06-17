/*
 * model_e3.cpp
 *
 *  Created on: Aug 19, 2014
 *      Author: dimalit
 */

#include "model_e4.h"

E4Config::E4Config(){
	set_n(1000);
	set_delta_0(0);
	set_alpha(0);
	set_e0(0);
	set_delta_phi(0.0);
}

E4State::E4State(){
	set_e(0.001);
	set_phi(0.0);
}

E4State::E4State(const E4Config* config){
	int N = config->n();
	for(int i=0; i<N; i++){
		this->add_particles();
		this->mutable_particles(i)->set_a(1);
		this->mutable_particles(i)->set_psi(0.0);
		this->mutable_particles(i)->set_z(0.0);
		this->mutable_particles(i)->set_delta(0.0);
	}
	set_e(0.001);
	set_phi(0.0);
}

