/*
 * scripts.cpp
 *
 *  Created on: Apr 8, 2015
 *      Author: dimalit
 */

#include "model_e3.h"
#include "model_e4.h"
#include "Gnuplot.h"
#include "ChartAnalyzer.h"
#include "MaxDetector.h"

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

	OdeProblem* inst_fact = OdeProblemManager::getInstance()->getProblem( problem_name );
	OdeSolverFactory* solver_fact = *OdeSolverFactoryManager::getInstance()->getTypesFor(inst_fact).first;

	E3Config* pcfg = dynamic_cast<E3Config*>(inst_fact->createConfig());
		pcfg->set_theta_e(0.0);
		pcfg->set_delta_e(0.0);
		pcfg->set_m(240);
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

	OdeProblem* inst_fact = OdeProblemManager::getInstance()->getProblem( problem_name );
	OdeSolverFactory* solver_fact = *OdeSolverFactoryManager::getInstance()->getTypesFor(inst_fact).first;

	E3Config* pcfg = dynamic_cast<E3Config*>(inst_fact->createConfig());
		pcfg->set_theta_e(0.0);
		pcfg->set_delta_e(0.0);
		pcfg->set_m(240);
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

	OdeProblem* inst_fact = OdeProblemManager::getInstance()->getProblem( problem_name );
	OdeSolverFactory* solver_fact = *OdeSolverFactoryManager::getInstance()->getTypesFor(inst_fact).first;

	E3Config* pcfg = dynamic_cast<E3Config*>(inst_fact->createConfig());
		pcfg->set_theta_e(0.0);
		pcfg->set_delta_e(0.0);
		pcfg->set_m(240);
		pcfg->set_n(1.0);
	E3PetscSolverConfig* scfg = dynamic_cast<E3PetscSolverConfig*>(solver_fact->createSolverConfg());
		scfg->set_init_step(0.01);
		scfg->set_atol(1e-7);
		scfg->set_rtol(1e-7);
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
	models.push_back("te");
//	models.push_back("tm");

	vector<double> rs;
	rs.push_back(1.0);
//	rs.push_back(2.0);
//	rs.push_back(3.0);

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
						double ksi = rand() / (double)RAND_MAX * (right-left) + left;
						//double ksi = i / (double)pcfg->m() * (right-left) + left;
						init_state->mutable_particles(i)->set_ksi(ksi);
					}
				E3PetscSolver* solver = dynamic_cast<E3PetscSolver*>(solver_fact->createSolver(scfg, pcfg, init_state));

				E_plot.reset();

				int dE = 0;
				int prev_dE = 0;
				double max_E=0.0;
				double max_time = 0.0;
				double e;
				const google::protobuf::Message *state_msg, *dstate_msg;

				double time = 0.0;
				solver->run(1000000, 120, true);

				for(int i=0;;i++){
					if(!solver->step())
						break;
					state_msg = dynamic_cast<const google::protobuf::Message*>(solver->getState());
					dstate_msg = dynamic_cast<const google::protobuf::Message*>(solver->getDState());
					time = solver->getTime();

					if(i%10==0){
						chart_analyzer.processState(solver->getState(), solver->getDState(), time);
						E_plot.processState(state_msg, dstate_msg, time);
					}

	//				double int1, int2, int3;
	//				compute_integrals(pcfg, dynamic_cast<const E3State*>(solver->getState()), &int1, &int2, &int3);

					e = dynamic_cast<const E3State*>(solver->getState())->e();

					if(max_E < e){
						max_E=e;
						max_time = time;
					}
//					if(i!=0)
//						dE = dynamic_cast<const E3State*>(solver->getDState())->e() > 0 ? 1 : -1;
//
//					// if peak
//					if(dE < 0 && prev_dE > 0){
//						csv << gamma << "\t" << e << endl;
//						break;
//					}
//
//					if(dE != 0.0)
//							prev_dE = dE;
				}// infinite loop

				csv << gamma << "\t" << max_E << "\t" << max_time << endl;
				std::ostringstream imgname;
				imgname << model << "_r" << r << "_g" << gamma << ".png";
				E_plot.processToFile(imgname.str(), state_msg, dstate_msg, time);

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

