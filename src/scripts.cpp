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
#include <fstream>
#include <vector>

using namespace std;

void compute_integrals(const E3Config* config, const E3State* estate, double* int1, double* int2, double* int3){
	double i1 = 0;
	double sum_a_2 = 0;
	double sum_eta = 0;
	for(int i=0; i<estate->particles_size(); i++){
		E3State::Particles p = estate->particles(i);

		double integral = config->r_e()*p.a()*p.a() - 2*config->n()*p.eta();
		i1 += integral;
		sum_a_2 += p.a()*p.a();
		sum_eta += p.eta();
	}

	*int1 =  i1/estate->particles_size();
	*int2 = estate->e()*estate->e()+2.0/config->r_e()/config->m()*sum_eta;
	*int3 = estate->e()*estate->e()+1.0/config->n()/config->m()*sum_a_2;
}

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
		scfg->set_atol(1e-6);
		scfg->set_rtol(1e-6);
		scfg->set_solver(E3PetscSolverConfig::rhs);

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
//	gammas.push_back(1.1);

	vector<string> models;
	models.push_back("te");
//	models.push_back("tm");

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

				std::ofstream int_str(fname+"_int.csv", ios::out | ios::binary);
				int_str.precision(10);

				E3State* init_state = dynamic_cast<E3State*>(inst_fact->createState(pcfg));
				init_state->set_e(0.01);
				init_state->set_phi(0);
				init_state->set_a0(1.0);
				double eta0 = 0.0;
				double right = 0.5, left = -0.5;
					for(int i=0; i<pcfg->m(); i++){
						init_state->mutable_particles(i)->set_a(init_state->a0());
						init_state->mutable_particles(i)->set_eta(eta0);
						//double ksi = rand() / (double)RAND_MAX * (right-left) + left;
						double ksi = i / (double)pcfg->m() * (right-left) + left;
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

					double int1, int2, int3;
					compute_integrals(pcfg, dynamic_cast<const E3State*>(solver->getState()), &int1, &int2, &int3);
					int_str << int1 << "\t" << int2 << "\t" << int3 << "\n";

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
					if(solver->getSteps() >= 300 || time > 120.0){
						E_plot.saveToCsv(fname+"_E.gnuplot", state_msg, dstate_msg, time);
						phi_plot.saveToCsv(fname+"_phi.gnuplot", state_msg, dstate_msg, time);
						field_plot.saveToCsv(fname+"_field_end.gnuplot", state_msg, dstate_msg, time);
						int_str.close();
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

void exp_random_init_R3(){
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
		scfg->set_atol(0.001);
		scfg->set_rtol(0.001);

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

		double gamma = 1.0;
			pcfg->set_gamma_0_2(gamma);
		string model = "te";
			scfg->set_model(model);
		double r = 3.0;
			pcfg->set_r_e(r);
		for(int exp=0; exp < 10; exp++){
	////////////////////////////////////////////////////
		std::ostringstream fname_ostr;
		fname_ostr << exp;
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

		double time = 0.0;
		for(;;){
			const google::protobuf::Message* state_msg = dynamic_cast<const google::protobuf::Message*>(solver->getState());
			const google::protobuf::Message* dstate_msg = dynamic_cast<const google::protobuf::Message*>(solver->getDState());
			time += solver->getTime();

			chart_analyzer.processState(solver->getState(), solver->getDState(), time);
			E_plot.processState(state_msg, dstate_msg, time);
			phi_plot.processState(state_msg, dstate_msg, time);
			field_plot.processState(state_msg, dstate_msg, time);

			if(time > 60.0){
				E_plot.saveToCsv(fname+"_E.gnuplot", state_msg, dstate_msg, time);
				break;
			}// if exit

			solver->run(1000000, 0.5);
		}// infinite loop

		delete solver;
		delete init_state;
	////////////////////////////////////////////////////
		}// for
	delete scfg;
	delete pcfg;
}// exp_6_images

void exp_gamma2e(){
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
		scfg->set_atol(1e-6);
		scfg->set_rtol(1e-6);
		scfg->set_solver(E3PetscSolverConfig::rhs);

	ChartAnalyzer chart_analyzer(pcfg);
	Gtk::Window win;
	win.add(chart_analyzer);
	win.show_all();

	Gnuplot E_plot;
		E_plot.addVar("E");
		E_plot.setXAxisTime();
		E_plot.setStyle(Gnuplot::STYLE_LINES);

	vector<string> models;
//	models.push_back("te");
	models.push_back("tm");

	vector<double> rs;
	rs.push_back(1.0);
	rs.push_back(2.0);
	rs.push_back(3.0);

	for(int i=0; i<rs.size(); i++){
		double r = rs[i];
		pcfg->set_r_e(r);

		for(int j=0; j<models.size(); j++){
			string model = models[j];
			scfg->set_model(model);

			std::ostringstream fname_ostr;
			fname_ostr << model << "_r" << r << ".csv";
			string fname = fname_ostr.str();
			std::ofstream csv(fname, ios::out | ios::binary);
			csv.precision(10);

			for(double gamma=0.05; gamma<=5.1; gamma+=0.25){
				pcfg->set_gamma_0_2(gamma);

		////////////////////////////////////////////////////

				E3State* init_state = dynamic_cast<E3State*>(inst_fact->createState(pcfg));
				init_state->set_e(0.01);
				init_state->set_phi(0);
				init_state->set_a0(1.0);
				double eta0 = 0.0;
				double right = 0.5, left = -0.5;
					for(int i=0; i<pcfg->m(); i++){
						init_state->mutable_particles(i)->set_a(init_state->a0());
						init_state->mutable_particles(i)->set_eta(eta0);
						//double ksi = rand() / (double)RAND_MAX * (right-left) + left;
						double ksi = i / (double)pcfg->m() * (right-left) + left;
						init_state->mutable_particles(i)->set_ksi(ksi);
					}
				E3PetscSolver* solver = dynamic_cast<E3PetscSolver*>(solver_fact->createSolver(scfg, pcfg, init_state));

				E_plot.reset();

				int dE = 0;
				int prev_dE = 0;

				double time = 0.0;
				for(bool first=true;;first=false){
					const google::protobuf::Message* state_msg = dynamic_cast<const google::protobuf::Message*>(solver->getState());
					const google::protobuf::Message* dstate_msg = dynamic_cast<const google::protobuf::Message*>(solver->getDState());
					time += solver->getTime();

					chart_analyzer.processState(solver->getState(), solver->getDState(), time);
					E_plot.processState(state_msg, dstate_msg, time);

	//				double int1, int2, int3;
	//				compute_integrals(pcfg, dynamic_cast<const E3State*>(solver->getState()), &int1, &int2, &int3);

					if(!first)
						dE = dynamic_cast<const E3State*>(solver->getDState())->e() > 0 ? 1 : -1;

					double e = dynamic_cast<const E3State*>(solver->getState())->e();

					// if peak
					if(dE < 0 && prev_dE > 0 && e>0.09){
						csv << gamma << "\t" << e << endl;
						break;
					}

					if(dE != 0.0)
							prev_dE = dE;
					solver->run(1000000, 0.5);
				}// infinite loop

				delete solver;
				delete init_state;
		////////////////////////////////////////////////////
			}// for gammas
			csv.close();
		}// for models
	}// for r
	delete scfg;
	delete pcfg;
}
