#include "Particle_IC_Constructor.h"
#define box_size 0
#define PI 3.14159265
Particle_IC_Constructor   constructor_Models; 
//Parameters
int Models_RSeed        =   123 ;                 // random seed for setting particle position and velocity (>=0) [123]
double Models_Rho0          =  665.3026853292198;     // peak density [unit: M mass of sun/kpc^3]
double Models_R0            =  0.25 ;                 // scale radius [unit: kpc]
double Models_MaxR          =  6.0;                    // maximum radius for particles [unit: kpc]
double Models_CenterX       =  10    ;              // central x coordinate [box center]
double Models_CenterY       =  10    ;              // central y coordinate [box center]
double Models_CenterZ        = 10      ;            // central z coordinate [box center]
double Models_BulkVelX      =  0.014748155644339204 ;// bulk velocity in x
double Models_BulkVelY      =  -0.014748155644339204 ;// bulk velocity in y
double Models_BulkVelZ      =  0.0          ;        // bulk velocity in z
double Models_GasMFrac      =  0.001  ;              // gas mass fraction (0.0<input<=1.0; useless when PARTICLE is off), as small as possible [0.001]
int Models_MassProfNBin   = 1000    ;             // number of radial bins in the mass profile table [1000]
double Models_Alpha          =1     ;               // alpha parameter for Eiasto model [1]
int Models_r_col         = 0         ;           // number of the column of the radius of density profile, when model is "UNKNOWN"  [0]
int Models_rho_col        =1          ;          // number of the column of the density of density profile, when model is "UNKNOWN"  [1]
double Models_truncation    = 0       ;             // whether to turn on a smoothy truncation function of density near MaxR [0]

int main(){
    double Newton_G = 1.0;
    constructor_Models.init("UNKNOWN",Models_Alpha,1.0,Models_Rho0,Models_R0,Models_MassProfNBin,Models_MaxR,Models_RSeed,Models_truncation,0.7,Models_r_col,Models_rho_col,"profile_halo.txt");

    //Example Code for Constructing Particle ICs (May be modified in your present code)
    const int N = 1000;
    double par_r [N];
    double par_vel[N];
    int count = 0;
    for (int p=0;p<N;p++){
        par_r[p] = constructor_Models.set_radius();
        //cout<<par_r[p]<<endl;
        par_vel[p] = constructor_Models.set_vel(p/1000.);//par_r[p]/Models_R0
        //cout<<par_vel[p]<<endl;
        //cout<<p<<endl;
        if (par_r[p]>Models_MaxR){
            count++;
        }
    }
    cout<<count<<endl;
}