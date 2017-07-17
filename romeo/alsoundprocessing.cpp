#include "alsoundprocessing.h"
#include <alcommon/alproxy.h>
#include <iostream>

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

    unsigned int nb_slices = 5;
    std::vector<float> EnergyVec_(nb_slices);




    // 0: right;  1: rear;   2: left;   3: front,
    fALMemoryKeys.push_back("ALSoundProcessing/leftMicEnergy");
    fALMemoryKeys.push_back("ALSoundProcessing/rightMicEnergy");
    //  fALMemoryKeys.push_back("ALSoundProcessing/frontMicEnergy");
    //  fALMemoryKeys.push_back("ALSoundProcessing/rearMicEnergy");
    fALMemoryKeys.push_back("ALSoundProcessing/ratioRightLeft");
    fALMemoryKeys.push_back("ALSoundProcessing/soundDetectedMaxValue");
    fALMemoryKeys.push_back("ALSoundProcessing/soundDetectedEnergy");
    fALMemoryKeys.push_back("ALSoundProcessing/leftEnergyVec");
    fALMemoryKeys.push_back("ALSoundProcessing/rightEnergyVec");


    fProxyToALMemory.insertData(fALMemoryKeys[0],0.0f);
    fProxyToALMemory.insertData(fALMemoryKeys[1],0.0f);
    fProxyToALMemory.insertData(fALMemoryKeys[2],0.0f);
    fProxyToALMemory.insertData(fALMemoryKeys[3],0.0f);
    fProxyToALMemory.insertData(fALMemoryKeys[4],0.0f);
    fProxyToALMemory.insertData(fALMemoryKeys[5], AL::ALValue(EnergyVec_));
    fProxyToALMemory.insertData(fALMemoryKeys[6], AL::ALValue(EnergyVec_));

    // fProxyToALMemory.insertData(fALMemoryKeys,0.0f);


    // Do not call the function setClientPreferences in your constructor!
    // setClientPreferences : can be called after its construction!
    audioDevice->callVoid("setClientPreferences",
                          getName(),                //Name of this module
                          48000,                    //48000 Hz requested
                          (int)ALLCHANNELS,         //4 Channels requested
                          1                         //Deinterleaving requested
                          );
#ifdef SOUNDPROCESSING_IS_REMOTE
    qi::Application::atStop(boost::bind(&ALSoundProcessing::stopDetection, this));
#endif
    startDetection();
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
    std::vector<float> fMicsEnergy;
    unsigned int nb_slices = 10;

    std::vector<float> EnergyVec_l(2*nb_slices-1);
    std::vector<float> EnergyVec_r(2*nb_slices-1);


    for(int i=0 ; i<nbOfChannels ; i++)
    {
        fMicsEnergy.push_back(0.0f);
    }


    int maxValueFront = 0;
    int maxValueFront_treshold = 600;
    float energy_left_treshold = 1.5e8;
    float energy_right_treshold = 1.5e8;
    int sliceSamples=nbOfSamplesByChannel/nb_slices;
      /// Calculation of the RMS power
    for(int channelInd = 0 ; channelInd < nbOfChannels ; channelInd++)
    {
        for(int i = 0 ; i < nbOfSamplesByChannel ; i++)
        {
            fMicsEnergy[channelInd] += (float)buffer[nbOfSamplesByChannel*channelInd + i]
                    *(float)buffer[nbOfSamplesByChannel*channelInd + i];
            // fMicsEnergy[channelInd] += abs((float)buffer[nbOfSamplesByChannel*channelInd + i]);
            //*(float)buffer[nbOfSamplesByChannel*channelInd + i];

            // Compute the maximum value of the front microphone signal.
            if (channelInd == 3) // Front micro
            {
                if(buffer[nbOfSamplesByChannel*channelInd + i] > maxValueFront)
                {
                    maxValueFront = buffer[nbOfSamplesByChannel*channelInd + i];
                }
            }




        }
        if(channelInd==0)
            for(int cpt=0;cpt<nb_slices*2-1;cpt++)
                for(int i=0;i<sliceSamples;i++)
                    EnergyVec_r[cpt]+=  (float)buffer[nbOfSamplesByChannel*channelInd + i+cpt*sliceSamples/2]
                            *(float)buffer[nbOfSamplesByChannel*channelInd+i+cpt*sliceSamples/2];

        if(channelInd==2)
            for(int cpt=0;cpt<nb_slices*2-1;cpt++)
                for(int i=0;i<sliceSamples;i++)
                    EnergyVec_l[cpt]+=  (float)buffer[nbOfSamplesByChannel*channelInd + i+cpt*sliceSamples/2]
                            *(float)buffer[nbOfSamplesByChannel*channelInd+i+cpt*sliceSamples/2];
        //fMicsEnergy[channelInd] /= (float)nbOfSamplesByChannel;
        //  fMicsEnergy[channelInd] = sqrtf(fMicsEnergy[channelInd]);
    }


    /// Puts the result in ALMemory
    /// (for example to be easily retrieved by another module)
    //  for(int i=0 ; i<nbOfChannels ; i++)
    //  {
    //    fProxyToALMemory.insertData(fALMemoryKeys[i],fMicsEnergy[i]);
    //  }


    fProxyToALMemory.insertData(fALMemoryKeys[0],fMicsEnergy[2]);
    fProxyToALMemory.insertData(fALMemoryKeys[1],fMicsEnergy[0]);
    fProxyToALMemory.insertData(fALMemoryKeys[5], AL::ALValue(EnergyVec_l));
    fProxyToALMemory.insertData(fALMemoryKeys[6],AL::ALValue(EnergyVec_r));
    // Compute ratio right over left channel
    float ratio = fMicsEnergy[0] / fMicsEnergy[2];
    fProxyToALMemory.insertData(fALMemoryKeys[2],ratio);

    if (maxValueFront > maxValueFront_treshold)
    {
        fProxyToALMemory.insertData(fALMemoryKeys[3],float(1.));
        // std::cout << "Sound detected () " << std::endl;
    }
    else
        fProxyToALMemory.insertData(fALMemoryKeys[3],float(0.));



    // Check the energy of the left channel to detect if there is sound or just noise
    if (fMicsEnergy[2] > energy_left_treshold || fMicsEnergy[0]>energy_right_treshold)
    {
        fProxyToALMemory.insertData(fALMemoryKeys[4],float(1.));
        std::cout << "Sound!!!!!!!! " << std::endl;
    }
    else
        fProxyToALMemory.insertData(fALMemoryKeys[4],float(0.));


    //  /// Displays the results on the Naoqi console
    // std::cout << " Left  = " <<  fMicsEnergy[2] << "      Right  = " << fMicsEnergy[0] << std::endl<< "      Front  = " << fMicsEnergy[3] << std::endl;
    //std::cout << "MaxValueFront  = " << maxValueFront << std::endl;
    //std::cout << "Ratio R/L = " <<  ratio << std::endl;
    std::cout << "E-l= " <<  fMicsEnergy[2] << std::endl;
    std::cout << "E-r= " <<  fMicsEnergy[0] << std::endl;
    for(int i = 0 ; i < nb_slices*2-1 ; i++){
        std::cout << "EnergyVec_l " << i <<" " <<   EnergyVec_l[i] << std::endl;
         std::cout << "EnergyVec_r " << i <<" " <<   EnergyVec_r[i] << std::endl;
    }



    //  audioDevice->callVoid("enableEnergyComputation");
    //  float a;
    //audioDevice->callVoid("getFrontMicEnergy");
    // std::cout << "getFrontMicEnergy  = " <<  a << std::endl;

}
