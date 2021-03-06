/*
Ye Tian edited at 05-14-2021 
Rakitha Beminiwattha Fri Jul  1 11:08:43 EDT 2016
File name : HallD_gen_lund.cc uses proton target electro-production pions from Hall D generator to produce electro-production events for following target,
PVDIS H and D, SIDIS He, J/psi, SIDIS Proton, MOLLER proton. use targ_index parameter to select correct target

For now one has to set the hall d rootfile manually. use following rootfiles (files can be found at ifarm:/work/halla/solid/rakithab/halld/halld_rootfiles/) 

for PVDIS H and D, SIDIS 3He ,
  set1               bggen_output_1M_*.root
  set2 to 5          bggen_set*_output_1M_*.root
  
  for moller
    set1               bggen_set1_output_MOLLER_1M_*.root
    
    for J/psi
      set1               bggen_output_Jpsi_1M_*.root
      
      Generate LUND files using hall d generator output rootfiles
      Script will generate pions and neutrons and protons background LUND files.
      
      Max no.of pions per LUND file allowed for now is 1 million
      
      To run,
      ex.
      ./HallD_gen_lund /home/rakithab/Simulation_Analysis/Generators/HallD/ElectroProduction/bggen_output_1M_\*.root 1 0 2
      
      where arg 1 is hall d rootfile, default is /home/rakithab/Simulation_Analysis/Generators/HallD/ElectroProduction/bggen_output_1M_\*.root
      arg 2 is target choose from {"LH","LD","3He","JPsi","SIDIS_LH","MOLLER"} using array index, default is 1
      arg 3 is no.of lund file entires choose from {1e3,1e4,1e5,1e6} using array index, default is 0
      arg 4 is file index, where multiple sets of lund files generated file index can be changed. default is 1
      
      Set of generated lund files are available at,
      ifarm:/work/halla/solid/rakithab/halld/lund_format
      
      that can be used to simulate pion background for SoLID and MOLLER targets.
      
      for more information contact rakitha beminiwattha, rakithab@jlab.org
	*/


#include <vector>
#include <string>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <new>
#include <cstdlib>
#include <math.h>
#include <stdio.h>

#include <TRandom.h>
#include <TRandom3.h>
#include <TApplication.h>
#include <TSystem.h>

#include <TH2F.h>
#include <TTree.h>
#include <TF1.h>
#include <TF2.h>
#include <TF3.h>
#include <TProfile.h>
#include <Rtypes.h>
#include <TROOT.h>
#include <TFile.h>
#include <TChain.h>
#include <TString.h> 
#include <TDatime.h>
#include <TStopwatch.h>
#include <stdexcept>
#include <time.h>
#include <cstdio>
#include <map>
#include <set>
#include <cassert>

#include <TMath.h>
#include <TStyle.h>
#include <TPaveStats.h>

#include <TCanvas.h>
#include <TLine.h>
#include <TGraph.h>
#include <TMultiGraph.h>
#include <TLegend.h>
#include <TGraphErrors.h>
#include <TFrame.h>
#include <TObjArray.h>
#include <TVector2.h>
#include <TVirtualFitter.h>


using namespace std;

#define __IO_MAXHIT 10000
#define __TARGETS 7
#define __MOMENTUM_BINS 6
#define __MOMENTUM_BINS1 10

Double_t fEvQ2;
Double_t fEvW2;
Double_t fEvXbj;
Double_t Wprate,Wmrate,y;
Double_t Ef;

//for ntuple from hall d code
// Declaration of leaf types
Int_t           ieve;
Int_t           irun;
Char_t          iend;
Int_t           iproc;
Int_t           itypin[2][2];
Float_t         amin[2];
Float_t         pin[2][3];
Int_t           np;
Int_t           ityp[600][6];   //[np]
Float_t         am[600];   //[np]
Float_t         pout[600][3];   //[np]
Float_t         vz;
// List of branches
TBranch        *b_ieve;   //!
TBranch        *b_irun;   //!
TBranch        *b_iend;   //!
TBranch        *b_iproc;   //!
TBranch        *b_itypin;   //!
TBranch        *b_amin;   //!
TBranch        *b_pin;   //!
TBranch        *b_np;   //!
TBranch        *b_ityp;   //!
TBranch        *b_am;   //!
TBranch        *b_pout;   //!
TBranch        *b_vz;

Bool_t kSaveRootFile =kFALSE;//kFALSE;//kTRUE;
Bool_t kSaveCanvas = kFALSE;
TFile * rootfile;

//parameters 
TString stgt[__TARGETS]={"H1","D","He","H2","H3","TWU","TW"};//PVDIS H and D, SIDIS He, J/psi, SIDIS Proton, MOLLER proton, tgt window for sidis
TString stgt_lundfile_stem[__TARGETS]={"LH","LD","3He","JPsi","SIDIS_LH","UPT_Wind","DNT_Wind"};//PVDIS H and D, SIDIS He, J/psi, SIDIS Proton, MOLLER proton, tgt window for sidis
//Double_t targth[__TARGETS]={0.1,0.1,0.1,15.0,2.826,0.012,0.012};//in cm
Double_t targth[__TARGETS]={40,40,40,15.0,2.826,0.012,0.012};//in cm
Double_t targ_A[__TARGETS]={1.0,2.0,3.0,1.0,1.0,72.0,72.0};
Double_t targ_Z[__TARGETS]={1.0,1.0,2.0,1.0,1.0,35.0,35.0};
Double_t ecurr[__TARGETS]={50.0,50.0,15.0,3.0,0.1,15.0,15.0}; 
Double_t targ_density[__TARGETS]={0.071,0.169,1.345e-3,0.071,0.819,2.76,2.76};
Double_t xlum[__TARGETS]; 
Double_t rate_LD_correction[__TARGETS];//={1.0,(0.169/2) / 0.071,(1.345e-3/3) / 0.071,1.0,0.0,2.76/35}; 
Double_t hd_total_rate[__TARGETS]={26475096.00,68154568.00,87485.445,436791.03,2259.55,54652.18,59032.97};//40cm target total rates (need to be changed for adjusting the input paramenters in the fort.15 file) 
//down window rate: 59032.97 up window rate 54652.18 //40cm target no window 87485.45 for upstream  59032.98 for downstream without A in kHz for half target length for bremss rad length rate from 1M events 26261326.00 for J/Psi and MOLLER total rates are different since target lengths are different. The total interaction rate is computed in the hall D code given the target lenght and density
//Double_t hd_total_rate[__TARGETS]={38056.00,90616.02,215.88,436791.03,2259.55,167415952.00,59032.98};//1mm
Int_t hd_tot_events[__TARGETS]={1e6,1e6,1e6,1e6,1e6,1e6,1e6}; //not in use, event size is accessed from the no. of entries in the ntuple
Double_t weight_rate[9];//no. of pid is 5
Double_t weight_xs[9];//no. of pid is 5
Double_t targ_offset[__TARGETS]={0.1,0.1, -3.50,-3.15,3.50,-3.70,-3.30};//in m for upstream -3.70 m and downstream -3.30 m
Double_t weight;
Int_t targ_index=0;
//pid status 
Int_t iPid[8]={0};
void compute4Target();//use this routine to calculate all the target len,lumi,rate and weight parameters for different targets

