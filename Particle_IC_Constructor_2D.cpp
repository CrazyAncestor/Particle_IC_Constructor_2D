#include "Particle_IC_Constructor_2D.h"
double Models_NEWTON_G;
double Models_rho;
double Models_r;
double Models_maxr;
double alpha;
int  Models_massprofnbin;

double *Table_MassProf_r_Models;
double *Table_MassProf_M_Models;
double *Table_MassProf_rho_Models;
double *Table_MassProf_rhodr_Models;
double *Table_MassProf_derho_overdx_Models;
double *Table_MassProf_g_Models;
double *Table_MassProf_pot_Models;

double Particle_IC_Constructor_2D::randomReal(double low,double high){
  std::random_device rd;

    /* 梅森旋轉演算法 */
  std::mt19937 generator( rd() );

  std::uniform_real_distribution<double> unif(0.0, 1.0);
  double x = unif(generator);
  
  return x*(high-low)+low;
}
double SQR(double a){
  return pow(a,2.0);
}
double CUBE(double a){
  return pow(a,3.0);
}

Particle_IC_Constructor_2D::Particle_IC_Constructor_2D()
{

}

Particle_IC_Constructor_2D::~Particle_IC_Constructor_2D()
{

}
//Input Parameter




//statistics
double Particle_IC_Constructor_2D::ave(double* a,int start,int fin){
  double sum=0;
  for(int k=start;k<fin;k++){
    sum+=a[k];
  }
  return sum/(fin-start);
}
double Particle_IC_Constructor_2D::var_n(double* a,int start,int fin){
  double sum=0;
  for(int k=start;k<fin;k++){
    sum+=(a[k])*(a[k]);
  }
  sum=sum-(fin-start)*pow(ave(a,start,fin),2);
  return sum;
}
double Particle_IC_Constructor_2D::cor(double* x,double* y,int start,int fin){
  double up=0,down = pow(var_n(x,start,fin)*var_n(y,start,fin),0.5);
  double ave_x = ave(x,start,fin),ave_y = ave(y,start,fin);
  for(int k=start;k<fin;k++){
    up+=(x[k]-ave_x)*(y[k]-ave_y);
  }
  return up/down;
}
void Particle_IC_Constructor_2D::mask(double* x,int start,int fin){
  double standard=3;
  for(int j=start;j<fin;j++){
    bool flag=0;
    for(int k=start;k<fin;k++){
      double test_fac;
      if(x[k]!=0){
        test_fac=fabs(x[j]/x[k]);
        if(test_fac>standard)flag=1;
      }
      if(flag)x[j]=0;
    }
  }
}
void Particle_IC_Constructor_2D::add_num(double* x,int start,int fin){
  double sum=0;
  int num=0;
  for(int j=start;j<fin;j++){
    if(x[j]!=0)sum+=x[j];
    num++;
  }
  double ave_x;
  if(num!=0){
    ave_x=sum/(num+0.0);
    for(int j=start;j<fin;j++){
      if(x[j]==0)x[j]=ave_x;
    }
  }
}
void Particle_IC_Constructor_2D::smooth_all(double* x,int start,int fin){
  int num=10;
  for(int k=start;k<fin-num+1;k++){
    mask(x,k,k+num);
  }
  for(int k=start;k<fin-num+1;k++){
    add_num(x,k,k+num);
  }
}

double Particle_IC_Constructor_2D::slope(double* x,double* y,int start,int fin){
  double cor_ = cor(x,y,start,fin);
  double var_n_x =var_n(x,start,fin), var_n_y =var_n(y,start,fin);
  double s =cor_*pow(var_n_y,0.5)/pow(var_n_x,0.5);

  return s;
}
//debug
double potential_Models(double x){
  if(x>double(Models_maxr/Models_r))return Table_MassProf_pot_Models[Models_massprofnbin-1]*(Models_maxr)/(x*Models_r);
  return Interpolation( Models_massprofnbin, Table_MassProf_r_Models, Table_MassProf_pot_Models, x*Models_r );  
}


