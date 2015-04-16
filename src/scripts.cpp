/*
 * scripts.cpp
 *
 *  Created on: Apr 8, 2015
 *      Author: dimalit
 */

#include "model_e3.h"
#include "Gnuplot.h"
#include "ChartAnalyzer.h"

#include <gtkmm/window.h>
#include <string>
#include <sstream>
#include <vector>

using namespace std;

void exp_6_images(){
	string problem_name = "model e3";

	OdeInstanceFactory* inst_fact = OdeInstanceFactoryManager::getInstance()->getFactory( problem_name );
	OdeSolverFactory* solver_fact = *OdeSolverFactoryManager::getInstance()->getFactoriesFor(inst_fact).first;

	E3Config* pcfg = dynamic_cast<E3Config*>(inst_fact->createConfig());
		pcfg->set_theta_e(0.0);
		pcfg->set_delta_e(0.0);
		pcfg->set_m(250);
		pcfg->set_n(1.0);
	E3PetscSolverConfig* scfg = dynamic_cast<E3PetscSolverConfig*>(solver_fact->createSolverConfg());
		scfg->set_init_step(0.01);
		scfg->set_tolerance(0.001);

	ChartAnalyzer chart_analyzer(pcfg);
	Gtk::Window win;
	win.add(chart_analyzer);
	win.show_all();

	Gnuplot E_plot;
		E_plot.addVar("E");
		E_plot.setXAxisTime();
		E_plot.setStyle(Gnuplot::STYLE_LINES);
	Gnuplot phi_plot;
		phi_plot.addVar("phi");
		phi_plot.setXAxisTime();
		phi_plot.setStyle(Gnuplot::STYLE_LINES);
	Gnuplot field_plot;
		field_plot.addVar("particles.a");
		field_plot.setXAxisVar("particles.ksi");
		field_plot.setStyle(Gnuplot::STYLE_POINTS);

	vector<double> gammas;
	gammas.push_back(0.1);
	gammas.push_back(1.0);

	vector<string> models;
	models.push_back("te");
	models.push_back("tm");

	vector<double> rs;
	rs.push_back(0.3);
	rs.push_back(1.0);
	rs.push_back(3.0);

	for(int i=0; i<gammas.size(); i++){
		double gamma = gammas[i];
		pcfg->set_gamma_0_2(gamma);

		for(int j=0; j<models.size(); j++){
			string model = models[j];
			scfg->set_model(model);

			for(int k=0; k<rs.size(); k++){
				double r = rs[k];
				pcfg->set_r_e(r);
	////////////////////////////////////////////////////
				std::ostringstream fname_ostr;
				fname_ostr << model << "_g" << gamma << "_r" << r;
				string fname = fname_ostr.str();

				E3State* init_state = dynamic_cast<E3State*>(inst_fact->createState(pcfg));
				init_state->set_e(0.01);
				init_state->set_phi(0);
				init_state->set_a0(1.0);
				double eta0 = 0.0;
				double right = 0.5, left = -0.5;
					for(int i=0; i<pcfg->m(); i++){
						init_state->mutable_particles(i)->set_a(init_state->a0());
						init_state->mutable_particles(i)->set_eta(eta0);
						double ksi = rand() / (double)RAND_MAX * (right-left) + left;
						init_state->mutable_particles(i)->set_ksi(ksi);
					}
				E3PetscSolver* solver = dynamic_cast<E3PetscSolver*>(solver_fact->createSolver(scfg, pcfg, init_state));

				E_plot.reset();
				phi_plot.reset();
				field_plot.reset();

				int dE = 0;
				int prev_dE = 0;
				int peaks_found = 0;
				double last_peak = init_state->e();
				double peak_time = std::numeric_limits<double>::infinity();

				double time = 0.0;
				for(bool first=true;;first=false){
					const google::protobuf::Message* state_msg = dynamic_cast<const google::protobuf::Message*>(solver->getState());
					const google::protobuf::Message* dstate_msg = dynamic_cast<const google::protobuf::Message*>(solver->getDState());
					time += solver->getTime();

					chart_analyzer.processState(solver->getState(), solver->getDState(), time);
					E_plot.processState(state_msg, dstate_msg, time);
					phi_plot.processState(state_msg, dstate_msg, time);
					field_plot.processState(state_msg, dstate_msg, time);

					if(!first)
						dE = dynamic_cast<const E3State*>(solver->getDState())->e() > 0 ? 1 : -1;

					double e = dynamic_cast<const E3State*>(solver->getState())->e();

					if(first)
						field_plot.saveToCsv(fname+"_field_begin.gnuplot", state_msg, dstate_msg, time);

					// if peak
					if(dE * prev_dE == -1 && (e/last_peak > 1.1 || e/last_peak < 0.9)){
						peaks_found++;
						std::ostringstream peaks_s;
						peaks_s<<peaks_found;
						field_plot.saveToCsv(fname+"_field"+peaks_s.str()+".gnuplot", state_msg, dstate_msg, time);
						peak_time = time;
						last_peak = e;
					}

					// exit?
					if(solver->getSteps() >= 1000 || time > 60.0){
						E_plot.saveToCsv(fname+"_E.gnuplot", state_msg, dstate_msg, time);
						phi_plot.saveToCsv(fname+"_phi.gnuplot", state_msg, dstate_msg, time);
						field_plot.saveToCsv(fname+"_field_end.gnuplot", state_msg, dstate_msg, time);
						break;
					}// if exit

					prev_dE = dE;
					solver->run(1000000, 0.5);
				}// infinite loop

				delete solver;
				delete init_state;
	////////////////////////////////////////////////////
			}// for k rs
		}// for j models
	}// for i gammas

	delete scfg;
	delete pcfg;
}// exp_6_images