//text files for lund files
ofstream myfile[9];//for pi(+,-,0) and p and n, last index saves multi-track event lund file
Bool_t blundstatus[9]={kFALSE};
Bool_t bWriteLund = kTRUE;
Int_t nLundEntries=1e3;//no.of entires in each 
Int_t nLundEntries_array[4]={1e3,1e4,1e5,1e6};//no.of entires in each
TString sLundEntries_array[4]={"1k","10k","100k","1M"};
Int_t nLundEntries_index=0;
Int_t nEvent[9]={0};//for pi(+,-,0) and p and n, last index saves multi-track event lund file and k(+,-,0)
Double_t totalRate[9]={0.0};
TRandom *r3[3];//for vertex

Double_t targ_length = 0.4;//m
Double_t rasterx_size = 0.004;//m
Double_t rastery_size = 0.004;//m
Double_t fEvV[3]={0.0};
std::map<int,double> pidmass;
std::map<int,int> pidcharge;
std::map<int,int> pid;
std::set<int> pid_set;
int track_pid;//for multi-track events
std::vector<int> track_index;

void set_plot_style();

Int_t fFile_index=1;

int main(Int_t argc,Char_t* argv[]) { 


	gROOT->SetStyle("Plain");
	gStyle->SetOptFit();
	set_plot_style();
	TString halld_rootfilename;
	//="/home/rakithab/Simulation_Analysis/Generators/HallD/ElectroProduction/bggen_output_1M_*.root";
	if (argc>4){
		halld_rootfilename=argv[1];
		targ_index = atoi(argv[2]);
		nLundEntries_index = atoi(argv[3]);
		fFile_index = atoi(argv[4]);
	}else if (argc>3){
		halld_rootfilename=argv[1];
		targ_index = atoi(argv[2]);
		nLundEntries_index = atoi(argv[3]);
		fFile_index = 1;
	}else if (argc>2){
		halld_rootfilename=argv[1];
		targ_index = atoi(argv[2]);
		nLundEntries_index = 0;//default
		fFile_index = 1;//default
	}else if (argc>1){
		halld_rootfilename=argv[1];
		targ_index = 1;
		nLundEntries_index = 0;//default
		fFile_index = 1;
	}else {
		printf("Running with default values \n");
		fFile_index = 1;
		targ_index = 1;
		nLundEntries_index = 0;//default
	}


	printf("root file name/s %s \n",halld_rootfilename.Data());
	printf("file index %i, target %s \n",fFile_index,stgt_lundfile_stem[targ_index].Data());
	printf("Lund file entries %s \n",sLundEntries_array[nLundEntries_index].Data());

	//set lund no.of entries
	nLundEntries=nLundEntries_array[nLundEntries_index];
	cout<<"nLundEntries="<<nLundEntries<<endl;

	pidmass[111]=134.9766/1000;//GeV pi0
	pidmass[211]=139.57018/1000;//GeV pi-
	pidmass[2212]=938.272/1000;//GeV proton
	pidmass[2112]=939.565/1000;//GeV neutron
	pidmass[11]=0.511/1000;//GeV electron
	pidmass[22]=0.0;//GeV photon
	pidmass[3112]=1197.449/1000;//GeV sigma-
	pidmass[3122]=1115.683/1000;//GeV lamda
	pidmass[3212]=1192.642/1000;//GeV sigma0 
	pidmass[3222]=1189.37/1000;//GeV sgima+
	pidmass[130]=497.648/1000;//GeV kaon0L
	pidmass[221]=547.862/1000;//GeV eta 
	pidmass[310]=497.648/1000;//GeV kaon0S
	pidmass[321]=493.667/1000;//GeV kaon+

	pidmass[-3112]=1197.449/1000;//GeV
	pidmass[-3122]=1115.683/1000;//GeV
	pidmass[-3212]=1192.642/1000;//GeV
	pidmass[-3222]=1189.37/1000;//GeV
	pidmass[-3312]=1321.71/1000;//GeV
	pidmass[-3322]=1314.86/1000;//GeV
	pidmass[-2212]=938.272/1000;//GeV
	pidmass[-2112]=939.565/1000;//GeV

	pidmass[-321]=493.667/1000;//GeV
	pidmass[-211]=139.57018/1000;//GeV
	pidmass[-11]=0.511/1000;//GeV


	pidcharge[111]=0;  
	pidcharge[211]=1;
	pidcharge[2112]=0;  
	pidcharge[2212]=1;
	pidcharge[11]=-1;//electron
	pidcharge[22]=0;// photon
	pidcharge[3112]=-1;// sigma-
	pidcharge[3122]=0;// lamda
	pidcharge[3212]=0;// sigma0 
	pidcharge[3222]=1;// sgima+
	pidcharge[130]=0;// kaon0L
	pidcharge[221]=0;// eta 
	pidcharge[310]=0;// kaon0S
	pidcharge[321]=1;// kaon+

	pidcharge[-3112]=1;//anti sigma-
	pidcharge[-3122]=0;//anti lamda
	pidcharge[-3212]=0;//anti sigma0
	pidcharge[-3222]=-1;//anti sgima+
	pidcharge[-3312]=1;//anti xi-
	pidcharge[-3322]=0;//anti xi0 
	pidcharge[-2212]=-1;//anti proton
	pidcharge[-2112]=0;//anti neutron

	pidcharge[-321]=-1;//kaon-
	pidcharge[-211]=-1;
	pidcharge[-11]=1;//positron



	pid[111]=1;  
	pid[-211]=2;
	pid[211]=0;
	pid[2212]=3;
	pid[2112]=4;  
	pid[1]=5;  //for multi track files
	pid[321]=6;  
	pid[-321]=7;
	pid[310]=8;

	compute4Target();
	//exit(1);
	//targ_index=1;
	printf("Generating events for %s \n",stgt_lundfile_stem[targ_index].Data());

	//momentum bins
	Double_t mom_bin_min[__MOMENTUM_BINS] = {0.,1.,2.,3.,4.,5.};
	Double_t mom_bin_max[__MOMENTUM_BINS] = {1.,2.,3.,4.,5.,10.};
	Double_t theta_bin_min[__MOMENTUM_BINS] = {0.,0.,0.,0.,0.,0.};
	Double_t theta_bin_max[__MOMENTUM_BINS] = {180.,60.,40.,20.,20.,20.};
	Int_t theta_bins[__MOMENTUM_BINS] = {90.,30.,20.,10.,10.,10.};
	Double_t mom_bin_min1[__MOMENTUM_BINS1] = {0.0,0.1,0.2,0.3,0.4,0.5,0.6,0.7,0.8,0.9};
	Double_t mom_bin_max1[__MOMENTUM_BINS1] = {0.1,0.2,0.3,0.4,0.5,0.6,0.7,0.8,0.9,1.0};

	Int_t thetabins = 90;
	Double_t maxtheta = 90;
	Int_t mombins = 10;



	TChain * halld_main = new TChain("h9");
	halld_main->Add(halld_rootfilename);//with 1M event with half target length used to compute bramss contribution  

	halld_main->SetBranchAddress("ieve", &ieve, &b_ieve); //event no
	halld_main->SetBranchAddress("irun", &irun, &b_irun);
	halld_main->SetBranchAddress("iend", &iend, &b_iend);
	halld_main->SetBranchAddress("iproc", &iproc, &b_iproc);//process
	halld_main->SetBranchAddress("itypin", itypin, &b_itypin);
	halld_main->SetBranchAddress("amin", amin, &b_amin);
	halld_main->SetBranchAddress("pin", pin, &b_pin);//photon energy pin[0][2]
	halld_main->SetBranchAddress("np", &np, &b_np);//no.of particle per event
	halld_main->SetBranchAddress("ityp", ityp, &b_ityp);//ityp[][1]==1  is real output partilce and ityp[][2] is the pid of the particle (111,-211,211 and etc)
	halld_main->SetBranchAddress("am", am, &b_am);
	halld_main->SetBranchAddress("pout", pout, &b_pout);//output momentum pout[][0], pout[][1], pout[][2]
	halld_main->SetBranchAddress("vz", &vz, &b_vz);

	Int_t hd_nentries = (Int_t)halld_main->GetEntries();
	hd_tot_events[0]=hd_nentries;
	hd_tot_events[1]=hd_nentries;
	hd_tot_events[2]=hd_nentries;
	hd_tot_events[3]=hd_nentries;
	hd_tot_events[4]=hd_nentries;
	hd_tot_events[5]=hd_nentries;

	printf("No.of enetries in the ntuple %i \n",hd_nentries);
	Double_t mom;
	Double_t th;
	Int_t mom_bin=0;
	Int_t mom_bin1=0;
	Double_t totalPions,totalBaryons;

	halld_main->Draw("TMath::Sqrt(TMath::Power(pout[][0],2)+TMath::Power(pout[][1],2)+TMath::Power(pout[][2],2))>>pionpmom(1000,0,10)",Form("(ityp[][1]==1 && ityp[][2]==211 && TMath::Sqrt(TMath::Power(pout[][0],2)+TMath::Power(pout[][1],2)+TMath::Power(pout[][2],2))>0 && TMath::ACos(pout[][2]/TMath::Sqrt(TMath::Power(pout[][0],2)+TMath::Power(pout[][1],2)+TMath::Power(pout[][2],2) ))*180/TMath::Pi() < %f )*%f",maxtheta,hd_total_rate[targ_index]/hd_tot_events[targ_index]),"goff");
	TH1F * pionpmom = (TH1F *)gDirectory->Get("pionpmom");
	totalRate[0] = pionpmom->Integral()*1e3;//rate in Hz Pi+
	weight_rate[0] = totalRate[0];//totalRate[0]/pionpmom->GetEntries();
	totalPions=pionpmom->GetEntries();
	weight_xs[0] = weight_rate[0]/xlum[targ_index]*1e6;//xs in micro barn [ub]
	printf("Pi+ Total rate %3.3f MHz %3.3f mb\n",totalRate[0]/1e6,weight_xs[0]);
	halld_main->Draw("TMath::Sqrt(TMath::Power(pout[][0],2)+TMath::Power(pout[][1],2)+TMath::Power(pout[][2],2))>>pion0mom_tmp(1000,0,10)",Form("(ityp[][1]==1 && ityp[][2]==111 && TMath::Sqrt(TMath::Power(pout[][0],2)+TMath::Power(pout[][1],2)+TMath::Power(pout[][2],2))>0 && TMath::ACos(pout[][2]/TMath::Sqrt(TMath::Power(pout[][0],2)+TMath::Power(pout[][1],2)+TMath::Power(pout[][2],2) ))*180/TMath::Pi() < %f )*%f",maxtheta,hd_total_rate[targ_index]/hd_tot_events[targ_index]),"goff");
	TH1F * pion0mom_tmp = (TH1F *)gDirectory->Get("pion0mom_tmp");
	totalRate[1] = pion0mom_tmp->Integral()*1e3;//rate in Hz  Pi0
	weight_rate[1] = totalRate[1];///pion0mom_tmp->GetEntries();
	weight_xs[1] = weight_rate[1]/xlum[targ_index]*1e6;//xs in micro barn [ub]
	printf("Pi0 Total rate %3.3f MHz %3.3f mb \n",totalRate[1]/1e6,weight_xs[1]);
	halld_main->Draw("TMath::Sqrt(TMath::Power(pout[][0],2)+TMath::Power(pout[][1],2)+TMath::Power(pout[][2],2))>>pionmmom(1000,0,10)",Form("(ityp[][1]==1 && ityp[][2]==-211 && TMath::Sqrt(TMath::Power(pout[][0],2)+TMath::Power(pout[][1],2)+TMath::Power(pout[][2],2))>0 && TMath::ACos(pout[][2]/TMath::Sqrt(TMath::Power(pout[][0],2)+TMath::Power(pout[][1],2)+TMath::Power(pout[][2],2) ))*180/TMath::Pi() < %f )*%f",maxtheta,hd_total_rate[targ_index]/hd_tot_events[targ_index]),"goff");
	TH1F * pionmmom = (TH1F *)gDirectory->Get("pionmmom");
	totalRate[2] = pionmmom->Integral()*1e3;//rate in Hz Pi-
	weight_rate[2] = totalRate[2];//totalRate[2]/pionmmom->GetEntries();
	totalPions+=pionpmom->GetEntries();
	weight_xs[2] = weight_rate[2]/xlum[targ_index]*1e6;//xs in micro barn [ub]
	printf("Pi- Total rate %3.3f MHz %3.3f mb \n",totalRate[2]/1e6,weight_xs[2]);
	halld_main->Draw("TMath::Sqrt(TMath::Power(pout[][0],2)+TMath::Power(pout[][1],2)+TMath::Power(pout[][2],2))>>protonmom(1000,0,10)",Form("(ityp[][1]==1 && ityp[][2]==2212 && TMath::Sqrt(TMath::Power(pout[][0],2)+TMath::Power(pout[][1],2)+TMath::Power(pout[][2],2))>0 && TMath::ACos(pout[][2]/TMath::Sqrt(TMath::Power(pout[][0],2)+TMath::Power(pout[][1],2)+TMath::Power(pout[][2],2) ))*180/TMath::Pi() < %f )*%f",maxtheta,hd_total_rate[targ_index]/hd_tot_events[targ_index]),"goff");
	TH1F * protonmom = (TH1F *)gDirectory->Get("protonmom");
	totalRate[3] = protonmom->Integral()*1e3;//rate in Hz Proton
	weight_rate[3] = totalRate[3];//totalRate[3]/protonmom->GetEntries();
	totalBaryons=protonmom->GetEntries();
	weight_xs[3] = weight_rate[3]/xlum[targ_index]*1e6;//xs in micro barn [ub]
	printf("Proton Total rate %3.3f MHz %3.3f mb \n",totalRate[3]/1e6,weight_xs[3]);
	halld_main->Draw("TMath::Sqrt(TMath::Power(pout[][0],2)+TMath::Power(pout[][1],2)+TMath::Power(pout[][2],2))>>neutronmom(1000,0,10)",Form("(ityp[][1]==1 && ityp[][2]==2112 && TMath::Sqrt(TMath::Power(pout[][0],2)+TMath::Power(pout[][1],2)+TMath::Power(pout[][2],2))>0 && TMath::ACos(pout[][2]/TMath::Sqrt(TMath::Power(pout[][0],2)+TMath::Power(pout[][1],2)+TMath::Power(pout[][2],2) ))*180/TMath::Pi() < %f )*%f",maxtheta,hd_total_rate[targ_index]/hd_tot_events[targ_index]),"goff");
	TH1F * neutronmom = (TH1F *)gDirectory->Get("neutronmom");
	totalRate[4] = neutronmom->Integral()*1e3;//rate in Hz Pi-
	weight_rate[4] = totalRate[4];//totalRate[4]/neutronmom->GetEntries();
	totalBaryons+=neutronmom->GetEntries();
	weight_xs[4] = weight_rate[4]/xlum[targ_index]*1e6;//xs in micro barn [ub]
	printf("Neutron Total rate %3.3f MHz %3.3f mb \n",totalRate[4]/1e6,weight_xs[4]);




	halld_main->Draw("TMath::Sqrt(TMath::Power(pout[][0],2)+TMath::Power(pout[][1],2)+TMath::Power(pout[][2],2))>>kaonpmom(1000,0,10)",Form("(ityp[][1]==1 && ityp[][2]==321 && TMath::Sqrt(TMath::Power(pout[][0],2)+TMath::Power(pout[][1],2)+TMath::Power(pout[][2],2))>0 && TMath::ACos(pout[][2]/TMath::Sqrt(TMath::Power(pout[][0],2)+TMath::Power(pout[][1],2)+TMath::Power(pout[][2],2) ))*180/TMath::Pi() < %f )*%f",maxtheta,hd_total_rate[targ_index]/hd_tot_events[targ_index]),"goff");
	TH1F * kaonpmom = (TH1F *)gDirectory->Get("kaonpmom");
	totalRate[6] = kaonpmom->Integral()*1e3;//rate in Hz Pi+
	weight_rate[6] = totalRate[6];//totalRate[0]/pionpmom->GetEntries();
	//totalPions=kaonpmom->GetEntries();
	weight_xs[6] = weight_rate[6]/xlum[targ_index]*1e6;//xs in micro barn [ub]
	printf("K+ Total rate %3.3f MHz %3.3f mb\n",totalRate[6]/1e6,weight_xs[6]);
	halld_main->Draw("TMath::Sqrt(TMath::Power(pout[][0],2)+TMath::Power(pout[][1],2)+TMath::Power(pout[][2],2))>>kaon0mom_tmp(1000,0,10)",Form("(ityp[][1]==1 && ityp[][2]==310 || ityp[][2]==130 && TMath::Sqrt(TMath::Power(pout[][0],2)+TMath::Power(pout[][1],2)+TMath::Power(pout[][2],2))>0 && TMath::ACos(pout[][2]/TMath::Sqrt(TMath::Power(pout[][0],2)+TMath::Power(pout[][1],2)+TMath::Power(pout[][2],2) ))*180/TMath::Pi() < %f )*%f",maxtheta,hd_total_rate[targ_index]/hd_tot_events[targ_index]),"goff");
	TH1F * kaon0mom_tmp = (TH1F *)gDirectory->Get("kaon0mom_tmp");
	totalRate[8] = kaon0mom_tmp->Integral()*1e3;//rate in Hz  Pi0
	weight_rate[8] = totalRate[8];///pion0mom_tmp->GetEntries();
	weight_xs[8] = weight_rate[8]/xlum[targ_index]*1e6;//xs in micro barn [ub]
	printf("K0 Total rate %3.3f MHz %3.3f mb \n",totalRate[8]/1e6,weight_xs[8]);
	halld_main->Draw("TMath::Sqrt(TMath::Power(pout[][0],2)+TMath::Power(pout[][1],2)+TMath::Power(pout[][2],2))>>kaonmmom(1000,0,10)",Form("(ityp[][1]==1 && ityp[][2]==-321 && TMath::Sqrt(TMath::Power(pout[][0],2)+TMath::Power(pout[][1],2)+TMath::Power(pout[][2],2))>0 && TMath::ACos(pout[][2]/TMath::Sqrt(TMath::Power(pout[][0],2)+TMath::Power(pout[][1],2)+TMath::Power(pout[][2],2) ))*180/TMath::Pi() < %f )*%f",maxtheta,hd_total_rate[targ_index]/hd_tot_events[targ_index]),"goff");
	TH1F * kaonmmom = (TH1F *)gDirectory->Get("kaonmmom");
	totalRate[7] = kaonmmom->Integral()*1e3;//rate in Hz Kaon-
	weight_rate[7] = totalRate[7];//totalRate[2]/pionmmom->GetEntries();
	//totalPions+=kaonpmom->GetEntries();
	weight_xs[7] = weight_rate[7]/xlum[targ_index]*1e6;//xs in micro barn [ub]
	printf("K- Total rate %3.3f MHz %3.3f mb \n",totalRate[7]/1e6,weight_xs[7]);


	if (targ_A[targ_index]>1){
		printf(" Weight factors from proton pip %3.3e  pi0 %3.3e pim %3.3e p %3.3e n %3.3e Kp %3.3e K0 %3.3e Km %3.3e \n",weight_xs[pid[211]],weight_xs[pid[111]],weight_xs[pid[-211]],weight_xs[pid[2212]],weight_xs[pid[2112]], weight_xs[pid[321]],weight_xs[pid[310]],weight_xs[pid[-321]]);
		printf(" Total tracks  %6.0f, %6.0f, %6.0f, %6.0f, %6.0f \n",pionpmom->GetEntries(),pion0mom_tmp->GetEntries(),pionmmom->GetEntries(),protonmom->GetEntries(),neutronmom->GetEntries());
		printf("Weight factors for A>1 :  pip %3.3e  pi0 %3.3e pim %3.3e p %3.3e n %3.3e Kp %3.3e K0 %3.3e Km %3.3e \n",((targ_A[targ_index] - targ_Z[targ_index])*weight_xs[pid[-211]] + targ_Z[targ_index]*weight_xs[pid[211]])/1,targ_A[targ_index]*weight_xs[pid[111]],((targ_A[targ_index] - targ_Z[targ_index])*weight_xs[pid[211]] + targ_Z[targ_index]*weight_xs[pid[-211]])/1,((targ_A[targ_index] - targ_Z[targ_index])*weight_xs[pid[2112]] + targ_Z[targ_index]*weight_xs[pid[2212]])/1,((targ_A[targ_index] - targ_Z[targ_index])*weight_xs[pid[2212]] + targ_Z[targ_index]*weight_xs[pid[2112]])/1,((targ_A[targ_index] - targ_Z[targ_index])*weight_xs[pid[-321]] + targ_Z[targ_index]*weight_xs[pid[321]])/1,targ_A[targ_index]*weight_xs[pid[310]],((targ_A[targ_index] - targ_Z[targ_index])*weight_xs[pid[321]] + targ_Z[targ_index]*weight_xs[pid[-321]])/1);
		weight_xs[pid[211]]/=(pionpmom->GetEntries()+pionmmom->GetEntries());
		weight_xs[pid[-211]]/=(pionpmom->GetEntries()+pionmmom->GetEntries());
		weight_xs[pid[2112]]/=(protonmom->GetEntries()+neutronmom->GetEntries());
		weight_xs[pid[2212]]/=(protonmom->GetEntries()+neutronmom->GetEntries());
		weight_xs[pid[111]]/=pion0mom_tmp->GetEntries();
		weight_xs[pid[310]]/=kaon0mom_tmp->GetEntries();
		weight_xs[pid[321]]/=(kaonpmom->GetEntries()+kaonmmom->GetEntries());
		weight_xs[pid[-321]]/=(kaonpmom->GetEntries()+kaonmmom->GetEntries());

	}else{
		printf(" Weight factors from proton pip %3.3e  pi0 %3.3e pim %3.3e p %3.3e n %3.3e Kp %3.3e K0 %3.3e Km %3.3e \n",weight_xs[pid[211]],weight_xs[pid[111]],weight_xs[pid[-211]],weight_xs[pid[2212]],weight_xs[pid[2112]],weight_xs[pid[321]],weight_xs[pid[310]],weight_xs[pid[-321]]);
		printf(" Total tracks  %6.0f, %6.0f, %6.0f, %6.0f, %6.0f \n",pionpmom->GetEntries(),pion0mom_tmp->GetEntries(),pionmmom->GetEntries(),protonmom->GetEntries(),neutronmom->GetEntries());
		printf("Weight factors for A>1 :  pip %3.3e  pi0 %3.3e pim %3.3e p %3.3e n %3.3e Kp %3.3e K0 %3.3e Km %3.3e \n",((targ_A[targ_index] - targ_Z[targ_index])*weight_xs[pid[-211]] + targ_Z[targ_index]*weight_xs[pid[211]])/1,targ_A[targ_index]*weight_xs[pid[111]],((targ_A[targ_index] - targ_Z[targ_index])*weight_xs[pid[211]] + targ_Z[targ_index]*weight_xs[pid[-211]])/1,((targ_A[targ_index] - targ_Z[targ_index])*weight_xs[pid[2112]] + targ_Z[targ_index]*weight_xs[pid[2212]])/1,((targ_A[targ_index] - targ_Z[targ_index])*weight_xs[pid[2212]] + targ_Z[targ_index]*weight_xs[pid[2112]])/1,((targ_A[targ_index] - targ_Z[targ_index])*weight_xs[pid[-321]] + targ_Z[targ_index]*weight_xs[pid[321]])/1,targ_A[targ_index]*weight_xs[pid[310]],((targ_A[targ_index] - targ_Z[targ_index])*weight_xs[pid[321]] + targ_Z[targ_index]*weight_xs[pid[-321]])/1);    
		weight_xs[pid[111]]/=pion0mom_tmp->GetEntries();
		weight_xs[pid[310]]/=kaon0mom_tmp->GetEntries();
		weight_xs[pid[211]]/=pionpmom->GetEntries();
		weight_xs[pid[-211]]/=pionmmom->GetEntries();
		weight_xs[pid[2112]]/=neutronmom->GetEntries();
		weight_xs[pid[2212]]/=protonmom->GetEntries();
		weight_xs[pid[321]]/=kaonpmom->GetEntries();
		weight_xs[pid[-321]]/=kaonpmom->GetEntries();
	}
	//following kinematics parameters are not in use for hadron backgrounds
	fEvQ2=0;
	fEvW2=0;
	fEvXbj=0;
	Wprate=0;
	Wmrate=0;
	y=0;

	if (bWriteLund){//for LUND format each event has the rate factor included. Therefore we need to know the total rate beforehand 
		//create LUND files

		myfile[0].open(Form("hallD_pion_p_%s_%s_%i.lund",stgt_lundfile_stem[targ_index].Data(),sLundEntries_array[nLundEntries_index].Data(),fFile_index),ios::out);
		myfile[1].open(Form("hallD_pion_0_%s_%s_%i.lund",stgt_lundfile_stem[targ_index].Data(),sLundEntries_array[nLundEntries_index].Data(),fFile_index),ios::out);
		myfile[2].open(Form("hallD_pion_m_%s_%s_%i.lund",stgt_lundfile_stem[targ_index].Data(),sLundEntries_array[nLundEntries_index].Data(),fFile_index),ios::out);
		myfile[3].open(Form("hallD_proton_%s_%s_%i.lund",stgt_lundfile_stem[targ_index].Data(),sLundEntries_array[nLundEntries_index].Data(),fFile_index),ios::out);
		myfile[4].open(Form("hallD_neutron_%s_%s_%i.lund",stgt_lundfile_stem[targ_index].Data(),sLundEntries_array[nLundEntries_index].Data(),fFile_index),ios::out);
		myfile[5].open(Form("hallD_%s_all_tracks_%s_%i.lund",stgt_lundfile_stem[targ_index].Data(),sLundEntries_array[nLundEntries_index].Data(),fFile_index),ios::out);
		myfile[6].open(Form("hallD_kaon_p_%s_%s_%i.lund",stgt_lundfile_stem[targ_index].Data(),sLundEntries_array[nLundEntries_index].Data(),fFile_index),ios::out);
		myfile[8].open(Form("hallD_kaon_0_%s_%s_%i.lund",stgt_lundfile_stem[targ_index].Data(),sLundEntries_array[nLundEntries_index].Data(),fFile_index),ios::out);
		myfile[7].open(Form("hallD_kaon_m_%s_%s_%i.lund",stgt_lundfile_stem[targ_index].Data(),sLundEntries_array[nLundEntries_index].Data(),fFile_index),ios::out);

		r3[0] = new TRandom3(1);// for vertex x distribution
		r3[1] = new TRandom3(2);// for vertex y distribution
		r3[2] = new TRandom3(3);// for vertex z distribution
	}

	TF1 *funwiser = new TF1("funwiser","550354+15264.4*x",-10,30);
	int pioN=0;
	cout<<"total_event="<<hd_nentries;
	for (int i=0; i<hd_nentries ; i++) {
		halld_main->GetEntry(i);
		track_index.clear();//clear the track index vector begining of each event
		for (int j = 0; j<np; j++){
			if (ityp[j][1]==1){
				track_index.push_back(j);
				mom = TMath::Sqrt(TMath::Power(pout[j][0],2)+TMath::Power(pout[j][1],2)+TMath::Power(pout[j][2],2));
				th = TMath::ACos(pout[j][2]/TMath::Sqrt(TMath::Power(pout[j][0],2)+TMath::Power(pout[j][1],2)+TMath::Power(pout[j][2],2) ))*180/TMath::Pi();
				if (th < maxtheta){
					/*	  for (Int_t k=0;k<__MOMENTUM_BINS;k++){	    
					//cout<<"DEBUG 1 "<<mom_bin<<" "<<mom<<" "<<mom_bin_min[k]<<" "<<mom_bin_max[k] <<endl;
					if (mom_bin_min[k]< mom && mom_bin_max[k]>= mom){
					mom_bin = k;
					if (k==0){
					for (Int_t k1=0;k1<__MOMENTUM_BINS1;k1++){	
					if (mom_bin_min1[k1] < mom  && mom_bin_max1[k1] >= mom){
					mom_bin1 = k1;
					break;
					}
					} 
					}	      
					break;
					}else
					mom_bin = -1;
					}
					//cout<<"DEBUG 2"<<mom_bin<<" "<<mom<<endl;
					if (mom_bin < 0)
					exit(1);*/
					//cout<<"DEBUG 1 "<<mom_bin<<" "<<mom<<endl;
					if (bWriteLund){
						//determine the scattering vertex position in the target
						if (rasterx_size>0)
							fEvV[0] = r3[0]->Uniform(-1,1)*rasterx_size/2;
						if (rastery_size>0)
							fEvV[1] = r3[1]->Uniform(-1,1)*rastery_size/2;
					}

					if (ityp[j][2]==111){
						//compute the total energy of the particle
						weight = targ_A[targ_index]*weight_xs[pid[111]];
						Ef=TMath::Sqrt(TMath::Power(mom,2) + TMath::Power(pidmass[111],2) );// subtract mass to get K.E - pidmass[pid];
						//save this event 
						if (bWriteLund){
							if (nEvent[pid[111]] < nLundEntries){
								myfile[pid[111]]<< "1" << " \t " << Wprate  << " \t " << Wmrate  << " \t " << "0"  << " \t " << "0" << " \t "  << fEvXbj << " \t " << y  << " \t " << fEvW2  << " \t " << fEvQ2  << " \t " << targ_A[targ_index]*totalRate[pid[111]]/nLundEntries << endl;
								myfile[pid[111]]<< " \t " << "1" << " \t " << pidcharge[111] << " \t " << "1" << " \t " << 111 << " \t " << "0" << " \t " << "0" << " \t " << pout[j][0] << " \t " << pout[j][1] << " \t " << pout[j][2] << " \t " << Ef << " \t " << pidmass[111] << " \t " << fEvV[0]*100  << " \t " << fEvV[1]*100 << " \t " << vz+targ_offset[targ_index]*100 << endl;
								pioN +=1;           
							} else if (!blundstatus[1])
								blundstatus[1]=kTRUE;
							//increment hadron event count
							nEvent[pid[111]]++;
						}

					}
					if (ityp[j][2]==-211 || ityp[j][2]==211){
						if (ityp[j][2]==-211){
							iPid[pid[-211]]=1;
							iPid[pid[211]]=0;
						}
						if (ityp[j][2]==211){
							iPid[pid[-211]]=0;
							iPid[pid[211]]=1;
						}

						//fill histograms
						if (iPid[pid[-211]]>0){//pi-
							weight = ((targ_A[targ_index] - targ_Z[targ_index])*weight_xs[pid[211]] + targ_Z[targ_index]*weight_xs[pid[-211]])/iPid[pid[-211]];
							Ef=TMath::Sqrt(TMath::Power(mom,2) + TMath::Power(pidmass[-211],2) );// subtract mass to get K.E - pidmass[pid];
						}
						if (iPid[pid[211]]>0){//pi+
							weight = ((targ_A[targ_index] - targ_Z[targ_index])*weight_xs[pid[-211]] + targ_Z[targ_index]*weight_xs[pid[211]])/iPid[pid[211]];
							Ef=TMath::Sqrt(TMath::Power(mom,2) + TMath::Power(pidmass[211],2) );// subtract mass to get K.E - pidmass[pid];
						}
						//save this event 
						if (bWriteLund){
							if (nEvent[2] < nLundEntries && iPid[pid[-211]]>0){//pi-
								myfile[2]<< "1" << " \t " << Wprate  << " \t " << Wmrate  << " \t " << "0"  << " \t " << "0" << " \t "  << fEvXbj << " \t " << y  << " \t " << fEvW2  << " \t " << fEvQ2  << " \t " << ((targ_A[targ_index] - targ_Z[targ_index])*totalRate[pid[211]] + targ_Z[targ_index]*totalRate[pid[-211]])/iPid[pid[-211]]/nLundEntries << endl;
								myfile[2]<< " \t " << "1" << " \t " << pidcharge[-211] << " \t " << "1" << " \t " << -211 << " \t " << "0" << " \t " << "0" << " \t " << pout[j][0] << " \t " << pout[j][1] << " \t " << pout[j][2] << " \t " << Ef << " \t " << pidmass[-211] << " \t " << fEvV[0]*100  << " \t " << fEvV[1]*100 << " \t " << vz+targ_offset[targ_index]*100 << endl;
							} else if (!blundstatus[2])
								blundstatus[2]=kTRUE;
							//increment hadron event count
							if (iPid[pid[-211]]>0)
								nEvent[2]++;
							if (nEvent[0] < nLundEntries && iPid[pid[211]]>0){//pi+
								myfile[0]<< "1" << " \t " << Wprate  << " \t " << Wmrate  << " \t " << "0"  << " \t " << "0" << " \t "  << fEvXbj << " \t " << y  << " \t " << fEvW2  << " \t " << fEvQ2  << " \t " << (targ_Z[targ_index]*totalRate[pid[211]] + (targ_A[targ_index] - targ_Z[targ_index])*totalRate[pid[-211]])/iPid[pid[211]]/nLundEntries << endl;
								myfile[0]<< " \t " << "1" << " \t " << pidcharge[211] << " \t " << "1" << " \t " << 211 << " \t " << "0" << " \t " << "0" << " \t " << pout[j][0] << " \t " << pout[j][1] << " \t " << pout[j][2] << " \t " << Ef << " \t " << pidmass[211] << " \t " << fEvV[0]*100  << " \t " << fEvV[1]*100 << " \t " << vz+targ_offset[targ_index]*100 << endl;
							} else if (!blundstatus[0])
								blundstatus[0]=kTRUE;
							//increment hadron event count
							if (iPid[pid[211]]>0)
								nEvent[0]++;

						}	  

					}

					if (ityp[j][2]==2212 || ityp[j][2]==2112){
						if (ityp[j][2]==2212){
							iPid[pid[2212]]=1;
							iPid[pid[2112]]=0;
						}
						if (ityp[j][2]==2112){
							iPid[pid[2212]]=0;
							iPid[pid[2112]]=1;
						}
						if(targ_A[targ_index]>1){
							iPid[pid[2212]]=1;
							iPid[pid[2112]]=1;
						}
						if (iPid[pid[2212]]>0){//proton
							weight = ((targ_A[targ_index] - targ_Z[targ_index])*weight_xs[pid[2112]] + targ_Z[targ_index]*weight_xs[pid[2212]])/iPid[pid[2212]];
							Ef=TMath::Sqrt(TMath::Power(mom,2) + TMath::Power(pidmass[2212],2) );// subtract mass to get K.E - pidmass[pid];
						}
						if (iPid[pid[2112]]>0){//neutron
							weight = ((targ_A[targ_index] - targ_Z[targ_index])*weight_xs[pid[2212]] + targ_Z[targ_index]*weight_xs[pid[2112]])/iPid[pid[2112]];
							Ef=TMath::Sqrt(TMath::Power(mom,2) + TMath::Power(pidmass[2112],2) );// subtract mass to get K.E - pidmass[pid];
						}
						//save this event 
						if (bWriteLund){
							if (nEvent[3] < nLundEntries && iPid[pid[2212]]>0){//proton
								myfile[3]<< "1" << " \t " << Wprate  << " \t " << Wmrate  << " \t " << "0"  << " \t " << "0" << " \t "  << fEvXbj << " \t " << y  << " \t " << fEvW2  << " \t " << fEvQ2  << " \t " << ( (targ_A[targ_index] - targ_Z[targ_index])*totalRate[pid[2112]] + targ_Z[targ_index]*totalRate[pid[2212]])/iPid[pid[2212]]/nLundEntries << endl;
								myfile[3]<< " \t " << "1" << " \t " << pidcharge[2212] << " \t " << "1" << " \t " << 2212 << " \t " << "0" << " \t " << "0" << " \t " << pout[j][0] << " \t " << pout[j][1] << " \t " << pout[j][2] << " \t " << Ef << " \t " << pidmass[2212] << " \t " << fEvV[0]*100  << " \t " << fEvV[1]*100 << " \t " << vz+targ_offset[targ_index]*100 << endl;
							} else if (!blundstatus[3])
								blundstatus[3]=kTRUE;
							//increment hadron event count
							if (iPid[pid[2212]]>0)
								nEvent[pid[2212]]++;
							if (nEvent[4] < nLundEntries && iPid[pid[2112]]>0){//neutron
								myfile[4]<< "1" << " \t " << Wprate  << " \t " << Wmrate  << " \t " << "0"  << " \t " << "0" << " \t "  << fEvXbj << " \t " << y  << " \t " << fEvW2  << " \t " << fEvQ2  << " \t " << ( (targ_A[targ_index] - targ_Z[targ_index])*totalRate[pid[2212]] + targ_Z[targ_index]*totalRate[pid[2112]])/iPid[pid[2112]]/nLundEntries << endl;
								myfile[4]<< " \t " << "1" << " \t " << pidcharge[2112] << " \t " << "1" << " \t " << 2112 << " \t " << "0" << " \t " << "0" << " \t " << pout[j][0] << " \t " << pout[j][1] << " \t " << pout[j][2] << " \t " << Ef << " \t " << pidmass[2112] << " \t " << fEvV[0]*100  << " \t " << fEvV[1]*100 << " \t " << vz+targ_offset[targ_index]*100 << endl;
							} else if (!blundstatus[4])
								blundstatus[4]=kTRUE;
							//increment hadron event count
							if (iPid[pid[2112]]>0)
								nEvent[pid[2112]]++;	      
						}
					}

					// kaon outputs

					if (ityp[j][2]==310 || ityp[j][2]==130 ){
						//compute the total energy of the particle
						if(ityp[j][2]==310){
							weight = targ_A[targ_index]*weight_xs[pid[310]];
							Ef=TMath::Sqrt(TMath::Power(mom,2) + TMath::Power(pidmass[310],2) );// subtract mass to get K.E - pidmass[pid];
						}else{
							weight = targ_A[targ_index]*weight_xs[pid[130]];
							Ef=TMath::Sqrt(TMath::Power(mom,2) + TMath::Power(pidmass[130],2) );// subtract mass to get K.E - pidmass[pid];
						}
						//save this event 
						if (bWriteLund){
							// cout<<"Eventpi0="<<nEvent[pid[111]]<<endl;
							if (nEvent[pid[310]]< nLundEntries){
								myfile[pid[310]]<< "1" << " \t " << Wprate  << " \t " << Wmrate  << " \t " << "0"  << " \t " << "0" << " \t "  << fEvXbj << " \t " << y  << " \t " << fEvW2  << " \t " << fEvQ2  << " \t " << targ_A[targ_index]*totalRate[pid[310]]/nLundEntries << endl;
								if(ityp[j][2]==310){
									myfile[pid[310]]<< " \t " << "1" << " \t " << pidcharge[310] << " \t " << "1" << " \t " << 310 << " \t " << "0" << " \t " << "0" << " \t " << pout[j][0] << " \t " << pout[j][1] << " \t " << pout[j][2] << " \t " << Ef << " \t " << pidmass[310] << " \t " << fEvV[0]*100  << " \t " << fEvV[1]*100 << " \t " << vz+targ_offset[targ_index]*100 << endl;
								}
								if(ityp[j][2]==130){
									myfile[pid[310]]<< " \t " << "1" << " \t " << pidcharge[130] << " \t " << "1" << " \t " << 130 << " \t " << "0" << " \t " << "0" << " \t " << pout[j][0] << " \t " << pout[j][1] << " \t " << pout[j][2] << " \t " << Ef << " \t " << pidmass[130] << " \t " << fEvV[0]*100  << " \t " << fEvV[1]*100 << " \t " << vz+targ_offset[targ_index]*100 << endl;
								}

								//pioN +=1;           
							} else if (!blundstatus[8])
								blundstatus[8]=kTRUE;
							//increment hadron event count
							nEvent[pid[310]]++;
						}

					}
					if (ityp[j][2]==-321 || ityp[j][2]==321){
						if (ityp[j][2]==-321){
							iPid[pid[-321]]=1;
							iPid[pid[321]]=0;
						}
						if (ityp[j][2]==321){
							iPid[pid[-321]]=0;
							iPid[pid[321]]=1;
						}
						if(targ_A[targ_index]>1){
							iPid[pid[-321]]=1;
							iPid[pid[321]]=1;
						}

						//fill histograms
						if (iPid[pid[-321]]>0){//pi-
							weight = ((targ_A[targ_index] - targ_Z[targ_index])*weight_xs[pid[321]] + targ_Z[targ_index]*weight_xs[pid[-321]])/iPid[pid[-321]];
							Ef=TMath::Sqrt(TMath::Power(mom,2) + TMath::Power(pidmass[-321],2) );// subtract mass to get K.E - pidmass[pid];
						}
						if (iPid[pid[321]]>0){//pi+
							weight = ((targ_A[targ_index] - targ_Z[targ_index])*weight_xs[pid[-321]] + targ_Z[targ_index]*weight_xs[pid[321]])/iPid[pid[321]];
							Ef=TMath::Sqrt(TMath::Power(mom,2) + TMath::Power(pidmass[321],2) );// subtract mass to get K.E - pidmass[pid];
						}
						//save this event 
						if (bWriteLund){
							if (nEvent[2] < nLundEntries && iPid[pid[-321]]>0){//pi-
								myfile[7]<< "1" << " \t " << Wprate  << " \t " << Wmrate  << " \t " << "0"  << " \t " << "0" << " \t "  << fEvXbj << " \t " << y  << " \t " << fEvW2  << " \t " << fEvQ2  << " \t " << ((targ_A[targ_index] - targ_Z[targ_index])*totalRate[pid[321]] + targ_Z[targ_index]*totalRate[pid[-321]])/iPid[pid[-321]]/nLundEntries << endl;
								myfile[7]<< " \t " << "1" << " \t " << pidcharge[-321] << " \t " << "1" << " \t " << -321 << " \t " << "0" << " \t " << "0" << " \t " << pout[j][0] << " \t " << pout[j][1] << " \t " << pout[j][2] << " \t " << Ef << " \t " << pidmass[-321] << " \t " << fEvV[0]*100  << " \t " << fEvV[1]*100 << " \t " << vz+targ_offset[targ_index]*100 << endl;
							} else if (!blundstatus[7])
								blundstatus[7]=kTRUE;
							//increment hadron event count
							if (iPid[pid[-321]]>0)
								nEvent[7]++;
							if (nEvent[6] < nLundEntries && iPid[pid[321]]>0){//pi+
								myfile[6]<< "1" << " \t " << Wprate  << " \t " << Wmrate  << " \t " << "0"  << " \t " << "0" << " \t "  << fEvXbj << " \t " << y  << " \t " << fEvW2  << " \t " << fEvQ2  << " \t " << (targ_Z[targ_index]*totalRate[pid[321]] + (targ_A[targ_index] - targ_Z[targ_index])*totalRate[pid[-321]])/iPid[pid[321]]/nLundEntries << endl;
								myfile[6]<< " \t " << "1" << " \t " << pidcharge[321] << " \t " << "1" << " \t " << 321 << " \t " << "0" << " \t " << "0" << " \t " << pout[j][0] << " \t " << pout[j][1] << " \t " << pout[j][2] << " \t " << Ef << " \t " << pidmass[321] << " \t " << fEvV[0]*100  << " \t " << fEvV[1]*100 << " \t " << vz+targ_offset[targ_index]*100 << endl;
							} else if (!blundstatus[6])
								blundstatus[6]=kTRUE;
							//increment hadron event count
							if (iPid[pid[321]]>0)
								nEvent[6]++;

						}	  

					}	     
				}
			}

		}
		//lund file with multi particle states
		if (bWriteLund){
			if (nEvent[pid[1]] < nLundEntries){//pid=1 is used for multi-track events
				myfile[pid[1]]<< track_index.size() << " \t " << Wprate  << " \t " << Wmrate  << " \t " << "0"  << " \t " << "0" << " \t "  << fEvXbj << " \t " << y  << " \t " << fEvW2  << " \t " << fEvQ2  << " \t " << targ_A[targ_index]*hd_total_rate[targ_index]*1e3/nLundEntries << endl;
				Int_t j=0;//track id
				for(Int_t k=0;k<track_index.size();k++){
					j=track_index[k];
					track_pid=ityp[j][2];
					mom = TMath::Sqrt(TMath::Power(pout[j][0],2)+TMath::Power(pout[j][1],2)+TMath::Power(pout[j][2],2));
					Ef=TMath::Sqrt(TMath::Power(mom,2) + TMath::Power(pidmass[track_pid],2) );// subtract mass to get K.E - pidmass[pid];
					myfile[pid[1]]<< " \t " << k+1 << " \t " << pidcharge[track_pid] << " \t " << "1" << " \t " << track_pid << " \t " << "0" << " \t " << "0" << " \t " << pout[j][0] << " \t " << pout[j][1] << " \t " << pout[j][2] << " \t " << Ef << " \t " << pidmass[track_pid] << " \t " << fEvV[0]*100  << " \t " << fEvV[1]*100 << " \t " << vz+targ_offset[targ_index]*100 << endl;
				}
			} else if (!blundstatus[5])
				blundstatus[5]=kTRUE;
			nEvent[pid[1]]++;
		}
	} //end event loop
	Double_t pion_xs[3][__MOMENTUM_BINS+1]={{0}};
	Double_t pion0_xs[__MOMENTUM_BINS1]={0};



	if (bWriteLund){
		for (Int_t i=0;i<6;i++)
			myfile[i].close();
	}

	printf("Generated LUND Files!\n");
	//theApp.Run();

	return(1);
} //end main loop