double psi_potential_Models(double x, void * parameters){
  double psi= *(double *)parameters;
  return psi+potential_Models(x);
}
double inverse_psi_to_ind_Models (double psi) {
  int max=Models_massprofnbin-1;
  int min =0;
  int mid =(max+min)/2;
  double mid_psi;

  while(true){
    if(max==min+1){
      return max;
    }
    mid_psi=- Table_MassProf_pot_Models[mid] ;
    if(psi>mid_psi){
      max=mid;
      mid =(max+min)/2;
    }
    else{
      min=mid;
      mid =(max+min)/2;
    }
  }

}
double inverse_psi_to_x_Models (double psi) {
  double psi1=psi;
  int status;
  int iter = 0, max_iter = 100;
  const gsl_root_fsolver_type *T;
  gsl_root_fsolver *s;
  double x0= 0;
  double x_lo =0.0001, x_hi = 100000000000.0;
  gsl_function F;
  
  F.function = &psi_potential_Models;
  F.params=&psi1;

  T = gsl_root_fsolver_brent;
  s = gsl_root_fsolver_alloc (T);
  gsl_root_fsolver_set (s, &F, x_lo, x_hi);

  do
  {
    iter++;
    status = gsl_root_fsolver_iterate (s);
    x0= gsl_root_fsolver_root (s);
    x_lo = gsl_root_fsolver_x_lower (s);
    x_hi = gsl_root_fsolver_x_upper (s);
    status = gsl_root_test_interval (x_lo, x_hi,0, 0.001);
  }
  while (status == GSL_CONTINUE && iter < max_iter);
  gsl_root_fsolver_free (s);
  return x0;
}
double Particle_IC_Constructor_2D::integration_eng_base_Models(double eng){
  double min =  eng_min_Models;
  double max = eng;
  int num=1000;

  double dx=(max-min)/num;
  double result_right=0,result_left=0,result_simpson=0;
  double result = 0;
  for(int i=0;i<num;i++){
    double psi_l = min+i*dx,psi_r = min+(i+1)*dx;
    int ind0 = inverse_psi_to_ind_Models(min+(i+0.5)*dx);
    if(i==num-1)result += -2* Table_MassProf_derho_overdx_Models[ind0] * ( pow(eng-psi_l,0.5) );
    else result += -2* Table_MassProf_derho_overdx_Models[ind0] * ( pow(eng-psi_l,0.5) - pow(eng-psi_r,0.5) );
  }
  return result;
}

//Calculate Mass
//Different Model Type
//Plummer
double mass_base_Plummer(double x,void* nothing){
    return 2*M_PI*pow(Models_r,2)*(Models_rho*pow(x,1)*pow(1+x*x,-2.5));//important
}
double mass_base_Plummer_trunc(double x,void* trunc_fac){
  double fac = *(double *) trunc_fac;
  double x0 = fac*Models_maxr/Models_r;
  double xmax = Models_maxr/Models_r;
  
  if(x<x0) return 2*M_PI*pow(Models_r,2)*(Models_rho*pow(x,1)*pow(1+x*x,-2.5));//important
  else {
    double rho0 = Models_rho*pow(x0,2)*pow(1+x0,-2.5);
    double rho = rho0 *( 1 - pow(1-pow((x-xmax)/(xmax-x0) ,2) ,0.5 ) );
    return 4*M_PI*pow(Models_r*x,2)* Models_r *rho;
  }
}

//NFW
double mass_base_NFW(double x,void* nothing){
    return 2*M_PI*pow(Models_r,2)*(Models_rho*(1/((1+x)*(1+x))));//important
}
double mass_base_NFW_trunc(double x,void* trunc_fac){
  double fac = *(double *) trunc_fac;
  double x0 = fac*Models_maxr/Models_r;
  double xmax = Models_maxr/Models_r;
  
  if(x<x0) return 2*M_PI*pow(Models_r,2)*(Models_rho*(1/((1+x)*(1+x))));//important
  else {
    double rho0 = Models_rho*(1/(x0*(1+x0)*(1+x0)));
    double rho = rho0 *( 1 - pow(1-pow((x-xmax)/(xmax-x0) ,2) ,0.5 ) );
    return 4*M_PI*pow(Models_r*x,2)* Models_r *rho;
  }
}

