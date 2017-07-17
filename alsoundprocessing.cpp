#include "alsoundprocessing.h"
#include <alcommon/alproxy.h>
#include <iostream>
#include <ctime>

 using namespace std;
ALSoundProcessing::ALSoundProcessing(boost::shared_ptr<ALBroker> pBroker,
                                     std::string pName)
  : ALSoundExtractor(pBroker, pName)
{
  setModuleDescription("This module processes the data collected by the "
                       "microphones and output in the ALMemory the RMS power "
                       "of each of the four channels.");
}

void ALSoundProcessing::init()
{

  //unsigned int nb_slices = 5;
  //std::vector<float> EnergyVec_(nb_slices);
	std::vector<float> EnergyVec_(16000*0.85);//16 kHz and 85 ms buffer


 int frequency=16000;
  // 0: right;  1: rear;   2: left;   3: front,

  fALMemoryKeys.push_back("ALSoundProcessing/leftVec");
  fALMemoryKeys.push_back("ALSoundProcessing/rightVec");


  fProxyToALMemory.insertData(fALMemoryKeys[0], AL::ALValue(EnergyVec_));
  fProxyToALMemory.insertData(fALMemoryKeys[1], AL::ALValue(EnergyVec_));
 
  // fProxyToALMemory.insertData(fALMemoryKeys,0.0f);


  // Do not call the function setClientPreferences in your constructor!
  // setClientPreferences : can be called after its construction!
  audioDevice->callVoid("setClientPreferences",
                        getName(),                //Name of this module
                        frequency,                    //48000 Hz requested
                        (int)ALLCHANNELS,         //4 Channels requested
                        1                         //Deinterleaving requested
                        );
#ifdef SOUNDPROCESSING_IS_REMOTE
  qi::Application::atStop(boost::bind(&ALSoundProcessing::stopDetection, this));
#endif

  
  startDetection();
 std::cout << "Start sound buffer recording" <<  " at " << frequency<< " Hz"<<std::endl;
}

ALSoundProcessing::~ALSoundProcessing()
{
  stopDetection();
}


/// Description: The name of this method should not be modified as this
/// method is automatically called by the AudioDevice Module.
void ALSoundProcessing::process(const int & nbOfChannels,
                                const int & nbOfSamplesByChannel,
                                const AL_SOUND_FORMAT * buffer,
                                const ALValue & timeStamp)
{
  /// Computation of the RMS power of the signal delivered by
  /// each microphone on a 170ms buffer
  /// init RMS power to 0
  ///
 
	
 // clock_t begin = clock();

  unsigned int index_r = 3; 
  unsigned int index_l = 2;

 
  std::vector<float> Signal_l(nbOfSamplesByChannel);
  std::vector<float> Signal_r(nbOfSamplesByChannel);
  //std::cout << "nbOfSamplesByChannel" <<   nbOfSamplesByChannel << std::endl;
 



  float energy_left_treshold = 1.5e8;
  float energy_right_treshold = 1.5e8;
 
 
  for(int i = 0 ; i < nbOfSamplesByChannel ; i++){	
        Signal_l[i]=buffer[index_l*nbOfSamplesByChannel+i]/32768.;
	Signal_r[i]=buffer[index_r*nbOfSamplesByChannel+i]/32768.;
  }	

 


  /// Puts the result in ALMemory
  /// (for example to be easily retrieved by another module)
  //  for(int i=0 ; i<nbOfChannels ; i++)
  //  {
  //    fProxyToALMemory.insertData(fALMemoryKeys[i],fMicsEnergy[i]);
  //  }


  //fProxyToALMemory.insertData(fALMemoryKeys[0],fMicsEnergy[index_l]);
  //fProxyToALMemory.insertData(fALMemoryKeys[1],fMicsEnergy[index_r]);
  fProxyToALMemory.insertData(fALMemoryKeys[0], AL::ALValue(Signal_l));
  fProxyToALMemory.insertData(fALMemoryKeys[1], AL::ALValue(Signal_r));
   //std::cout << "EnergyVec_l " <<   Signal_l << std::endl;
   //std::cout << "Signal_l_r " <<   Signal_l << std::endl;

   //clock_t end = clock();
   //double elapsed_secs = double(end - begin) / CLOCKS_PER_SEC;
  // std::cout << "Time:" <<   elapsed_secs << std::endl;
   // std::cout << "Signal_l_r " <<   Signal_rb.size() << std::endl;


}
