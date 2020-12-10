/*----------------------------------------------------------------------------
 USE_REVOLVE -- Checkpointing approaches
 File:       use_revolve.cpp
  
 Copyright (c) Andreas Griewank, Andrea Walther, Philipp Stumm
  
 This software is provided under the terms of
 the Common Public License. Any use, reproduction, or distribution of the
 software constitutes recipient's acceptance of the terms of this license.
 See the accompanying copy of the Common Public License for more details.
----------------------------------------------------------------------------*/


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <iostream>
#include <sstream>
#include <iomanip>
#include <cstdlib>
#include <fstream>
#include <cmath>
#include <cstring>
#include "offline_schedule.h"

const double e = 2.7182818;

using namespace std;

double func_U(double t)
{
  return 2.*(pow(e,3.*t)-pow(e,3))/(pow(e,3.*t/2.)*(2.+pow(e,3)));
}

void func(double X[2],double t, double F[2])
{
  F[0] = 0.5*X[0]+ func_U(t);
  F[1] = X[0]*X[0]+0.5*(func_U(t)*func_U(t));
}

void func_lambda(double L[2], double X[2], double F[2])
{
  F[0] = -0.5*L[0]-2.*X[0]*L[1];
  F[1] = 0.;
}

void func_adj(double BF[2], double X[2], double BX[2])
{
  BX[0] = 0.5*BF[0]+2.*X[0]*BF[1];
  BX[1] = 0.;
}

void opt_sol(double Y[2],double t)
{
  Y[0] = (2.*pow(e,3.*t)+pow(e,3))/(pow(e,3.*t/2.)*(2.+pow(e,3)));
  Y[1] = (2.*pow(e,3.*t)-pow(e,6.-3.*t)-2.+pow(e,6))/(pow(2.+pow(e,3),2));
}

void opt_lambda(double L[2],double t)
{
  L[0] = (2.*pow(e,3-t)-2.*pow(e,2.*t))/(pow(e,t/2.)*(2+pow(e,3)));
  L[1] = 1.;
}

void advance(double F[2],double F_H[2],double t,double h)
{
  double k0[2],k1[2],G[2];

  func(F_H,t,k0);
  G[0] = F_H[0] + h/2.*k0[0];
  G[1] = F_H[1] + h/2.*k0[1];
  func(G,t+h/2.,k1);
  F[0] = F_H[0] + h*k1[0];
  F[1] = F_H[1] + h*k1[1];
}

void store(double F_H[2], double **F_C,double t,int i)
{
  F_C[0][i] = F_H[0]; 
  F_C[1][i] = F_H[1];
  F_C[2][i] = t;  
}

void restore(double F_H[2], double **F_C,double *t,int i)
{
  F_H[0] = F_C[0][i]; 
  F_H[1] = F_C[1][i];
  *t = F_C[2][i];
}


void adjoint(double L_H[2],double F_H[2],double L[2],double t,double h)
{
  double k0[2],k1[2],G[2],BH[2],Bk0[2],Bk1[2],BG[2];

  func(F_H,t,k0);
  G[0] = F_H[0] + h/2.*k0[0];
  G[1] = F_H[1] + h/2.*k0[1];
  func(G,t+h/2.,k1);
  L[0] = L_H[0];
  L[1] = L_H[1];
  Bk1[0] = h*L_H[0];
  Bk1[1] = h*L_H[1];
  func_adj(Bk1,G,BG);
  L[0] += BG[0];
  L[1] += BG[1];
  Bk0[0] = h/2.*BG[0];
  Bk0[1] = h/2.*BG[1];
  func_adj(Bk0,F_H,BH);
  L[0] += BH[0];
  L[1] += BH[1];
}