//Burkert
double mass_base_Burkert(double x,void* nothing){
    return 2*M_PI*pow(Models_r,2)*(Models_rho*(x*(1/((1+x)*(1+x*x)))));//important
}
double mass_base_Burkert_trunc(double x,void* trunc_fac){
  double fac = *(double *) trunc_fac;
  double x0 = fac*Models_maxr/Models_r;
  double xmax = Models_maxr/Models_r;
  
  if(x<x0) return 2*M_PI*pow(Models_r,2)*(Models_rho*(x*(1/((1+x)*(1+x*x)))));//important
  else {
    double rho0 = Models_rho*(1/((1+x0)*(1+x0*x0)));
    double rho = rho0 *( 1 - pow(1-pow((x-xmax)/(xmax-x0) ,2) ,0.5 ) );
    return 4*M_PI*pow(Models_r*x,2)* Models_r *rho;
  }
}

//Jaffe
double mass_base_Jaffe(double x,void* nothing){
    return 2*M_PI*pow(Models_r,2)*(Models_rho*(1/(1+x)/x));//important
}
double mass_base_Jaffe_trunc(double x,void* trunc_fac){
  double fac = *(double *) trunc_fac;
  double x0 = fac*Models_maxr/Models_r;
  double xmax = Models_maxr/Models_r;
  
  if(x<x0) return 2*M_PI*pow(Models_r,2)*(Models_rho*(1/(1+x)/x));//important
  else {
    double rho0 = Models_rho*(1/(x0*x0*(1+x0)));
    double rho = rho0 *( 1 - pow(1-pow((x-xmax)/(xmax-x0) ,2) ,0.5 ) );
    return 4*M_PI*pow(Models_r*x,2)* Models_r *rho;
  }
}

//Hernquist
double mass_base_Hernquist(double x,void* nothing){
    return 2*M_PI*pow(Models_r,2)*(Models_rho*(1/((1+x)*(1+x)*(1+x)*x)));//important
}
double mass_base_Hernquist_trunc(double x,void* trunc_fac){
  double fac = *(double *) trunc_fac;
  double x0 = fac*Models_maxr/Models_r;
  double xmax = Models_maxr/Models_r;
  
  if(x<x0) return 2*M_PI*pow(Models_r,2)*(Models_rho*(1/((1+x)*(1+x)*(1+x)*x)));//important
  else {
    double rho0 = Models_rho*(1/(x0*(1+x0)*(1+x0)*(1+x0)));
    double rho = rho0 *( 1 - pow(1-pow((x-xmax)/(xmax-x0) ,2) ,0.5 ) );
    return 4*M_PI*pow(Models_r*x,2)* Models_r *rho;
  }
}