void compute4Target(){
	for(Int_t i=0;i<__TARGETS;i++){
		//rate_LD_correction[i] = targ_density[i]/targ_A[i]/targ_density[0];
		rate_LD_correction[i]=1/targ_A[i];
		hd_total_rate[i]*=rate_LD_correction[i];
		xlum[i] = ecurr[i]/1.6E-19*targth[i]*targ_density[i]/targ_A[i]*0.6022*1e-6;
		//printf(" %f \t %f \t %f \n", xlum[i], hd_total_rate[i], hd_total_rate[i]/xlum[i]);
		//weight_rate[i] = hd_total_rate[i]/hd_tot_events[i]*targ_A[i];
		//weight_xs[i] = hd_total_rate[i]*1e3/hd_tot_events[i]/xlum[i]*1e6*targ_A[i];//weighted by xs
	}
};

void set_plot_style()
{
	const Int_t NRGBs = 5;
	const Int_t NCont = 255;

	Double_t stops[NRGBs] = { 0.00, 0.34, 0.61, 0.84, 1.00 };
	Double_t red[NRGBs]   = { 0.00, 0.00, 0.87, 1.00, 0.51 };
	Double_t green[NRGBs] = { 0.00, 0.81, 1.00, 0.20, 0.00 };
	Double_t blue[NRGBs]  = { 0.51, 1.00, 0.12, 0.00, 0.00 };
	TColor::CreateGradientColorTable(NRGBs, stops, red, green, blue, NCont);
	gStyle->SetNumberContours(NCont);
}


