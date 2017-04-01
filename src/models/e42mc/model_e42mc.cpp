#include <models/e42mc/model_e42mc.h>

E42mcConfig::E42mcConfig(){
	set_n(900);
	set_delta_0(0);
	set_alpha(0.1);
}

E42mcState::E42mcState(){
	set_x_p(0.05);
	set_y_p(0.0);
	set_x_m(0.0);
	set_y_m(0.0);
}

E42mcState::E42mcState(const E42mcConfig* config){
	int N = config->n();
	for(int i=0; i<N; i++){
		this->add_particles();
		this->mutable_particles(i)->set_x(1);
		this->mutable_particles(i)->set_y(0.0);
		this->mutable_particles(i)->set_z(0.0);
		this->mutable_particles(i)->set_xn(0.0);
		this->mutable_particles(i)->set_yn(0.0);
	}
	set_x_p(0.05);
	set_y_p(0.0);
	set_x_m(0.0);
	set_y_m(0.0);
}