//Einasto
double mass_base_Einasto(double x,void *nothing){
  return 2*M_PI*Models_rho*pow(Models_r,2)*pow(x,1) *exp(-pow(x,alpha));//important
}
double mass_base_Einasto_trunc(double x,void *nothing){
  return 2*M_PI*Models_rho*pow(Models_r,2)*pow(x,1) *exp(-pow(x,alpha));//important
}
double test(double x,void *nothing){
  return 2*M_PI*Models_rho*pow(Models_r,2)*pow(x,1) *exp(-pow(x,alpha));//important
}
double Particle_IC_Constructor_2D::set_rho(double x){
  if (model_type == "UNKNOWN"){
    if(x>=Table_MassProf_r_Models[Models_massprofnbin-1])return Table_MassProf_rho_Models[Models_massprofnbin-1];
    return Interpolation( Models_massprofnbin, Table_MassProf_r_Models, Table_MassProf_rho_Models, x*Models_r );
  }

  else{
    double rho;
    double* nothing;
    if(Trunc_Flag){
      if(model_type=="Plummer")rho=mass_base_Plummer_trunc(x,nothing);
      else if(model_type=="NFW")rho=mass_base_NFW_trunc(x,nothing);
      else if(model_type=="Burkert")rho=mass_base_Burkert_trunc(x,nothing);
      else if(model_type=="Jaffe")rho=mass_base_Jaffe_trunc(x,nothing);
      else if(model_type=="Hernquist")rho=mass_base_Hernquist_trunc(x,nothing);
      else if(model_type=="Einasto")rho=mass_base_Einasto_trunc(x,nothing);

    }
    else {
      if(model_type=="Plummer")rho=mass_base_Plummer(x,nothing);
      else if(model_type=="NFW")rho=mass_base_NFW(x,nothing);
      else if(model_type=="Burkert")rho=mass_base_Burkert(x,nothing);
      else if(model_type=="Jaffe")rho=mass_base_Jaffe(x,nothing);
      else if(model_type=="Hernquist")rho=mass_base_Hernquist(x,nothing);
      else if(model_type=="Einasto")rho=mass_base_Einasto(x,nothing);
    }
    return rho/(2*M_PI*pow(x,1)*pow(Models_r,2));//important
  }
  
}
double Particle_IC_Constructor_2D::set_mass(double r){
  
  double x = r/Models_r;
  if (model_type == "UNKNOWN"){
    if(r>=Table_MassProf_r_Models[Models_massprofnbin-1])return Table_MassProf_M_Models[Models_massprofnbin-1];
    return Interpolation( Models_massprofnbin, Table_MassProf_r_Models, Table_MassProf_M_Models, r );
  }
  
  else{
    gsl_integration_workspace * w 
    = gsl_integration_workspace_alloc (1000);

    double  error;
    double result;
    gsl_function F;
    
    if(Trunc_Flag){
      if(model_type=="Plummer")F.function = &mass_base_Plummer_trunc;
      else if(model_type=="NFW")F.function = &mass_base_NFW_trunc;
      else if(model_type=="Burkert")F.function = &mass_base_Burkert_trunc;
      else if(model_type=="Jaffe")F.function = &mass_base_Jaffe_trunc;
      else if(model_type=="Hernquist")F.function = &mass_base_Hernquist_trunc;
      else if(model_type=="Einasto")F.function = &mass_base_Einasto_trunc;
    
      F.params =&Trunc_Fac;
      gsl_integration_qag  (&F, 0, x, 0, 1e-7, 1000, 1, w, &result,  &error);
      gsl_integration_workspace_free (w);
      
      return result;
    }
    else {
      if(model_type=="Plummer")F.function = &mass_base_Plummer;
      else if(model_type=="NFW")F.function = &mass_base_NFW;
      else if(model_type=="Burkert")F.function = &mass_base_Burkert;
      else if(model_type=="Jaffe")F.function = &mass_base_Jaffe;
      else if(model_type=="Hernquist")F.function = &mass_base_Hernquist;
      else if(model_type=="Einasto")F.function = &mass_base_Einasto;
      
      gsl_integration_qag  (&F, 0, x, 0, 1e-7, 1000, 1, w, &result,  &error);
      gsl_integration_workspace_free (w);
      
      return result;
    }
  }
  
}
void Particle_IC_Constructor_2D::initialize_mass_UNKNOWN(int Models_massprofnbin){
  
  //Mass
  Table_MassProf_M_Models[0]=0;
  double rho,dr,r;
  for (int b=1; b<Models_massprofnbin; b++)
  {
    rho = (Table_MassProf_rho_Models[b] + Table_MassProf_rho_Models[b-1])/2;
    dr = Table_MassProf_r_Models[b] - Table_MassProf_r_Models[b-1];
    r = (Table_MassProf_r_Models[b] + Table_MassProf_r_Models[b-1])/2;
    Table_MassProf_M_Models[b] = Table_MassProf_M_Models[b-1] + 2*M_PI*pow(r,1) *rho * dr;//important

  }

  //Rhodr
  Table_MassProf_rhodr_Models[0]=(Table_MassProf_rho_Models[1]-Table_MassProf_rho_Models[0])/(Table_MassProf_r_Models[1]-Table_MassProf_r_Models[0]);
  for (int b=1; b<Models_massprofnbin-1; b++)
  {
    int num=3;
    if(b==0)Table_MassProf_rhodr_Models[b] = slope(Table_MassProf_r_Models,Table_MassProf_rho_Models,0,num/2+1);
    else if(b==1)Table_MassProf_rhodr_Models[b] = slope(Table_MassProf_r_Models,Table_MassProf_rho_Models,0,num/2+2);
    
    else if(b==Models_massprofnbin-2)Table_MassProf_rhodr_Models[b] = slope(Table_MassProf_r_Models,Table_MassProf_rho_Models,Models_massprofnbin-num/2-1,Models_massprofnbin);
    else Table_MassProf_rhodr_Models[b] = slope(Table_MassProf_r_Models,Table_MassProf_rho_Models,b-num/2,b+num/2+1);
    

    
  }Table_MassProf_rhodr_Models[Models_massprofnbin-1]=Table_MassProf_rhodr_Models[Models_massprofnbin-2];
  
}

