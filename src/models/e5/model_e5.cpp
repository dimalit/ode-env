#include <models/e5/model_e5.h>

E5Config::E5Config(){
	set_n(100);
	set_e0(0);
}

E5State::E5State(){
}

E5State::E5State(const E5Config* config){
	int N = config->n();
	for(int i=0; i<N; i++){
		this->add_particles();
		this->mutable_particles(i)->set_x(0);
		this->mutable_particles(i)->set_y(0.0);
		this->mutable_particles(i)->set_z(0.0);

		this->add_fields();
		this->mutable_fields(i)->set_x(0.0);
		this->mutable_fields(i)->set_y(0.0);
	}
}

