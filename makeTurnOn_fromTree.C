#include <TFile.h>
#include <TTree.h>
#include <TChain.h>
#include <TCanvas.h>
#include <TLegend.h>
#include <TH1D.h>
#include <TH2D.h>
#include <TLatex.h>
#include <TGraph.h>
#include <TString.h>
#include <TLegendEntry.h>
#include <TGraphAsymmErrors.h>
#include <TMath.h>

#include <vector>
#include <iostream>
#include <map>

const int MAXJETS = 500;
const Int_t THRESHOLDS = 11;
const Double_t L1_THRESHOLD[THRESHOLDS] = {16,20,32,36,40,44,52,68,80,92,128};

void makeTurnOn_fromTree()
{
  //TFile *inFile = TFile::Open("hydjet_jets_compTree.root");
  //TFile *inFile = TFile::Open("/export/d00/scratch/luck/jet55_data_compTree_combined.root");
  TFile *inFile = TFile::Open("hydjet_276_jets_compTree.root");
  TTree *inTree = (TTree*)inFile->Get("l1_photon_tree");

  Int_t run, lumi, evt;

  Int_t nl1Jet, nPuJet;//, nVsJet;
  Int_t l1Jet_hwPt[MAXJETS], l1Jet_hwEta[MAXJETS], l1Jet_hwPhi[MAXJETS], l1Jet_hwQual[MAXJETS];
  Float_t l1Jet_pt[MAXJETS], l1Jet_eta[MAXJETS], l1Jet_phi[MAXJETS];
  Float_t PuJet_pt[MAXJETS], PuJet_eta[MAXJETS], PuJet_phi[MAXJETS], PuJet_rawpt[MAXJETS];
  //Float_t VsJet_pt[MAXJETS], VsJet_eta[MAXJETS], VsJet_phi[MAXJETS], VsJet_rawpt[MAXJETS];
  Bool_t goodEvent;
  Int_t hiBin;

  Int_t L1_SingleEG2_BptxAND, L1_SingleEG5_BptxAND, L1_SingleEG8_BptxAND, L1_SingleEG12;

  Int_t nGenJet;
  Float_t genJet_pt[MAXJETS], genJet_eta[MAXJETS], genJet_phi[MAXJETS];


  inTree->SetBranchAddress("run",&run);
  inTree->SetBranchAddress("lumi",&lumi);
  inTree->SetBranchAddress("evt",&evt);

  inTree->SetBranchAddress("goodEvent",&goodEvent);
  inTree->SetBranchAddress("hiBin",&hiBin);
  inTree->SetBranchAddress("nl1Jet",&nl1Jet);
  inTree->SetBranchAddress("nPuJet",&nPuJet);
//  inTree->SetBranchAddress("nVsJet",&nVsJet);
  inTree->SetBranchAddress("l1Jet_hwPt",l1Jet_hwPt);
  inTree->SetBranchAddress("l1Jet_hwEta",l1Jet_hwEta);
  inTree->SetBranchAddress("l1Jet_hwPhi",l1Jet_hwPhi);
  inTree->SetBranchAddress("l1Jet_hwQual",l1Jet_hwQual);
  inTree->SetBranchAddress("l1Jet_pt",l1Jet_pt);
  inTree->SetBranchAddress("l1Jet_eta",l1Jet_eta);
  inTree->SetBranchAddress("l1Jet_phi",l1Jet_phi);
  inTree->SetBranchAddress("PuJet_pt",PuJet_pt);
  inTree->SetBranchAddress("PuJet_eta",PuJet_eta);
  inTree->SetBranchAddress("PuJet_phi",PuJet_phi);
  inTree->SetBranchAddress("PuJet_rawpt",PuJet_rawpt);
  // inTree->SetBranchAddress("VsJet_pt",VsJet_pt);
  // inTree->SetBranchAddress("VsJet_eta",VsJet_eta);
  // inTree->SetBranchAddress("VsJet_phi",VsJet_phi);
  // inTree->SetBranchAddress("VsJet_rawpt",VsJet_rawpt);

  inTree->SetBranchAddress("nGenJet",&nGenJet);
  inTree->SetBranchAddress("genJet_pt",genJet_pt);
  inTree->SetBranchAddress("genJet_eta",genJet_eta);
  inTree->SetBranchAddress("genJet_phi",genJet_phi);

  inTree->SetBranchAddress("L1_SingleEG2_BptxAND", &L1_SingleEG2_BptxAND);
  inTree->SetBranchAddress("L1_SingleEG5_BptxAND", &L1_SingleEG5_BptxAND);
  inTree->SetBranchAddress("L1_SingleEG8_BptxAND", &L1_SingleEG8_BptxAND);
  inTree->SetBranchAddress("L1_SingleEG12", &L1_SingleEG12);


  TFile *outFile = new TFile(Form("hist_hydjet_276_jets_gen.root"),"RECREATE");
  outFile->cd();

  const int nBins = 75;
  const double maxPt = 300;

  TH1D *l1Pt = new TH1D("l1Pt",";L1 p_{T} (GeV)",nBins,0,maxPt);
  TH1D *fPt[3];
  fPt[0] = new TH1D("fPt_0",";offline p_{T} (GeV)",nBins,0,maxPt);
  fPt[1] = (TH1D*)fPt[0]->Clone("fPt_1");
  fPt[2] = (TH1D*)fPt[0]->Clone("fPt_2");
  TH1D *accepted[THRESHOLDS][3];

  for(int i = 0; i < THRESHOLDS; ++i)
    for(int j = 0; j < 3; ++j)
    {
      accepted[i][j] = new TH1D(Form("accepted_pt%d_%d",(int)L1_THRESHOLD[i],j),";offline p_{T}",nBins,0,maxPt);
    }

  TH2D *corr = new TH2D("corr",";offline p_{T};l1 p_{T}",nBins,0,maxPt,nBins,0,maxPt);

  Long64_t entries = inTree->GetEntries();
  for(Long64_t j = 0; j < entries; ++j)
  {
    if(j % 10000 == 0)
      printf("%lld / %lld\n",j,entries);

    inTree->GetEntry(j);

    double maxl1pt = 0;
    for(int i = 0; i < nl1Jet; ++i)
    {
      if(l1Jet_pt[i] > maxl1pt)
	maxl1pt = l1Jet_pt[i];
    }

    double maxfpt = 0;
    // for(int i = 0; i < nPuJet; ++i)
    // {
    //   //if(TMath::Abs(PuJet_eta[i]) > 2) continue;
    //   if(PuJet_pt[i] > maxfpt) {
    // 	maxfpt = PuJet_pt[i];
    //   }
    // }
    for(int i = 0; i < nGenJet; ++i)
    {
      //if(TMath::Abs(genJet_eta[i]) > 2.0) continue;
      if(genJet_pt[i] > maxfpt)
    	maxfpt = genJet_pt[i];
    }
    //if(f_num > MAXJETS) std::cout << "TOO SMALL" << std::endl;
    l1Pt->Fill(maxl1pt);

    if(goodEvent)
    {
      fPt[0]->Fill(maxfpt);
      if(hiBin < 60)
	fPt[1]->Fill(maxfpt);
      else if (hiBin >= 100)
	fPt[2]->Fill(maxfpt);

      corr->Fill(maxfpt,maxl1pt);

      for(int k = 0; k < THRESHOLDS; ++k)
      {
	if(maxl1pt>L1_THRESHOLD[k])
	{
	  accepted[k][0]->Fill(maxfpt);
	  if(hiBin < 60)
	    accepted[k][1]->Fill(maxfpt);
	  else if (hiBin >= 100)
	    accepted[k][2]->Fill(maxfpt);
	}
      }
    }
  }

  TGraphAsymmErrors *a[THRESHOLDS][3];
  for(int k = 0; k < THRESHOLDS; ++k){
    for(int l = 0; l < 3; ++l)
    {
      a[k][l] = new TGraphAsymmErrors();
      a[k][l]->BayesDivide(accepted[k][l],fPt[l]);
      a[k][l]->SetName(Form("asymm_pt_%d_%d",(int)L1_THRESHOLD[k],l));
    }
  }

  l1Pt->Write();
  fPt[0]->Write();
  fPt[1]->Write();
  fPt[2]->Write();
  corr->Write();
  for(int k = 0; k < THRESHOLDS; ++k){
    for(int l = 0; l < 3; ++l)
    {
      accepted[k][l]->Write();
      a[k][l]->Write();
    }
  }

  inFile->Close();
  outFile->Close();
}

int main()
{
  makeTurnOn_fromTree();
  return 0;
}