void Particle_IC_Constructor_2D::initialize_pot_UNKNOWN(int Models_massprofnbin){
  
  

  Table_MassProf_g_Models[0] =0;
  for (int b=1; b<Models_massprofnbin; b++)
  {
    Table_MassProf_g_Models[b] = -2*Models_NEWTON_G*Table_MassProf_M_Models[b]/pow(Table_MassProf_r_Models[b],1); //important
  }
  //Pot
  Table_MassProf_pot_Models[Models_massprofnbin-1] = 0;//-Models_NEWTON_G*Table_MassProf_M_Models[Models_massprofnbin-1]/Table_MassProf_r_Models[Models_massprofnbin-1];//important
  eng_min_Models = -Table_MassProf_pot_Models[Models_massprofnbin-1];
  for (int b=Models_massprofnbin-2;b>0;b--)
  {
    double dr = Table_MassProf_r_Models[b+1]-Table_MassProf_r_Models[b];
    
    Table_MassProf_pot_Models[b] = Table_MassProf_pot_Models[b+1] + Table_MassProf_g_Models[b] * dr;
    
  }Table_MassProf_pot_Models[0]=Table_MassProf_pot_Models[1];
    
  //derho_overdx
  for (int b=0; b<Models_massprofnbin; b++)
  {
    Table_MassProf_derho_overdx_Models[b] = -Table_MassProf_rhodr_Models[b]/(Table_MassProf_g_Models[b]);
  }
  
}

void Particle_IC_Constructor_2D::initialize_mass_others(){
  
  double dr = Models_maxr / (Models_massprofnbin-1);
  //Radius & Mass
  for (int b=0; b<Models_massprofnbin; b++)
  {
    
    Table_MassProf_r_Models[b] = dr*b;
    Table_MassProf_M_Models[b] = set_mass( Table_MassProf_r_Models[b]);
    
  }
  
  //Rho
  for (int b=1; b<Models_massprofnbin; b++)
  {
    double x = dr*b/Models_r;
    Table_MassProf_rho_Models[b] =set_rho(x);
  }
  Table_MassProf_rho_Models[0] = Table_MassProf_rho_Models[1];

  //Rhodr
  Table_MassProf_rhodr_Models[0]=(Table_MassProf_rho_Models[1]-Table_MassProf_rho_Models[0])/(dr);
  for (int b=1; b<Models_massprofnbin-1; b++)
  {
    int num=3;
    if(b==0)Table_MassProf_rhodr_Models[b] = slope(Table_MassProf_r_Models,Table_MassProf_rho_Models,0,num/2+1);
    else if(b==1)Table_MassProf_rhodr_Models[b] = slope(Table_MassProf_r_Models,Table_MassProf_rho_Models,0,num/2+2);
    
    else if(b==Models_massprofnbin-2)Table_MassProf_rhodr_Models[b] = slope(Table_MassProf_r_Models,Table_MassProf_rho_Models,Models_massprofnbin-num/2-1,Models_massprofnbin);
    else Table_MassProf_rhodr_Models[b] = slope(Table_MassProf_r_Models,Table_MassProf_rho_Models,b-num/2,b+num/2+1);
    
  }Table_MassProf_rhodr_Models[Models_massprofnbin-1]=Table_MassProf_rhodr_Models[Models_massprofnbin-2];
  
}