void exp_r2e(){
	string problem_name = "model e3";

	OdeProblem* inst_fact = OdeProblemManager::getInstance()->getProblem( problem_name );
	OdeSolverFactory* solver_fact = *OdeSolverFactoryManager::getInstance()->getTypesFor(inst_fact).first;

	E3Config* pcfg = dynamic_cast<E3Config*>(inst_fact->createConfig());
		pcfg->set_theta_e(0.0);
		pcfg->set_delta_e(0.0);
		pcfg->set_m(240);
		pcfg->set_n(1.0);
	E3PetscSolverConfig* scfg = dynamic_cast<E3PetscSolverConfig*>(solver_fact->createSolverConfg());
		scfg->set_init_step(0.01);
		scfg->set_atol(1e-10);
		scfg->set_rtol(1e-10);
		scfg->set_solver(E3PetscSolverConfig::rhs);

	ChartAnalyzer chart_analyzer(pcfg);
	Gtk::Window win;
	win.add(chart_analyzer);
	win.show_all();

	Gnuplot E_plot;
		E_plot.addVar("E");
		E_plot.setXAxisTime();
		E_plot.setStyle(Gnuplot::STYLE_LINES);

	Gnuplot a_plot;
		a_plot.addVar("particles.a");
		a_plot.setXAxisVar("particles.ksi");
		a_plot.setPolar(true);

	vector<string> models;
	//models.push_back("te");
	models.push_back("tm");

	vector<double> a0s;
//	a0s.push_back(0.1);
//	a0s.push_back(0.3);
	a0s.push_back(0.6);
//	a0s.push_back(1.0);
//	a0s.push_back(1.79);
//	a0s.push_back(1.85);
//	a0s.push_back(2.0);
//	a0s.push_back(2.5);
//	a0s.push_back(3.0);
	a0s.push_back(3.5);

	pcfg->set_gamma_0_2(0);

	for(int j=0; j<models.size(); j++){
		string model = models[j];
		scfg->set_model(model);

		for(int k=0; k<a0s.size(); k++){
			double a0 = a0s[k];

			std::ostringstream fname_ostr;
			fname_ostr << "r2e_" << model << "_a0_" << a0 << ".csv";
			string fname = fname_ostr.str();
			std::ofstream csv(fname, ios::out | ios::binary);
			csv.precision(10);

			for(double r=0.0002;r<17.0; r*=2){
				pcfg->set_r_e(r);


	////////////////////////////////////////////////////

			E3State* init_state = dynamic_cast<E3State*>(inst_fact->createState(pcfg));
			init_state->set_e(0.01);
			init_state->set_phi(0);
			init_state->set_a0(a0);
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
			a_plot.reset();

			int dE = 0;
			int prev_dE = 0;
			double max_E=0.0;
			double max_time = 0.0;
			double e;
			const google::protobuf::Message *state_msg, *dstate_msg;

			double time = 0.0;
			solver->run(1000000, 1000, true);

			MaxDetector d1, d2;
			d2.push(0.0);

			for(int i=0;;i++){
				if(!solver->step())
					break;
				state_msg = dynamic_cast<const google::protobuf::Message*>(solver->getState());
				dstate_msg = dynamic_cast<const google::protobuf::Message*>(solver->getDState());
				time = solver->getTime();

				if(i%10==0){
					chart_analyzer.processState(solver->getState(), solver->getDState(), time);
					E_plot.processState(state_msg, dstate_msg, time);
					a_plot.processState(state_msg, dstate_msg, time);
				}

//				double int1, int2, int3;
//				compute_integrals(pcfg, dynamic_cast<const E3State*>(solver->getState()), &int1, &int2, &int3);

				e = dynamic_cast<const E3State*>(solver->getState())->e();

				if(d1.push(e) && d2.push(d1.get(1)))
						break;
				// if almost equal
				if(d2.getSize()==3 && abs(d2.get(2)/d2.get(1)-1) < 0.05)
					break;
				// 4 decreasing
				if(d2.getProcessed() >= 4 && d2.isDecreasing())
					break;
//					if(i!=0)
//						dE = dynamic_cast<const E3State*>(solver->getDState())->e() > 0 ? 1 : -1;
//
//					// if peak
//					if(dE < 0 && prev_dE > 0){
//						csv << gamma << "\t" << e << endl;
//						break;
//					}
//
//					if(dE != 0.0)
//							prev_dE = dE;
			}// infinite loop

			csv << r << "\t" << d2.get(1) << endl;
			std::ostringstream imgname;
			imgname << "r2e_" << model << "_a0_" << a0 << "_r" << r << ".png";
			E_plot.processToFile(imgname.str(), state_msg, dstate_msg, time);

			delete solver;
			delete init_state;
	////////////////////////////////////////////////////
		}// for r
		csv.close();
		}// for a0
	}// for models
	delete scfg;
	delete pcfg;
}