int main(int argc, char *argv[])
{
  // These variables merit exact and approximate solution 
  double F[2],L[2],F_opt[2],L_opt[2];
  double t,h,F_H[2],L_H[2],F_final[2];
  double **F_Check;

  cout << "**************************************************************************" << endl;
  cout << "*              Solution of the optimal control problem                   *" << endl;
  cout << "*                                                                        *" << endl;
  cout << "*                     J(y) = y_2(1) -> min                               *" << endl;
  cout << "*           s.t.   dy_1/dt = 0.5*y_1(t) + u(t),            y_1(0)=1      *" << endl;
  cout << "*                  dy_2/dt = y_1(t)^2 + 0.5*u(t)^2         y_2(0)=0      *" << endl;
  cout << "*                                                                        *" << endl;
  cout << "*                  the adjoints equations fulfill                        *" << endl;
  cout << "*                                                                        *" << endl;
  cout << "*         dl_1/dt = -0.5*l_1(t) - 2*y_1(t)*l_2(t)          l_1(1)=0      *" << endl;
  cout << "*         dl_2/dt = 0                                      l_2(1)=1      *" << endl;
  cout << "*                                                                        *" << endl;
  cout << "*   with Revolve for Online and (Multi-Stage) Offline Checkpointing      *" << endl;
  cout << "*                                                                        *" << endl;
  cout << "**************************************************************************" << endl;
  
  cout << "**************************************************************************" << endl;
  cout << "*        The solution of the optimal control problem above is            *" << endl;
  cout << "*                                                                        *" << endl;
  cout << "*        y_1*(t) = (2*e^(3t)+e^3)/(e^(3t/2)*(2+e^3))                     *" << endl;
  cout << "*        y_2*(t) = (2*e^(3t)-e^(6-3t)-2+e^6)/((2+e^3)^2)                 *" << endl;
  cout << "*          u*(t) = (2*e^(3t)-e^3)/(e^(3t/2)*(2+e^3))                     *" << endl;
  cout << "*        l_1*(t) = (2*e^(3-t)-2*e^(2t))/(e^(t/2)*(2+e^3))                *" << endl;
  cout << "*        l_2*(t) = 1                                                     *" << endl;
  cout << "*                                                                        *" << endl;
  cout << "**************************************************************************" << endl;

  cout << "\n \n Using Binomial Offline Checkpointing for the approximate solution: \n \n" ;

  if (argc < 4) {
    cout << " Run the exe with arguments: STEPS CHECKPOINTS INFO \n";
    cout << endl << "STEPS    -> number of time steps to perform" << endl;
    cout << "CHECKPOINTS    -> number of checkpoints" << endl;
    cout << "INFO = 1 -> calculate only approximate solution" << endl;
    cout << "INFO = 2 -> calculate approximate solution + takeshots" << endl;
    cout << "INFO = 3 -> calculate approximate solution + all information" << endl << endl;
    return -1;
  }

  //int check, capo, fine, steps,   checkpoints,oldcapo;
  int info,checkpoints,steps;
  int nextcheckpointstep,num_checkpoints_stored,num_checkpoints_avail;
  size_t pos;
  steps       = stoi(argv[1],&pos);
  checkpoints = stoi(argv[2],&pos);
  info        = stoi(argv[3],&pos);

  int *Stored_Step = new int [checkpoints];

  h = 1./steps;
  t = 0.;
  F[0] = 1.;
  F[1] = 0.;

  F_Check = new double *[3];
  for (int i=0; i<3; i++)
    F_Check[i] = new double [checkpoints];

  // forward sweep
  nextcheckpointstep = 0;
  num_checkpoints_stored = 0;
  for (int i=0; i<steps; i++) {
    if (i == nextcheckpointstep) {
      store(F,F_Check,t,num_checkpoints_stored);
      Stored_Step[num_checkpoints_stored++] = i;
      if(info > 1) {
        cout << " takeshot at " << setw(6) << i << " in CP " << num_checkpoints_stored-1 << endl;
      }
      num_checkpoints_avail = checkpoints - num_checkpoints_stored + 1;
      revolve(Stored_Step[num_checkpoints_stored-1],num_checkpoints_avail,steps,&nextcheckpointstep);
    }
    F_H[0] = F[0];
    F_H[1] = F[1];
    advance(F,F_H,t,h);
    t += h;
  }
  F_final[0] = F[0];
  F_final[1] = F[1];
  F[0] = F_H[0];
  F[1] = F_H[1];
  t = t - h;
  // backward sweep
  L[0] = 0.;
  L[1] = 1.;
  for (int i=steps; i>0; i--) {
    int rstep;

    if (i==steps) { // Intermediate info already in memory
      rstep = i - 1;
    } else { //Restore a checkpoint
      restore(F,F_Check,&t,num_checkpoints_stored-1);
      rstep = Stored_Step[num_checkpoints_stored-1];
      t = rstep*h;
      if(info > 1)
        cout << " restore at " << setw(7) << rstep << " in CP " << num_checkpoints_stored-1 << endl;
      if (i == rstep+1) { // can delete this checkpoint 
        num_checkpoints_stored--;
      }
      // Recompute from the restored point
      num_checkpoints_avail = checkpoints - num_checkpoints_stored + 1;
      revolve(rstep,num_checkpoints_avail,i,&nextcheckpointstep);
      for (int j=rstep; j<i-1; j++) {
        if (j == nextcheckpointstep) {
          store(F,F_Check,t,num_checkpoints_stored);
          Stored_Step[num_checkpoints_stored++] = j;
          if(info > 1) {
            cout << " takeshot at " << setw(6) << j << " in CP " << num_checkpoints_stored-1 << endl;
          }
          num_checkpoints_avail = checkpoints - num_checkpoints_stored + 1;
          revolve(Stored_Step[num_checkpoints_stored-1],num_checkpoints_avail,i,&nextcheckpointstep);
        }
        F_H[0] = F[0];
        F_H[1] = F[1];
        advance(F,F_H,t,h);
        t += h;
      }
    }
    // do an adjoint step
    L_H[0] = L[0];
    L_H[1] = L[1];
    adjoint(L_H,F,L,t,h);
    t = t - h;
    if(info > 1) {
      if(i==steps)
        cout << " firsturn at " << setw(6) << i-1 << endl;
      else
        cout << " youturn at " << setw(7) << i-1 << endl;
    }
  }
  
  for (int i=0; i<3; i++)
    delete [] F_Check[i];
  delete [] F_Check;
  delete [] Stored_Step;

  opt_sol(F_opt,1.);
  opt_lambda(L_opt,0.);
  cout << "\n\n";
  cout << "y_1*(1)  = " << setiosflags(ios::fixed) << setprecision(7) << F_opt[0] << "  ";
  cout << "y_2*(1)  = " << setiosflags(ios::fixed) << setprecision(7) << F_opt[1] << endl;  
  cout << "y_1 (1)  = " << setiosflags(ios::fixed) << setprecision(7) << F_final[0] << "  ";
  cout << "y_2 (1)  = " << setiosflags(ios::fixed) << setprecision(7) << F_final[1] << endl;  
  cout << "\n\n";
  cout << "l_1*(0)  = " << setiosflags(ios::fixed) << setprecision(7) << L_opt[0] << "  ";
  cout << "l_2*(0)  = " << setiosflags(ios::fixed) << setprecision(7) << L_opt[1] << endl;    
  cout << "l_1 (0)  = " << setiosflags(ios::fixed) << setprecision(7) << L[0]     << "  ";
  cout << "l_2 (0)  = " << setiosflags(ios::fixed) << setprecision(7) << L[1] << endl;
  return 0;  
}