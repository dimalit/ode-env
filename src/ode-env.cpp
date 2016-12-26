#include "HelloWorld2.h"
#include "models/e1/model_e1.h"

#include <goffice/goffice.h>
#include <gtkmm/main.h>

extern void exp_6_images();
extern void exp_random_init_R3();
extern void exp_gamma2e();
extern void exp_r2e();
extern void as_in_book();
extern void exp_4_gyro();
extern void exp_ts4_super_emission();

int main (int argc, char *argv[])
{
	srand(time(NULL));

	Gtk::Main kit(argc, argv);
	libgoffice_init();
	//go_plugins_init (NULL, NULL, NULL, NULL, TRUE, GO_TYPE_PLUGIN_LOADER_MODULE);
	go_plugins_init(NULL, NULL,NULL,NULL,TRUE, G_TYPE_TYPE_MODULE);

	HelloWorld2 helloworld;
	kit.run(helloworld);

	srand(time(NULL));
//	exp_6_images();
//	exp_random_init_R3();
//	exp_r2e();
//	exp_gamma2e();
//	as_in_book();

//	exp_4_gyro();
//	exp_ts4_super_emission();

/*
	E1InstanceFactory* eif = E1InstanceFactory::getInstance();
	E1SolverFactory* esf = E1SolverFactory::getInstance();

	E1Config* conf = (E1Config*) eif->createConfig();
	E1State* state = (E1State*) eif->createState(conf);

	E1PetscSolverConfig* sconf = (E1PetscSolverConfig*) esf->createSolverConfg();
	E1PetscSolver* solver = (E1PetscSolver*) esf->createSolver(sconf, conf, state);

	for(int i=0; i<1000; i++){
		E1State* cur_state = (E1State*) solver->getCurrentState();
		solver->step();
	}
*/

	libgoffice_shutdown();

	return 0;
}