// n = -1
// R=25
// E0 = 4e-3 / dqrt(R) = 0.0008
// a0 = E0 (match old integral)
// phi0 should be -PI/4 but 0 is OK too

// THEN:
// T = 27
// maxE = 0.39
// (if we set 2.0 coefs in 1 and 2 eqns of ts3 - will get T=20 maxE*5=3.2 (as in book))

void as_in_book(){
	string problem_name = "model e3";

	OdeProblem* inst_fact = OdeProblemManager::getInstance()->getProblem( problem_name );
	OdeSolverFactory* solver_fact = *OdeSolverFactoryManager::getInstance()->getTypesFor(inst_fact).first;

	E3Config* pcfg = dynamic_cast<E3Config*>(inst_fact->createConfig());
		pcfg->set_theta_e(0.0);
		pcfg->set_delta_e(0.0);
		pcfg->set_m(240);
		pcfg->set_n(-1.0);
		pcfg->set_gamma_0_2(0);
		pcfg->set_r_e(25.0);
	E3PetscSolverConfig* scfg = dynamic_cast<E3PetscSolverConfig*>(solver_fact->createSolverConfg());
		scfg->set_init_step(0.01);
		scfg->set_atol(1e-7);
		scfg->set_rtol(1e-7);
		scfg->set_solver(E3PetscSolverConfig::rhs);
		scfg->set_model("tm");

	ChartAnalyzer chart_analyzer(pcfg);
	Gtk::Window win;
	win.add(chart_analyzer);
	win.show_all();

	Gnuplot E_plot;
		E_plot.addVar("E");
		E_plot.setXAxisTime();
		E_plot.setStyle(Gnuplot::STYLE_LINES);

	////////////////////////////////////////////////////

			E3State* init_state = dynamic_cast<E3State*>(inst_fact->createState(pcfg));
			init_state->set_e(0.0008);
			init_state->set_phi(0.0);
			init_state->set_a0(0.0008);
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

			const google::protobuf::Message *state_msg, *dstate_msg;

			solver->run(1000000, 100, true);

			double prev_e = 0.0;
			double prev_time = 0.0;

			for(int i=0;;i++){
				if(!solver->step())
					break;
				state_msg = dynamic_cast<const google::protobuf::Message*>(solver->getState());
				dstate_msg = dynamic_cast<const google::protobuf::Message*>(solver->getDState());
				double time = solver->getTime();

				chart_analyzer.processState(solver->getState(), solver->getDState(), time);
				E_plot.processState(state_msg, dstate_msg, time);

				double b = 0;
				if(prev_e > 0.0){
					double e = dynamic_cast<const E3State*>(solver->getState())->e();
					b = pow(e/prev_e, 1.0/(time-prev_time));
				}

				double int1, int2, int3;
				compute_integrals(pcfg, dynamic_cast<const E3State*>(solver->getState()), &int1, &int2, &int3);
				fprintf(stderr, "%.10lf\t%.10lf\t%.10lf\t%.10lf\n", int1, int2, int3, b);

				if(time-prev_time > 1.0){
					prev_e = dynamic_cast<const E3State*>(solver->getState())->e();
					prev_time = time;
				}
			}// infinite loop

			delete solver;
			delete init_state;
	////////////////////////////////////////////////////
	delete scfg;
	delete pcfg;
}

