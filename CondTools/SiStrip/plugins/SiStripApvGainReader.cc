#include "CondFormats/SiStripObjects/interface/SiStripApvGain.h"
#include "CondFormats/DataRecord/interface/SiStripApvGainRcd.h"
#include "CalibFormats/SiStripObjects/interface/SiStripGain.h"
#include "CalibTracker/Records/interface/SiStripGainRcd.h"

#include "CondTools/SiStrip/plugins/SiStripApvGainReader.h"

#include <iostream>
#include <cstdio>
#include <sys/time.h>


using namespace cms;

SiStripApvGainReader::SiStripApvGainReader( const edm::ParameterSet& iConfig ):
  printdebug_(iConfig.getUntrackedParameter<bool>("printDebug",true)),
  formatedOutput_(iConfig.getUntrackedParameter<std::string>("outputFile","")),
  gainType_ (iConfig.getUntrackedParameter<uint32_t>("gainType",1)),
  tree_(0){
  if (fs_.isAvailable()){
    tree_=fs_->make<TTree>("Gains","Gains");

    tree_->Branch("Index",&id_,"Index/I");
    tree_->Branch("DetId",&detId_,"DetId/I");
    tree_->Branch("APVId",&apvId_,"APVId/I");
    tree_->Branch("Gain",&gain_,"Gain/D");
  }
}

SiStripApvGainReader::~SiStripApvGainReader(){}

void SiStripApvGainReader::analyze( const edm::Event& e, const edm::EventSetup& iSetup){

  edm::ESHandle<SiStripGain> SiStripApvGain_;
  iSetup.get<SiStripGainRcd>().get(SiStripApvGain_);
  edm::LogInfo("SiStripApvGainReader") << "[SiStripApvGainReader::analyze] End Reading SiStripApvGain" << std::endl;
  std::vector<uint32_t> detid;
  SiStripApvGain_->getDetIds(detid);
  edm::LogInfo("Number of detids ")  << detid.size() << std::endl;

  FILE* pFile=nullptr;
  if(formatedOutput_!="")pFile=fopen(formatedOutput_.c_str(), "w");
  for (size_t id=0;id<detid.size();id++){
    SiStripApvGain::Range range=SiStripApvGain_->getRange(detid[id], gainType_);	
    if(printdebug_){
       int apv=0;
       for(int it=0;it<range.second-range.first;it++){
          edm::LogInfo("SiStripApvGainReader")  << "detid " << detid[id] << " \t " << apv++ << " \t " << SiStripApvGain_->getApvGain(it,range)     << std::endl;        
	  id_++;

	  if (tree_){
	    detId_ = detid[id];
	    apvId_ = apv ;
	    gain_ = SiStripApvGain_->getApvGain(it,range);
	    tree_->Fill();
	  }
       }
    }

    if(pFile){
       fprintf(pFile,"%i ",detid[id]);
       for(int it=0;it<range.second-range.first;it++){
          fprintf(pFile,"%f ", SiStripApvGain_->getApvGain(it,range) );
       }fprintf(pFile, "\n");
    }

  }

  if(pFile)fclose(pFile);
}
