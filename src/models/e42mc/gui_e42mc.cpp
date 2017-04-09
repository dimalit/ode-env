#include <models/e42mc/gui_e42mc.h>

E42mcStateGeneratorWidget::E42mcStateGeneratorWidget(const E42mcConfig* _config){
	this->state = NULL;

	if(_config)
		this->config = new E42mcConfig(*_config);
	else
		this->config = new E42mcConfig();

	// TODO: may be state should remember its config?!
	pb::E42mcStateGeneratorConfig* sgc = new pb::E42mcStateGeneratorConfig();
	sgc->set_e_p0(0.05);
	sgc->set_phi_p0(0.0);
	sgc->set_e_m0(0.0);
	sgc->set_phi_m0(0.0);
	sgc->set_a0(1.0);
	cfg_widget.setData(sgc);

	this->add(cfg_widget);
	cfg_widget.signal_changed().connect(sigc::mem_fun(*this, &E42mcStateGeneratorWidget::on_changed));

	assert(!this->state);
	newState();
}

E42mcStateGeneratorWidget::~E42mcStateGeneratorWidget(){
	// XXX where are deletes?
}

void E42mcStateGeneratorWidget::on_changed(){
	newState();
}

void E42mcStateGeneratorWidget::loadConfig(const OdeConfig* cfg){
	const E42mcConfig* ecfg = dynamic_cast<const E42mcConfig*>(cfg);
		assert(ecfg);

	delete this->config;
	this->config = new E42mcConfig(*ecfg);

	delete this->state;
	this->state = NULL;

	newState();
}
const OdeConfig* E42mcStateGeneratorWidget::getConfig(){
	return config;
}

const OdeState* E42mcStateGeneratorWidget::getState(){
	assert(config);
	assert(state);
	return state;
}

void E42mcStateGeneratorWidget::newState(bool emit){
	delete state;
	state = new E42mcState(config);

	const pb::E42mcStateGeneratorConfig* sgc = dynamic_cast<const pb::E42mcStateGeneratorConfig*>(cfg_widget.getData());

	double e_p = sgc->e_p0();
	double phi_p = sgc->phi_p0();
	double e_m = sgc->e_m0();
	double phi_m = sgc->phi_m0();
	double a0 = sgc->a0();

//	double right = 2*M_PI;
//	double left = 0.0;

	int N = config->n();
	int sqN = sqrt(N);

//	double* psis = new double[N];
//	int i = 0;
//	for_each(psis, psis+N, [&i,N](double& d)->void{d = (double)i++ / N * (2*M_PI);});
//	random_shuffle(psis, psis+N);

	for(int i=0; i<sqN; i++){
		for(int j=0; j<sqN; j++){
			int k = i*sqN+j;

			//double psi = rand() / (double)RAND_MAX * (2*M_PI) + 0;
	//		double psi = psis[i];
			double z = i / (double)sqN;// * 5 + 0;
			double psi = j / (double)sqN * 2*M_PI;// * 5 + 0;
	//		double delta = rand() / (double)RAND_MAX * (2*config->delta_0()) - config->delta_0();

			pb::E42mcState::Particles p;
			p.set_x(0);
			p.set_y(0);
			p.set_z(z);
			p.set_xn(a0*cos(psi));//+0.1*sin(psi-2*M_PI*z));
			p.set_yn(a0*sin(psi));
			//p.set_a(a0*(1.0+0.2*sin(int(psi-z+phi))));

			state->mutable_particles(k)->CopyFrom(p);
		}
	}
//	delete psis;

	state->set_x_p(e_p*cos(phi_p));
	state->set_y_p(e_p*sin(phi_p));
	state->set_x_m(e_m*cos(phi_m));
	state->set_y_m(e_m*sin(phi_m));

//	center_masses();

	if(emit)
		m_signal_changed();
}

void E42mcStateGeneratorWidget::center_masses(){
	int N = config->n();

	double sum_px = 0, sum_py = 0;
	double sum_mx = 0, sum_my = 0;
	double half_sum = N / M_PI;

	// center cosine-weighted masses...
	for(int i=0; i<N; i++){
		E42mcState::Particles p = state->particles(i);

		if(cos(2*M_PI*p.z()) > 0){
			sum_px += (p.x()+p.xn())*cos(2*M_PI*p.z());
			sum_py += (p.y()+p.yn())*cos(2*M_PI*p.z());
		}
		else{
			sum_mx += (p.x()+p.xn())*cos(2*M_PI*p.z());
			sum_my += (p.y()+p.yn())*cos(2*M_PI*p.z());
		}// else
	}

	for(int i=0; i<N; i++){
		E42mcState::Particles& p = *state->mutable_particles(i);

		if(cos(2*M_PI*p.z()) > 0){
			p.set_xn(p.xn()-sum_px/half_sum);
			p.set_yn(p.yn()-sum_py/half_sum);
		}
		else{
			p.set_xn(p.xn()+sum_mx/half_sum);
			p.set_yn(p.yn()+sum_my/half_sum);
		}//else
	}
}
