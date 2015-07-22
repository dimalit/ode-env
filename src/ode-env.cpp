#include "HelloWorld.h"

#include "model_e1.h"

#include <gtkmm/main.h>

extern void exp_6_images();
extern void exp_random_init_R3();
extern void exp_gamma2e();
extern void exp_r2e();

int main (int argc, char *argv[])
{
	srand(time(NULL));

	Gtk::Main kit(argc, argv);
	libgoffice_init();
	go_plugins_init (NULL, NULL, NULL, NULL, TRUE, GO_TYPE_PLUGIN_LOADER_MODULE);

//	HelloWorld helloworld;
//	kit.run(helloworld);

//	exp_6_images();
//	exp_random_init_R3();
//	exp_r2e();
	exp_gamma2e();

/*
	E1InstanceFactory* eif = E1InstanceFactory::getInstance();
	E1SolverFactory* esf = E1SolverFactory::getInstance();
1
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
