/*
 * model_e1.cpp
 *
 *  Created on: Jan 4, 2014
 *      Author: dimalit
 */

#include "model_e1.h"

#include <map>
#include <limits>

E1InstanceFactory E1InstanceFactory::instance;
E1SolverFactory E1SolverFactory::instance;

extern PetscErrorCode InitialConditions(Vec);
extern PetscErrorCode RHSFunction(TS,PetscReal,Vec,Vec,void*);
extern PetscErrorCode PostStepFunction(TS ts);

// if they don't want to sore extra parameter - we will!
std::map<TS, E1PetscSolver*> PostStep_selfs;

E1PetscSolver::E1PetscSolver(const E1PetscSolverConfig* scfg, const E1Config* pcfg, const E1State* init_state){
	pconfig = new E1Config(*pcfg);
	sconfig = new E1PetscSolverConfig(*scfg);

	max_integral = -std::numeric_limits<double>::infinity();
	min_integral = +std::numeric_limits<double>::infinity();;

	PetscErrorCode ierr;

	ierr = VecCreateMPI(PETSC_COMM_WORLD, PETSC_DECIDE, 2*pconfig->g_m+2, &u);assert(!ierr);

	ierr = TSCreate(PETSC_COMM_WORLD,&ts);assert(!ierr);
	ierr = TSSetProblemType(ts,TS_NONLINEAR);assert(!ierr);
	ierr = TSSetRHSFunction(ts,RHSFunction,this);assert(!ierr);
	ierr = TSSetPostStep(ts, &PostStepFunction);assert(!ierr);
		PostStep_selfs[ts] = this;

	ierr = TSSetInitialTimeStep(ts,0.0,sconfig->g_step);assert(!ierr);

	ierr = TSSetType(ts,TSRUNGE_KUTTA);assert(!ierr);
	  ierr = TSRKSetTolerance(ts, sconfig->g_tol);assert(!ierr);
	ierr = TSSetDuration(ts,1,0);assert(!ierr);	// 1 step, max time
//	ierr = TSSetFromOptions(ts);assert(!ierr);

	//ierr = InitialConditions(u);assert(!ierr);
	VecDuplicate(init_state->u, &u);
	// Not sure about need of u being global
	ierr = TSSetSolution(ts,u);assert(!ierr);

}
double E1PetscSolver::getTime() const {
	PetscErrorCode ierr;
	PetscReal t;
	ierr = TSGetTime(ts, &t); assert(!ierr);
	return t;
}

const OdeState* E1PetscSolver::getCurrentState() const{
	E1State* s = new E1State(pconfig);
	s->u = u;
	return s;
}

E1PetscSolver::~E1PetscSolver(){
	PetscErrorCode ierr;
	ierr = TSDestroy(ts);assert(!ierr);
	ierr = VecDestroy(u);assert(!ierr);
}

void E1PetscSolver::step(){
	PetscErrorCode ierr;
	PetscInt steps;
	PetscReal time;
	ierr = TSStep(ts,&steps,&time);assert(!ierr);
}