void Particle_IC_Constructor_2D::initialize_pot_others(){

  double dr = Models_maxr / (Models_massprofnbin-1);

  Table_MassProf_g_Models[0] =0;
  for (int b=1; b<Models_massprofnbin; b++)
  {
    Table_MassProf_g_Models[b] = -2*Models_NEWTON_G*Table_MassProf_M_Models[b]/pow(Table_MassProf_r_Models[b],1);//important
    
  }
  //Pot
  Table_MassProf_pot_Models[Models_massprofnbin-1] = 0;//-Models_NEWTON_G*Table_MassProf_M_Models[Models_massprofnbin-1]/Table_MassProf_r_Models[Models_massprofnbin-1];//important
  eng_min_Models = -Table_MassProf_pot_Models[Models_massprofnbin-1];
  for (int b=Models_massprofnbin-2;b>0;b--)
  {
    Table_MassProf_pot_Models[b] = Table_MassProf_pot_Models[b+1] + Table_MassProf_g_Models[b] * dr;
    
  }Table_MassProf_pot_Models[0]=Table_MassProf_pot_Models[1];
    
  //derho_overdx
  for (int b=0; b<Models_massprofnbin; b++)
  {
    Table_MassProf_derho_overdx_Models[b] = -Table_MassProf_rhodr_Models[b]/(Table_MassProf_g_Models[b]);
  }
  
}
void Particle_IC_Constructor_2D::initialize_prob_dens(){
  double min,max;
  min=-Table_MassProf_pot_Models[Models_massprofnbin-1];
  max =-Table_MassProf_pot_Models[1];
  delta =(max-min)/size_Models;
  double eng=min;

  for(int k =0;k<size_Models;k++){
    psi[k] = eng;
    int_prob_dens[k] = integration_eng_base_Models(eng);
    eng +=delta;
  }
  for(int k =0;k<size_Models;k++){

    if(k==0)prob_dens[k]=slope(psi,int_prob_dens,k,k+5);
    else if(k==1)prob_dens[k]=slope(psi,int_prob_dens,k-1,k+4);

    else if(k==size_Models-2)prob_dens[k]=slope(psi,int_prob_dens,k-3,k+2);
    else if(k==size_Models-1)prob_dens[k]=slope(psi,int_prob_dens,k-4,k+1);

    else prob_dens[k]=slope(psi,int_prob_dens,k-2,k+3);

    if(prob_dens[k]<0)prob_dens[k]=0;
    
  }
  smooth_all(prob_dens,0,size_Models);
}