void model_e3_fill_spec_msg(const E3State* estate, const E3State* dstate, const E3Config* config, pb::E3Special* spec_msg){
	int N = estate->particles_size();
//	double T = this->total_time;
//	double dT = this->total_time - this->last_refresh_time;

	double sum_a_2 = 0;
	double Na = 0, Nb = 0;			// da/dt>0 and <0
	double Ia = 0.0, Ib = 0.0;	// sum da/dt
	double Wa = 0.0, Wb = 0.0;	// sum a^2
	for(int i=0; i<N; i++){
		E3State::Particles p = estate->particles(i);
		E3State::Particles dp = dstate->particles(i);

		sum_a_2 += p.a()*p.a();

		if(dp.a() > 0.0){
			Na++;
			Ia += dp.a();
			Wa += p.a()*p.a();
		}// Na
		else{
			Nb++;
			Ib += dp.a();
			Wb += p.a()*p.a();
		}// Nb
	}

	Na/=N; Nb/=N;
	Wa/=N; Wb/=N;

	spec_msg->set_e_2(estate->e()*estate->e());
	spec_msg->set_aver_a_2(sum_a_2/estate->particles_size());

	spec_msg->set_na(Na);
	spec_msg->set_nb(Nb);
	spec_msg->set_wa(Wa);
	spec_msg->set_wb(Wb);

	spec_msg->set_n(Na+Nb);
	spec_msg->set_m(Na-Nb);


	// compute max/min
	double min = std::numeric_limits<double>::infinity();
	double max = -std::numeric_limits<double>::infinity();

	for(int i=0; i<N; i++){
		E3State::Particles p = estate->particles(i);
		double v = p.a()*p.a();
		if(v>max)
			max = v;
		if(v<min)
			min = v;
	}// for

	// fill histogram
	int hist[10] = {0};
	for(int i=0; i<N; i++){
		E3State::Particles p = estate->particles(i);
		int bin = int((p.a()*p.a()-min)/(max-min)*10);
		if(bin>=10)
			bin=9;
		if(bin<0)
			bin=0;
		hist[bin]++;
	}// for

	spec_msg->clear_hist();
	for(int i=0; i<10; i++){
		spec_msg->add_hist();
		spec_msg->mutable_hist(i)->set_x(min+i/10.0*(max-min));
		spec_msg->mutable_hist(i)->set_y(hist[i]);
	}
}