PetscErrorCode RHSFunction(TS ts,PetscReal t,Vec global_in,Vec global_out,void *ctx)
{
  E1PetscSolver* self = (E1PetscSolver*)ctx;
  PetscErrorCode ierr;
  PetscInt       i;
  PetscScalar    *in_ptr,*out_ptr;

  int rank;
  MPI_Comm_rank(PETSC_COMM_WORLD, &rank);

  /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
     Get ready for local function computations
     - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  /*
      Access directly the values in our local arrays
  */
  ierr = VecGetArray(global_in,&in_ptr);assert(!ierr);
  ierr = VecGetArray(global_out,&out_ptr);assert(!ierr);

  /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
     Compute entries for the locally owned part
     - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  // 1 broadcast field
  PetscScalar wave[2];

  PetscInt begin, end;
  ierr = VecGetOwnershipRange(global_in, &begin, &end); assert(!ierr);
  // broadcast wave if rank 0
  if(begin==0){
	  wave[0] = in_ptr[0];
	  wave[1] = in_ptr[1];
  }
  MPI_Datatype scalar; PetscDataTypeToMPIDataType(PETSC_SCALAR, &scalar);
  MPI_Bcast(wave, 2, scalar, 0, PETSC_COMM_WORLD);// !!! rank = 0 always holds wave???

  PetscScalar sin_sum = 0, cos_sum = 0;
  // 2 iterate over particles
  for (i = begin; i < end; i+=2) {								// for wave
	  if(i==0 && begin==0)
		  continue;

	  PetscScalar s = sin(2*M_PI*in_ptr[i+1-begin]+wave[1]);	// sin(2 pi e + phi)
	  PetscScalar c = cos(2*M_PI*in_ptr[i+1-begin]+wave[1]);	// sin(2 pi e + phi)

	  // dt for particles
	  out_ptr[i-begin]   = - wave[0]/2 * s;							// - E / 2 * sin
	  out_ptr[i+1-begin] = - wave[0]/4/M_PI/in_ptr[i-begin] * c;// - E / 4PIa * cos

	  // dt for field
	  sin_sum += in_ptr[i-begin] * s;							// a * sin
	  cos_sum += in_ptr[i-begin] * c;							// a * cos
  }

  // 3 sum dt for wave
  wave[0] = sin_sum;
  wave[1] = cos_sum;
  MPI_Reduce(wave, out_ptr, 2, scalar, MPI_SUM, 0, PETSC_COMM_WORLD);		// reduce to out
  if(rank==0){
	  out_ptr[0] /= + self->pconfig->g_m;											// 1/N
	  out_ptr[1] /= + in_ptr[0]*self->pconfig->g_m;									// 1/(E*N)
//	  std::cout << "dE = " << out_ptr[0] << "\n";
//	  std::cout << "a = " << wave[1]/m << "\tdphi = " << out_ptr[1] << "\n";
//	  std::cout << "argsin = " << 2*M_PI*in_ptr[3]+in_ptr[1] << "\n";
   }

  // init drawing
  PetscDraw draw;
/*  PetscViewerDrawGetDraw(phase_diagram, 0, &draw);
  ierr = PetscDrawSetDoubleBuffer(draw);assert(!ierr);

  PetscDrawSP sp;
  PetscDrawAxis axis;
  const char *xlabel, *ylabel, *toplabel;
  xlabel = "ksi";
  ylabel = "dksi";
  toplabel = "ksi-dksi plot of particles";

  ierr = PetscDrawPointSetSize(draw, 0);assert(!ierr);

  ierr = PetscDrawSPCreate(draw,1,&sp);assert(!ierr);
  ierr = PetscDrawSPGetAxis(sp,&axis);assert(!ierr);
  ierr = PetscDrawAxisSetColors(axis,PETSC_DRAW_BLACK,PETSC_DRAW_RED,PETSC_DRAW_BLUE);assert(!ierr);
  ierr = PetscDrawAxisSetLabels(axis,toplabel,xlabel,ylabel);assert(!ierr);
  ierr = PetscDrawAxisSetLimits(axis, 0, 1, -4, 4);assert(!ierr);
  //ierr = PetscDrawAxisSetHoldLimits(axis,PETSC_TRUE);assert(!ierr);

  for (i=0; i<end-begin; i+=2) {
	  if(rank==0 && i==0)
		  continue;
	  PetscScalar xd, yd;
	  xd = in_ptr[i+1];
	  yd = out_ptr[i+1];
	  	  xd = fmod(100+xd, 1.0);	// return to 0..1
	  ierr = PetscDrawSPAddPoint(sp,&xd,&yd);assert(!ierr);
  }

  // finish drawing
  ierr = PetscDrawSPDraw(sp);assert(!ierr);
  ierr = PetscDrawSynchronizedFlush(draw);assert(!ierr);
  ierr = PetscDrawSPDestroy(sp);assert(!ierr);
*/
//  std::cout << rank << " final wave = " << wave.re << " " << wave.im << std::endl;

  /*
     Restore vectors
  */
  ierr = VecRestoreArray(global_in,&in_ptr);assert(!ierr);
  ierr = VecRestoreArray(global_out,&out_ptr);assert(!ierr);

  /* Print debugging information if desired */
//  if (debug) {
////	 VecView(global_out, field_viewer);
//     ierr = PetscPrintf(comm,"RHS function vector\n");assert(!ierr);
//     ierr = VecView(global_out,PETSC_VIEWER_STDOUT_WORLD);assert(!ierr);
//  }

  return 0;
}

PetscErrorCode PostStepFunction(TS ts){
	PetscErrorCode ierr;

	E1PetscSolver* self = PostStep_selfs[ts];

	int rank;
	MPI_Comm_rank(PETSC_COMM_WORLD, &rank);

	// get current solution
	Vec u;
	TSGetSolution(ts, &u);
	PetscScalar* localptr;
	PetscInt localsize;
	ierr = VecGetArray(u,&localptr);assert(!ierr);
	ierr = VecGetLocalSize(u,&localsize);assert(!ierr);

	////////// return ksi to 0..1
	for (int i = 0; i < localsize; i+=2) {
		if(rank==0 && i==0)
			continue;
		localptr[i+1] = fmod(100+localptr[i+1], 1.0);	// 100 is just big int
	}

	///////// find E^2 - 2 * 1/N+sum a^2
	if(rank==0){
		PetscScalar res = 0;
		for (int i = 2; i < localsize; i+=2) {
			res += localptr[i]*localptr[i];
		}
		PetscScalar energy_integral = localptr[0]*localptr[0]+2*res/self->pconfig->g_m;

		if(energy_integral > self->max_integral)
			self->max_integral = energy_integral;
		if(energy_integral < self->min_integral)
			self->min_integral = energy_integral;
	}

	ierr = VecRestoreArray(u,&localptr);assert(!ierr);
	return 0;
}