void Particle_IC_Constructor_2D::init(string type,double al,double newton_g,double rho,double r,int nbin,double rmax,int rseed,bool trunc_flag,double trunc_fac,int r_col,int rho_col,const char* Filename){
  Table_MassProf_r_Models=NULL;
  Table_MassProf_M_Models=NULL;
  Table_MassProf_rho_Models=NULL;
  Table_MassProf_rhodr_Models=NULL;
  Table_MassProf_derho_overdx_Models=NULL;
  Table_MassProf_g_Models=NULL;
  Table_MassProf_pot_Models=NULL;

  model_type = type;
  alpha = al;
  Models_NEWTON_G=newton_g;
  Models_rho=rho;
  Models_r=r;
  Models_massprofnbin=nbin;
  
  Models_maxr=rmax;

  Trunc_Flag=trunc_flag;
  Trunc_Fac=trunc_fac;
  
  
  //Initialize densities with Table
  if(model_type=="UNKNOWN"){
    int Tcol_r[1]={r_col};
    int Tcol_rho[1]={rho_col};
    int row_r_Models;
    row_r_Models= LoadTable( Table_MassProf_r_Models, Filename, 1, Tcol_r,true );
    
    int row_density_Models;
    row_density_Models= LoadTable( Table_MassProf_rho_Models, Filename, 1, Tcol_rho,true );

    Models_massprofnbin=row_r_Models;

    Table_MassProf_M_Models = new double [Models_massprofnbin];
    Table_MassProf_rhodr_Models = new double [Models_massprofnbin];
    Table_MassProf_g_Models = new double [Models_massprofnbin];
    Table_MassProf_pot_Models = new double [Models_massprofnbin];
    Table_MassProf_derho_overdx_Models = new double [Models_massprofnbin];
    
    initialize_mass_UNKNOWN(Models_massprofnbin);
    initialize_pot_UNKNOWN(Models_massprofnbin);
    initialize_prob_dens();

  }

  else{
    Table_MassProf_r_Models = new double [Models_massprofnbin];
    Table_MassProf_M_Models = new double [Models_massprofnbin];
    Table_MassProf_rho_Models = new double [Models_massprofnbin];
    Table_MassProf_rhodr_Models = new double [Models_massprofnbin];
    Table_MassProf_g_Models = new double [Models_massprofnbin];
    Table_MassProf_pot_Models = new double [Models_massprofnbin];
    Table_MassProf_derho_overdx_Models = new double [Models_massprofnbin];
    
    initialize_mass_others();
    initialize_pot_others();
    initialize_prob_dens();
  }


  
}
double Particle_IC_Constructor_2D::set_vel(double x){ 
  double rho_high = Interpolation(Models_massprofnbin,Table_MassProf_r_Models,Table_MassProf_rho_Models,x*Models_r);
  double rho_low = Table_MassProf_rho_Models[Models_massprofnbin-1];
  double rho_rand = randomReal(rho_low,rho_high);
  double pot_chosen = Interpolation(Models_massprofnbin,Table_MassProf_rho_Models,Table_MassProf_pot_Models,rho_rand);
  return pow(2*fabs(pot_chosen-potential_Models(x)),0.5);
}  
double Particle_IC_Constructor_2D::set_radius(){  
  double Enclosed_mass=Table_MassProf_M_Models[Models_massprofnbin-1];

  double x = randomReal(0.,1.);

  double M_rand = x*Enclosed_mass;//randomReal(0., 1.0)*Enclosed_mass;
  
  double index = 0; 
  double radius = Interpolation(Models_massprofnbin,Table_MassProf_M_Models,Table_MassProf_r_Models,M_rand);
  
  return radius;
}  
double Particle_IC_Constructor_2D::set_vel_test(double r){  
  const double TotM_Inf    = 4.0/3.0*M_PI*CUBE(Models_r)*Models_rho;
  const double Vmax_Fac    = sqrt( 2.0*Models_NEWTON_G*TotM_Inf );

  double  Vmax, RanV, RanProb, Prob;

  Vmax = Vmax_Fac*pow( SQR(Models_r) + SQR(r*Models_r), -0.25 );

  //       randomly determine the velocity amplitude (ref: Aarseth, S. et al. 1974, A&A, 37, 183: Eq. [A4,A5])
  do
  {
    RanV    = randomReal(0., 1.0);          // (0.0, 1.0)
    RanProb = randomReal(0., 1.0);         // (0.0, 0.1)
    Prob    = SQR(RanV)*pow( 1.0-SQR(RanV), 3.5 );  // < 0.1
  }
  while ( RanProb > Prob );
  
  //       randomly set the velocity vector with the given amplitude (RanV*Vmax)
  return RanV*Vmax;
}  

/*double Particle_IC_Constructor_2D::vel_dispersion(double x){   

  double index,norm=0,v2_norm = 0;
  double psi_per =-potential_Models(x);
  
  for(int k =0;k<size_Models;k++){
    
    if(psi[k]>psi_per){
      index =k-1;
      
      break;
    }
    
    norm += prob_dens[k] *pow(psi_per-psi[k],0.5) *delta;
    v2_norm += prob_dens[k] *pow(psi_per-psi[k],1.5) *delta;
  }
  
  return v2_norm/norm*2;
  return 0;
}  
double Particle_IC_Constructor_2D::specific_prob(double x,double vm){   

  double index,norm=0,wanted = 0;
  double psi_per =-potential_Models(x);
  bool cont = false;
  for(int k =0;k<size_Models;k++){
    double vx = pow(2*(psi_per-psi[k]),0.5);
    if(vx<vm){
      cont = true;
    }
    if(psi[k]>psi_per){
      break;
    }
    
    norm += prob_dens[k] *pow(psi_per-psi[k],0.5) *delta;
    if(cont)wanted += prob_dens[k] *pow(psi_per-psi[k],0.5) *delta;
  }
  
  return wanted/norm;
  return 0;
}  */