void exp_4_gyro(){
	string problem_name = "model e3";

	OdeProblem* inst_fact = OdeProblemManager::getInstance()->getProblem( problem_name );
	OdeSolverFactory* solver_fact = *OdeSolverFactoryManager::getInstance()->getTypesFor(inst_fact).first;

	E3Config* pcfg = dynamic_cast<E3Config*>(inst_fact->createConfig());
		pcfg->set_m(1000);
		pcfg->set_n(1.0);
	E3PetscSolverConfig* scfg = dynamic_cast<E3PetscSolverConfig*>(solver_fact->createSolverConfg());
		scfg->set_model("tm");
		scfg->set_init_step(0.01);
		scfg->set_atol(1e-9);
		scfg->set_rtol(1e-9);
		scfg->set_solver(E3PetscSolverConfig::rhs);
		scfg->set_n_cores(2);

	vector<double> a0s, e0s;
	a0s.push_back(1);
	a0s.push_back(0.1);
	a0s.push_back(1);

	e0s.push_back(1);
	e0s.push_back(1);
	e0s.push_back(0.1);

	// add R=-0.6 mode
	a0s.push_back(0.1);
	e0s.push_back(1.0);
	vector<double> rs;
	rs.push_back(0.0);
	rs.push_back(0.0);
	rs.push_back(0.0);
	rs.push_back(-0.6);

	Gnuplot e_plot;
		e_plot.addVar("E");
		e_plot.setXAxisTime();
		e_plot.setTitle("E");

	Gnuplot phi_plot;
		phi_plot.addVar("phi");
		phi_plot.setXAxisTime();
		phi_plot.setTitle("phi");

	Gnuplot a_plot;
		a_plot.addVar("particles.a");
		a_plot.setXAxisVar("particles.ksi");
		a_plot.setPolar(true);
		a_plot.setTitle("a|ksi");

	Gnuplot hist_plot;
		hist_plot.addVar("hist.y");
		hist_plot.setXAxisVar("hist.x");

	Gnuplot wa_plot;
		wa_plot.addVar("Wa");
		wa_plot.addVar("Wb");
		wa_plot.addVar("aver_a_2");
		wa_plot.addVar("e_2");
		wa_plot.setXAxisTime();
		wa_plot.setTitle("Wa");

	Gnuplot na_plot;
		na_plot.addVar("Na");
		na_plot.addVar("Nb");
		na_plot.setTitle("Na");

	Gnuplot an_plot;
		an_plot.addVar("particles.a");

	Gnuplot ksin_plot;
		ksin_plot.addVar("particles.ksi");

	for(int e_step=3; e_step<a0s.size(); e_step++){
		double a0 = a0s[e_step];
		double e0 = e0s[e_step];
		double r = rs[e_step];
		pcfg->set_r_e(r);

		E3State* init_state = dynamic_cast<E3State*>(inst_fact->createState(pcfg));
		init_state->set_e(e0);
		init_state->set_phi(0);
		init_state->set_a0(a0);
		double right = 0.5, left = -0.5;
		for(int i=0; i<pcfg->m(); i++){
			init_state->mutable_particles(i)->set_a(init_state->a0());
			init_state->mutable_particles(i)->set_eta(0);
			//double ksi = i / (double)pcfg->m() * (right-left) + left + rand()/(double)RAND_MAX*1e-4;
			double ksi = rand()/(double)RAND_MAX * (right-left) + left;
			init_state->mutable_particles(i)->set_ksi(ksi);
		}
		E3PetscSolver* solver = dynamic_cast<E3PetscSolver*>(solver_fact->createSolver(scfg, pcfg, init_state));

		MaxDetector md1;
		MaxDetector md2;
		int md_counter = 0;

		e_plot.reset();
		phi_plot.reset();
		a_plot.reset();
		hist_plot.reset();
		wa_plot.reset();
		na_plot.reset();
		an_plot.reset();
		ksin_plot.reset();

		const google::protobuf::Message *state_msg, *dstate_msg;
		double time = 0.0;
		solver->run(1000000, 1000, true);

		pb::E3Special special_msg;
		for(int step=0;;step++){
			if(!solver->step())
				break;
			state_msg = dynamic_cast<const google::protobuf::Message*>(solver->getState());
			dstate_msg = dynamic_cast<const google::protobuf::Message*>(solver->getDState());
			time = solver->getTime();

			model_e3_fill_spec_msg(
				dynamic_cast<const E3State*>(solver->getState()),
				dynamic_cast<const E3State*>(solver->getDState()),
				pcfg, &special_msg);

			e_plot.processState(state_msg, dstate_msg, time);
			phi_plot.processState(state_msg, dstate_msg, time);
			a_plot.processState(state_msg, dstate_msg, time);
			hist_plot.processState(&special_msg, NULL, time);
			wa_plot.processState(&special_msg, NULL, time);
			na_plot.processState(&special_msg, NULL, time);
			an_plot.processState(state_msg, dstate_msg, time);
			ksin_plot.processState(state_msg, dstate_msg, time);

			const E3State* estate = dynamic_cast<const E3State*>(solver->getState());
			double e = estate->e();
			md1.push(e);
			md2.push(-e);

			if(step==0 || md1.hasMax() || md2.hasMax()){
				std::ostringstream dir;
				dir << "exp_4_gyro/" << e_step << "/" << md_counter << "/";
				mkdir(dir.str().c_str(), 0777);

				a_plot.saveSerie(0, dir.str() + "/a.csv", state_msg, dstate_msg, time);
				hist_plot.saveSerie(0, dir.str() + "/hist.csv", &special_msg, NULL, time);
				an_plot.saveSerie(0, dir.str() + "/an.csv", state_msg, dstate_msg, time);
				ksin_plot.saveSerie(0, dir.str() + "/ksinn.csv", state_msg, dstate_msg, time);

				md_counter++;
			}// if

			if(time >= 13.9)
				break;
		}//for steps

		std::ostringstream dir;
		dir << "exp_4_gyro/" << e_step << "/";

		wa_plot.saveSerie(0, dir.str() + "/Wa.csv");
		wa_plot.saveSerie(1, dir.str() + "/Wb.csv");
		wa_plot.saveSerie(2, dir.str() + "/aver_a_2.csv");
		wa_plot.saveSerie(3, dir.str() + "/e_2.csv");

		na_plot.saveSerie(0, dir.str() + "/Na.csv");
		na_plot.saveSerie(1, dir.str() + "/Nb.csv");

		phi_plot.saveSerie(0, dir.str() + "/phi.csv");

		delete solver;
		delete init_state;
	}// for a0s and e0s
}

