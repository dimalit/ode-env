#include <models/e5/gui_e5.h>

E5StateGeneratorWidget::E5StateGeneratorWidget(const E5Config* _config){
	this->state = NULL;

	if(_config)
		this->config = new E5Config(*_config);
	else
		this->config = new E5Config();

	// TODO: may be state should remember its config?!
	pb::E5StateGeneratorConfig* sgc = new pb::E5StateGeneratorConfig();
	sgc->set_e0(0);
	sgc->set_a0(1.0);
	sgc->set_left(0);
	sgc->set_right(1);
	cfg_widget.setData(sgc);

	this->add(cfg_widget);
	cfg_widget.signal_changed().connect(sigc::mem_fun(*this, &E5StateGeneratorWidget::on_changed));

	assert(!this->state);
	newState();
}

E5StateGeneratorWidget::~E5StateGeneratorWidget(){
	// XXX where are deletes?
}

void E5StateGeneratorWidget::on_changed(){
	newState();
}

void E5StateGeneratorWidget::loadConfig(const OdeConfig* cfg){
	const E5Config* ecfg = dynamic_cast<const E5Config*>(cfg);
		assert(ecfg);

	delete this->config;
	this->config = new E5Config(*ecfg);

	delete this->state;
	this->state = NULL;

	newState();
}
const OdeConfig* E5StateGeneratorWidget::getConfig(){
	return config;
}

const OdeState* E5StateGeneratorWidget::getState(){
	assert(config);
	assert(state);
	return state;
}

void E5StateGeneratorWidget::newState(bool emit){
//	srand(0);

	delete state;
	state = new E5State(config);

	const pb::E5StateGeneratorConfig* sgc = dynamic_cast<const pb::E5StateGeneratorConfig*>(cfg_widget.getData());

	double e0 = sgc->e0();
	double a0 = sgc->a0();
	double left = sgc->left();
	double right = sgc->right();

	int N = config->n();

	for(int i=0; i<N; i++){
		double psi = rand() / (double)RAND_MAX * (2*M_PI);
		double z = i / (double)N * (right-left) + left;

		pb::E5State::Particles p;
		p.set_x(a0*cos(psi));
		p.set_y(a0*sin(psi));
		p.set_z(z);

		state->mutable_particles(i)->CopyFrom(p);
		state->mutable_fields(i)->set_x(0);
		state->mutable_fields(i)->set_y(0);
	}

	state->set_a0(a0);

	if(emit)
		m_signal_changed();
}

void E5StateGeneratorWidget::center_masses(){
	int N = config->n();

	double sum_px = 0, sum_py = 0;
	double sum_mx = 0, sum_my = 0;
	double half_sum = N / M_PI;

	// center cosine-weighted masses...
	for(int i=0; i<N; i++){
		E5State::Particles p = state->particles(i);

		if(cos(2*M_PI*p.z()) > 0){
			sum_px += p.x()*cos(2*M_PI*p.z());
			sum_py += p.y()*cos(2*M_PI*p.z());
		}
		else{
			sum_mx += p.x()*cos(2*M_PI*p.z());
			sum_my += p.y()*cos(2*M_PI*p.z());
		}// else
	}

	for(int i=0; i<N; i++){
		E5State::Particles& p = *state->mutable_particles(i);

		if(cos(2*M_PI*p.z()) > 0){
			p.set_x(p.x()-sum_px/half_sum);
			p.set_y(p.y()-sum_py/half_sum);
		}
		else{
			p.set_x(p.x()+sum_mx/half_sum);
			p.set_y(p.y()+sum_my/half_sum);
		}//else
	}
}
