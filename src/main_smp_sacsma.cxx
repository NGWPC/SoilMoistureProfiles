#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "bmi_struct_sacsma.hxx"
#include "bmi_sacsma.hxx"   // contains all extern "C" Fortran wrappers

#include "../bmi/bmi.hxx"
#include "bmi_soil_moisture_profile.hxx"
#include "soil_moisture_profile.hxx"
#include "Logger.hxx"


/*
  This pseudo-framework couples SACSMA and SoilMoistureProfiles modules to compute 

*/

/***************************************************************
 * Function to pass the parameters from SACSMA to SoilMoistureProfiles
***************************************************************/
void pass_data_from_sacsma_to_smp(Bmi_sacsma_struct sacsma_bmi, BmiSoilMoistureProfile *smp_bmi) {

  double var_value;      // variable in SACSMA
  std::string var_name = "land_surface_water__baseflow_volume_flux"; //variable name in SACSMA

  sacsma_bmi.get_value_double(sacsma_bmi.data, var_name.c_str(), &var_value);
  smp_bmi->SetValue("var_name_in_smp",&var_value);
}

/************************************************************************
    This main program is a mock framework.
    This is not part of BMI, but acts as the driver that calls the model.
************************************************************************/
int
main(int argc, const char *argv[]) {

  /************************************************************************
      A configuration file is required for running this model through BMI
  ************************************************************************/
  if(argc<=1) {
    printf("Expected path argument not received. Make sure to include path to config files.\n");
    LOG("Expected path argument not received. Make sure to include path to config files.", LogLevel::FATAL);
    exit(1);
  }

  BmiSoilMoistureProfile smp_bmi;

  /************************************************************************
      Registering the BMI model for SACSMA
  ************************************************************************/
  LOG("Registering BMI SACSMA", LogLevel::DEBUG);
  Bmi_sacsma_struct sacsma_bmi;
  if (!register_bmi_sacsma(&sacsma_bmi)) {
    LOG("Registering BMI SACSMA failed", LogLevel::FATAL);
    exit(1);
  }
  
  /************************************************************************
      Initializing the BMI for SACSMA
  ************************************************************************/

  LOG("Initializing BMI SACSMA: %s", argv[1]);
  const char *cfg_file_sacsma = argv[1];
  if (sacsma_bmi.initialize(sacsma_bmi.data, cfg_file_sacsma)!= 0) {
    LOG("SACSMA model initialization failed", LogLevel::FATAL);
    exit(1);
  }

  LOG("Initializing BMI SMP", LogLevel::INFO);
  const char *cfg_file_smp = argv[2];
  smp_bmi.Initialize(cfg_file_smp);

  

  int nstep;
  nstep = 720;


  /************************************************************************
    Now loop through time and call the models with the intermediate get/set
  ************************************************************************/
  LOG("Running SACSMA and SMP BMI's", LogLevel::INFO);

  // output files -- writing water table depth, soil moisture fraction, and soil moisture profiles to separate files
  ofstream fout, fout_wt;
  fout.open("smp_data.csv");
  fout_wt.open("water_table.csv");
  fout_wt << "water_table [m]"<<",soil_moisture_fraction"<<"\n";

  int nz = 20; // number of cells for soil discretization
  double *smc = new double[nz];
  double water_table;
  double soil_moisture_fraction;

  LOG("Looping through and calling update", LogLevel::DEBUG);
  for (int i = 0; i < nstep; i++) {

    sacsma_bmi.update(sacsma_bmi.data);

    pass_data_from_sacsma_to_smp(sacsma_bmi, &smp_bmi);   // Get and Set values


    smp_bmi.Update();

    smp_bmi.GetValue("soil_moisture_profile",&smc[0]);
    smp_bmi.GetValue("soil_water_table",&water_table);
    smp_bmi.GetValue("soil_moisture_fraction",&soil_moisture_fraction);

    fout<<i<<",";
    for (int k = 0; k < 20; k++)
      fout <<smc[k]<<",";
    fout<< std::endl;

    fout_wt << water_table <<","<< soil_moisture_fraction <<"\n";
    //smp_bmi.PrintSoilMoistureProfile();
  }

  fout.close();

  /************************************************************************
    Finalize both the BMIs
  ************************************************************************/
  LOG(LogLevel::INFO, "Finalizing SACSMA and SMP BMIs");
  sacsma_bmi.finalize(sacsma_bmi.data);

  return 0;
}