void exp_ts4_super_emission(){
	string problem_name = "model e4";

	OdeProblem* inst_fact = OdeProblemManager::getInstance()->getProblem( problem_name );
	OdeSolverFactory* solver_fact = *OdeSolverFactoryManager::getInstance()->getTypesFor(inst_fact).first;

	E4Config* pcfg = dynamic_cast<E4Config*>(inst_fact->createConfig());
		pcfg->set_n(1000);
	EXPetscSolverConfig* scfg = dynamic_cast<EXPetscSolverConfig*>(solver_fact->createSolverConfg());
		scfg->set_init_step(0.01);
		scfg->set_atol(1e-8);
		scfg->set_rtol(1e-8);
		scfg->set_n_cores(2);

	vector<double> alphas, e0s;
//	alphas.push_back(0.1);
//	alphas.push_back(0.05);
	alphas.push_back(0.02);

//	e0s.push_back(0.5);
//	e0s.push_back(0.4);
//	e0s.push_back(0.3);
//	e0s.push_back(0.2);
//	e0s.push_back(0.1);
	e0s.push_back(0.05);
	e0s.push_back(0.02);
	e0s.push_back(0.01);

	Gnuplot e_plot;
		e_plot.addVar("E");
		e_plot.setXAxisTime();
		e_plot.setTitle("E");

	Gnuplot phi_plot;
		phi_plot.addVar("phi");
		phi_plot.setXAxisTime();
		phi_plot.setTitle("phi");

	Gnuplot a_plot;
		a_plot.addVar("particles.a");
		a_plot.setXAxisVar("particles.psi");
		a_plot.setPolar(true);
		a_plot.setTitle("a|psi");

	int counter = 21;
	string dir = "exp_ts4_super_emission/";
	mkdir(dir.c_str(), 0777);
	FILE* fp = fopen((dir+"results.txt").c_str(), "ab");

	for(int alpha_step=0; alpha_step<alphas.size(); alpha_step++){
		double alpha = alphas[alpha_step];
		pcfg->set_alpha(alpha);
		for(int e_step=0; e_step<e0s.size(); e_step++){

			double a0 = 1.0;
			double e0 = e0s[e_step];

			E4State* init_state = dynamic_cast<E4State*>(inst_fact->createState(pcfg));
			init_state->set_e(e0);
			init_state->set_phi(0);
			double right = M_PI, left = -M_PI;
			for(int i=0; i<pcfg->n(); i++){
				init_state->mutable_particles(i)->set_a(a0);
				//double ksi = i / (double)pcfg->m() * (right-left) + left + rand()/(double)RAND_MAX*1e-4;
				double psi = rand()/(double)RAND_MAX * (right-left) + left;
				init_state->mutable_particles(i)->set_psi(psi);
			}
			E4PetscSolver* solver = dynamic_cast<E4PetscSolver*>(solver_fact->createSolver(scfg, pcfg, init_state));

			MaxDetector md1;
			MaxDetector md2;
			int md_counter = 0;

			e_plot.reset();
			phi_plot.reset();
			a_plot.reset();

			const google::protobuf::Message *state_msg, *dstate_msg;
			double time = 0.0;
			solver->run(1000000, 10000, true);

			pb::E3Special special_msg;
			for(int step=0;;step++){
				if(!solver->step())
					break;
				state_msg = dynamic_cast<const google::protobuf::Message*>(solver->getState());
				dstate_msg = dynamic_cast<const google::protobuf::Message*>(solver->getDState());
				time = solver->getTime();

				if(step % 10 == 0){
					e_plot.processState(state_msg, dstate_msg, time);
					phi_plot.processState(state_msg, dstate_msg, time);
					a_plot.processState(state_msg, dstate_msg, time);
				}

				const E4State* estate = dynamic_cast<const E4State*>(solver->getState());
				double e = estate->e();

				if(md1.push(e))
					md2.push(md1.get(1));

				if(md2.hasMax() && e > 0.3 && e > e0*1.1){
					std::ostringstream dir;
					dir << "exp_ts4_super_emission/" << counter << "/";
					mkdir(dir.str().c_str(), 0777);

					a_plot.saveSerie(0, dir.str() + "/a.csv", state_msg, dstate_msg, time);

					fprintf(fp, "%d\t%lf\t%lf\t%lf\t%lf\n", counter, alpha, e0, md2.getMax(), time);
					fflush(fp);

					counter++;
					break;
				}// if
			}//for steps

			std::ostringstream dir;
			dir << "exp_ts4_super_emission/" << (counter-1) << "/";

			e_plot.saveSerie(0, dir.str() + "/e.csv");
			phi_plot.saveSerie(0, dir.str() + "/phi.csv");

			delete solver;
			delete init_state;
		}// for e0s
	}// for alphas

	fclose(fp);
}
