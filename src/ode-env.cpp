#include "HelloWorld.h"

#include "model_e1.h"

#include <gtkmm/main.h>

int main (int argc, char *argv[])
{
	Gtk::Main kit(argc, argv);
	libgoffice_init();
	go_plugins_init (NULL, NULL, NULL, NULL, TRUE, GO_TYPE_PLUGIN_LOADER_MODULE);

	HelloWorld helloworld;

	kit.run(helloworld);

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
