#ifndef BMI_SMP_C_INCLUDED
#define BMI_SMP_C_INCLUDED


#include <stdio.h>
#include <string>
#include <cstring>
#include <cstdlib>
#include <vector>
#include <algorithm>
#include "../bmi/bmi.hxx"
#include "../include/bmi_soil_moisture_profile.hxx"
#include "../include/soil_moisture_profile.hxx"
#include "Logger.hxx"

#include <boost/serialization/serialization.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <cmath>
using std::isfinite;   

void BmiSoilMoistureProfile::
Initialize (std::string config_file)
{
#ifdef EWTS_HAVE_NGEN_BRIDGE
  EwtsInit(EWTS_ID_SMP, true);
#else
  EwtsInit(EWTS_ID_SMP, false);
#endif

  LOG(LogLevel::INFO, "Initializing SMP");

  if (config_file.compare("") != 0 ) {
    this->state = new soil_moisture_profile::soil_profile_parameters();
    soil_moisture_profile::SoilMoistureProfile(config_file, state);
  }
  else {
    LOG(LogLevel::FATAL, "SMP %s config file not provided", config_file.c_str());
    throw std::runtime_error("Missing SMP Config file");
  }

  this->verbosity = this->state->verbosity;
}


void BmiSoilMoistureProfile::
Update()
{
  soil_moisture_profile::SoilMoistureProfileUpdate(state);
}


void BmiSoilMoistureProfile::
UpdateUntil(double t)
{
  soil_moisture_profile::SoilMoistureProfileUpdate(state);
}


void BmiSoilMoistureProfile::
Finalize()
{
  if (this->state) {
    delete [] this->state->soil_moisture_profile;
    this->state->soil_moisture_profile = nullptr;

    delete [] this->state->smcmax;
    this->state->smcmax = nullptr;

    delete [] this->state->soil_z;
    this->state->soil_z = nullptr;

    delete [] this->state->soil_depth_layers;
    this->state->soil_depth_layers = nullptr;

    delete this->state;
    this->state = nullptr;
  }

  this->free_serialized();
}

void BmiSoilMoistureProfile::
PrintSoilMoistureProfile()
{
  soil_moisture_profile::PrintSoilMoistureProfile(this->state);
}

int BmiSoilMoistureProfile::
GetVarGrid(std::string name)
{
  if (
    name.compare("soil_storage_model") == 0
    || name.compare("num_wetting_fronts") == 0
    || name.compare("serialization_free") == 0
  ) // int
    return 0;
  else if (
    name.compare("soil_storage") == 0
    || name.compare("soil_storage_change") == 0
	  || name.compare("soil_water_table") == 0
    || name.compare("soil_moisture_fraction") == 0
    || name.compare("Qb_topmodel") == 0
    || name.compare("Qv_topmodel") == 0
    || name.compare("global_deficit") == 0
    || name.compare("b") == 0
    || name.compare("satpsi") == 0
    || name.compare("reset_time") == 0
  ) // double
    return 1;
  else if (
    name.compare("soil_moisture_profile") == 0
  ) // array of doubles (conceptual model)
    return 2;
  else if (
    name.compare("soil_moisture_wetting_fronts") == 0
    || name.compare("soil_depth_wetting_fronts") == 0
  ) // array of doubles (layered model)
    return 3;
  else if (
    name.compare("smcmax") == 0
  ) // fixed number of layers for calibratable params
    return 4;
  else if (
    name.compare("serialization_state") == 0
  ) // char* beginning of serialized data
    return 5;
  else if (
    name.compare("serialization_create") == 0
    || name.compare("serialization_size") == 0
  ) // uint64_t
    return 6;
  else
    return -1;
}


std::string BmiSoilMoistureProfile::
GetVarType(std::string name)
{
  int var_grid = GetVarGrid(name);

  if (var_grid == 0)
    return "int";
  else if (var_grid == 1 || var_grid == 2 || var_grid == 3 || var_grid == 4)
    return "double";
  else if (var_grid == 5)
    return "char";
  else if (var_grid == 6)
    return "uint64_t";
  else
    return "";
}


int BmiSoilMoistureProfile::
GetVarItemsize(std::string name)
{
  std::string var_type = GetVarType(name);

  if (var_type.compare("int") == 0)
    return sizeof(int);
  else if (var_type.compare("double") == 0)
    return sizeof(double);
  else if (var_type.compare("char") == 0)
    return sizeof(char);
  else if (var_type.compare("uint64_t") == 0)
    return sizeof(uint64_t);
  else
    return 0;
}


std::string BmiSoilMoistureProfile::
GetVarUnits(std::string name)
{
  if (name.compare("soil_storage") == 0 || name.compare("soil_storage_change") == 0 ||
      name.compare("soil_water_table") == 0)
    return "m";
  else if (name.compare("soil_moisture_profile") == 0 || name.compare("soil_moisture_wetting_fronts") == 0 ||
	   name.compare("soil_moisture_fraction") == 0)
    return "1";  // dimensionless (UDUNITS)
  else if (name.compare("Qb_topmodel") == 0 || name.compare("Qv_topmodel") == 0)
    return "m h^-1";
  else if (name.compare("global_deficit") == 0)
    return "m";
  else if (name.compare("soil_depth_wetting_fronts") == 0)
    return "m";
  else
    return "none";
}


int BmiSoilMoistureProfile::
GetVarNbytes(std::string name)
{
  int itemsize;
  int gridsize;

  itemsize = this->GetVarItemsize(name);
  gridsize = this->GetGridSize(this->GetVarGrid(name));
  return itemsize * gridsize;
}


std::string BmiSoilMoistureProfile::
GetVarLocation(std::string name)
{
  int var_grid = GetVarGrid(name);

  if (var_grid <= 3 && name != "serialization_free")
    return "node";
  else
    return "none";
}

void BmiSoilMoistureProfile::
GetGridShape(const int grid, int *shape)
{
  if (shape == nullptr) {
    LOG(LogLevel::FATAL, "GetGridShape received null shape pointer");
    throw std::runtime_error("Null shape pointer");
  }

  if (grid == 0 || grid == 1 || grid == 5 || grid == 6) {
    shape[0] = 1;
  }
  else if (grid == 2) {
    shape[0] = this->state->shape[0];
  }
  else if (grid == 3) {
    shape[0] = this->state->shape[1];
  }
  else if (grid == 4) {
    shape[0] = this->state->num_layers;
  }
  else {
    LOG(LogLevel::FATAL, "Invalid grid id %d in GetGridShape", grid);
    throw std::runtime_error("Invalid grid id in GetGridShape");
  }
}

void BmiSoilMoistureProfile::
GetGridSpacing (const int grid, double * spacing)
{
  if (grid == 0) {
    spacing[0] = this->state->spacing[0];
  }
}


void BmiSoilMoistureProfile::
GetGridOrigin (const int grid, double *origin)
{
  if (grid == 0) {
    origin[0] = this->state->origin[0];
  }
}

int BmiSoilMoistureProfile::
GetGridRank(const int grid)
{
  if (grid >= 0 && grid <= 6)
    return 1;
  else
    return -1;
}

int BmiSoilMoistureProfile::
GetGridSize(const int grid)
{
  if (grid == 0 || grid == 1 || grid == 6)
    return 1;
  else if (grid == 2)
    return this->state->shape[0];
  else if (grid == 3)
    return this->state->shape[1];
  else if (grid == 4)
    return this->state->num_layers;
  else if (grid == 5)
    return this->m_serialized_length; // size of currently saved state
  else
    return -1;
}


std::string BmiSoilMoistureProfile::
GetGridType(const int grid)
{
  if (grid == 0)
    return "uniform_rectilinear";
  else
    return "";
}


void BmiSoilMoistureProfile::
GetGridX(const int grid, double *x)
{
  LOG(LogLevel::SEVERE, "GetGridX Not Implemented");
  throw coupler::NotImplemented();
}


void BmiSoilMoistureProfile::
GetGridY(const int grid, double *y)
{
  LOG(LogLevel::SEVERE, "GetGridY Not Implemented");
  throw coupler::NotImplemented();
}


void BmiSoilMoistureProfile::
GetGridZ(const int grid, double *z)
{
  LOG(LogLevel::SEVERE, "GetGridZ Not Implemented");
  throw coupler::NotImplemented();
}


int BmiSoilMoistureProfile::
GetGridNodeCount(const int grid)
{
  LOG(LogLevel::SEVERE, "GetGridNodeCount Not Implemented");
  throw coupler::NotImplemented();
  /*
  if (grid == 0)
    return this->state->shape[0];
  else
    return -1;
  */
}


int BmiSoilMoistureProfile::
GetGridEdgeCount(const int grid)
{
  LOG(LogLevel::SEVERE, "GetGridEdgeCount Not Implemented");
  throw coupler::NotImplemented();
}


int BmiSoilMoistureProfile::
GetGridFaceCount(const int grid)
{
  LOG(LogLevel::SEVERE, "GetGridFaceCount Not Implemented");
  throw coupler::NotImplemented();
}


void BmiSoilMoistureProfile::
GetGridEdgeNodes(const int grid, int *edge_nodes)
{
  LOG(LogLevel::SEVERE, "GetGridEdgeNodes Not Implemented");
  throw coupler::NotImplemented();
}


void BmiSoilMoistureProfile::
GetGridFaceEdges(const int grid, int *face_edges)
{
  LOG(LogLevel::SEVERE, "GetGridFaceEdges Not Implemented");
  throw coupler::NotImplemented();
}


void BmiSoilMoistureProfile::
GetGridFaceNodes(const int grid, int *face_nodes)
{
  LOG(LogLevel::SEVERE, "GetGridFaceNodes Not Implemented");
  throw coupler::NotImplemented();
}


void BmiSoilMoistureProfile::
GetGridNodesPerFace(const int grid, int *nodes_per_face)
{
  LOG(LogLevel::SEVERE, "GetGridNodesPerFace Not Implemented");
  throw coupler::NotImplemented();
}

void BmiSoilMoistureProfile::
GetValue (std::string name, void *dest)
{
  if (dest == nullptr) {
    LOG(LogLevel::FATAL, "GetValue received null destination for variable %s", name.c_str());
    throw std::runtime_error("Null destination in GetValue");
  }

  void *src = this->GetValuePtr(name);
  int nbytes = this->GetVarNbytes(name);

  if (src == nullptr) {
    LOG(LogLevel::FATAL, "GetValue received null source for variable %s", name.c_str());
    throw std::runtime_error("Null source in GetValue");
  }

  if (nbytes <= 0) {
    LOG(LogLevel::FATAL, "GetValue computed invalid nbytes=%d for variable %s", nbytes, name.c_str());
    throw std::runtime_error("Invalid nbytes in GetValue");
  }

  memcpy(dest, src, nbytes);
}

void *BmiSoilMoistureProfile::
GetValuePtr (std::string name)
{
  if (this->state == nullptr) {
    LOG(LogLevel::FATAL, "GetValuePtr called before Initialize for variable %s", name.c_str());
    throw std::runtime_error("Null state in GetValuePtr");
  }

  if (name.compare("soil_storage") == 0)
    return (void*)(&this->state->soil_storage);
  else if (name.compare("soil_storage_change") == 0)
    return (void*)(&this->state->soil_storage_change_per_timestep);
  else if (name.compare("soil_water_table") == 0)
    return (void*)(&this->state->water_table_depth);
  else if (name.compare("soil_moisture_fraction") == 0)
    return (void*)(&this->state->soil_moisture_fraction);
  else if (name.compare("soil_moisture_profile") == 0) {
    int nz = this->state->shape[0];

    if (nz <= 0) {
      LOG(LogLevel::FATAL, "Invalid shape[0]=%d for soil_moisture_profile", nz);
      throw std::runtime_error("Invalid soil_moisture_profile size");
    }

    if (this->state->soil_moisture_profile == nullptr) {
      LOG(LogLevel::FATAL, "soil_moisture_profile pointer is null");
      throw std::runtime_error("Null soil_moisture_profile");
    }

    for (int i = 0; i < nz; ++i) {
      double v = this->state->soil_moisture_profile[i];

      if (!std::isfinite(v)) {
        LOG(LogLevel::FATAL,
            "Invalid soil_moisture_profile[%d]=%e before BMI export", i, v);
        throw std::runtime_error("Invalid soil_moisture_profile passed to model");
      }
    }

    return (void*)this->state->soil_moisture_profile;
  }
  else if (name.compare("soil_moisture_wetting_fronts") == 0)
    return this->state->soil_moisture_wetting_fronts.data();
  else if (name.compare("soil_depth_wetting_fronts") == 0)
    return this->state->soil_depth_wetting_fronts.data();
  else if (name.compare("soil_storage_model") == 0)
    return (void*)(&this->state->soil_storage_model);
  else if (name.compare("num_wetting_fronts") == 0)
    return (void*)(&this->state->num_wetting_fronts);
  else if (name.compare("Qb_topmodel") == 0)
    return (void*)(&this->state->Qb_topmodel);
  else if (name.compare("Qv_topmodel") == 0)
    return (void*)(&this->state->Qv_topmodel);
  else if (name.compare("global_deficit") == 0)
    return (void*)(&this->state->global_deficit);
  else if (name.compare("smcmax") == 0) {
    if (this->state->smcmax == nullptr) {
      LOG(LogLevel::FATAL, "smcmax pointer is null");
      throw std::runtime_error("Null smcmax");
    }
    return (void*)this->state->smcmax;
  }
  else if (name.compare("b") == 0)
    return (void*)(&this->state->b);
  else if (name.compare("satpsi") == 0)
    return (void*)(&this->state->satpsi);
  else if (name.compare("serialization_state") == 0)
    return (void*)(this->m_serialized.data());
  else if (name.compare("serialization_size") == 0)
    return (void*)(&this->m_serialized_length);
  else {
    std::stringstream errMsg;
    errMsg << "variable " << name << " does not exist";
    LOG(LogLevel::FATAL, errMsg.str());
    throw std::runtime_error(errMsg.str());
  }
}

void BmiSoilMoistureProfile::
GetValueAtIndices (std::string name, void *dest, int *inds, int len)
{
  if (dest == nullptr || inds == nullptr) {
    LOG(LogLevel::FATAL, "GetValueAtIndices received null input for variable %s", name.c_str());
    throw std::runtime_error("Null pointer in GetValueAtIndices");
  }

  void *src = this->GetValuePtr(name);
  if (src == nullptr) {
    LOG(LogLevel::FATAL, "GetValueAtIndices received null source for variable %s", name.c_str());
    throw std::runtime_error("Null source in GetValueAtIndices");
  }

  int itemsize = this->GetVarItemsize(name);
  int gridsize = this->GetGridSize(this->GetVarGrid(name));

  if (itemsize <= 0 || gridsize <= 0) {
    LOG(LogLevel::FATAL, "Invalid itemsize=%d or gridsize=%d for variable %s",
        itemsize, gridsize, name.c_str());
    throw std::runtime_error("Invalid size info in GetValueAtIndices");
  }

  for (int i = 0; i < len; i++) {
    if (inds[i] < 0 || inds[i] >= gridsize) {
      LOG(LogLevel::FATAL, "Index %d out of bounds for variable %s with gridsize=%d",
          inds[i], name.c_str(), gridsize);
      throw std::out_of_range("Index out of bounds in GetValueAtIndices");
    }

    int offset = inds[i] * itemsize;
    memcpy((char*)dest + i * itemsize, (char*)src + offset, itemsize);
  }
}

void BmiSoilMoistureProfile::
ResetSize (std::string name)
{
  if (this->state == nullptr) {
    LOG(LogLevel::FATAL, "ResetSize called with null state");
    throw std::runtime_error("Null state in ResetSize");
  }

  if (name.compare("soil_moisture_wetting_fronts") == 0 ||
      name.compare("soil_depth_wetting_fronts") == 0) {

    if (this->state->num_wetting_fronts <= 0) {
      std::string error_msg =
        "The number of wetting fronts must be greater than zero. Current num_wetting_fronts = " +
        std::to_string(this->state->num_wetting_fronts);
      LOG(LogLevel::FATAL, error_msg);
      throw std::out_of_range(error_msg);
    }

    this->state->shape[1] = this->state->num_wetting_fronts;

    if (name.compare("soil_moisture_wetting_fronts") == 0)
      this->state->soil_moisture_wetting_fronts.resize(this->state->num_wetting_fronts);
    else
      this->state->soil_depth_wetting_fronts.resize(this->state->num_wetting_fronts);
  }
}

void BmiSoilMoistureProfile::
SetValue (std::string name, void *src)
{
  if (src == nullptr &&
      name != "serialization_create" &&
      name != "serialization_free" &&
      name != "reset_time") {
    LOG(LogLevel::FATAL, "SetValue received null source for variable %s", name.c_str());
    throw std::runtime_error("Null source in SetValue");
  }

  // serialization special cases
  if (name.compare("serialization_state") == 0) {
    this->load_serialized((char*)src);
    return;
  }
  else if (name.compare("serialization_create") == 0) {
    this->new_serialized();
    return;
  }
  else if (name.compare("serialization_free") == 0) {
    this->free_serialized();
    return;
  }
  else if (name == "reset_time") {
    return;
  }

  if (this->state == nullptr) {
    LOG(LogLevel::FATAL, "SetValue called before Initialize for variable %s", name.c_str());
    throw std::runtime_error("Null state in SetValue");
  }

  // Handle num_wetting_fronts first, before resizing dependent vectors
  if (name.compare("num_wetting_fronts") == 0) {
    int new_num_wetting_fronts = *(static_cast<int*>(src));

    if (new_num_wetting_fronts <= 0) {
      LOG(LogLevel::FATAL,
          "Invalid num_wetting_fronts=%d. Must be > 0",
          new_num_wetting_fronts);
      throw std::out_of_range("Invalid num_wetting_fronts in SetValue");
    }

    this->state->num_wetting_fronts = new_num_wetting_fronts;
    this->state->shape[1] = new_num_wetting_fronts;

    if ((int)this->state->soil_moisture_wetting_fronts.size() != new_num_wetting_fronts)
      this->state->soil_moisture_wetting_fronts.resize(new_num_wetting_fronts);

    if ((int)this->state->soil_depth_wetting_fronts.size() != new_num_wetting_fronts)
      this->state->soil_depth_wetting_fronts.resize(new_num_wetting_fronts);

    LOG(LogLevel::INFO,
        "SMP SetValue num_wetting_fronts=%d src_ptr=%p",
        this->state->num_wetting_fronts, src);

    return;
  }

  ResetSize(name);

  void *dest = this->GetValuePtr(name);
  if (dest == nullptr) {
    LOG(LogLevel::FATAL, "SetValue got null destination for variable %s", name.c_str());
    throw std::runtime_error("Null destination in SetValue");
  }

  int nbytes = this->GetVarNbytes(name);
  if (nbytes <= 0) {
    LOG(LogLevel::FATAL, "SetValue computed invalid nbytes=%d for variable %s", nbytes, name.c_str());
    throw std::runtime_error("Invalid nbytes in SetValue");
  }

  if (name.compare("soil_storage") == 0) {
    memcpy(dest, src, nbytes);

    double v = this->state->soil_storage;
    if (!std::isfinite(v)) {
      LOG(LogLevel::FATAL, "soil_storage=%e is not finite in SetValue", v);
      throw std::runtime_error("Non-finite soil_storage in SetValue");
    }
  }
  else if (name.compare("soil_storage_change_per_timestep") == 0 ||
           name.compare("soil_storage_change") == 0) {
    memcpy(dest, src, nbytes);

    double v = this->state->soil_storage_change_per_timestep;
    if (!std::isfinite(v)) {
      LOG(LogLevel::FATAL, "soil_storage_change_per_timestep=%e is not finite in SetValue", v);
      throw std::runtime_error("Non-finite soil_storage_change_per_timestep in SetValue");
    }
  }
  else if (name.compare("Qb_topmodel") == 0) {
    memcpy(dest, src, nbytes);

    double v = this->state->Qb_topmodel;
    if (!std::isfinite(v)) {
      LOG(LogLevel::FATAL, "Qb_topmodel=%e is not finite in SetValue", v);
      throw std::runtime_error("Non-finite Qb_topmodel in SetValue");
    }
  }
  else if (name.compare("Qv_topmodel") == 0) {
    memcpy(dest, src, nbytes);

    double v = this->state->Qv_topmodel;
    if (!std::isfinite(v)) {
      LOG(LogLevel::FATAL, "Qv_topmodel=%e is not finite in SetValue", v);
      throw std::runtime_error("Non-finite Qv_topmodel in SetValue");
    }
  }
  else if (name.compare("global_deficit") == 0) {
    memcpy(dest, src, nbytes);

    double v = this->state->global_deficit;
    if (!std::isfinite(v)) {
      LOG(LogLevel::FATAL, "global_deficit=%e is not finite in SetValue", v);
      throw std::runtime_error("Non-finite global_deficit in SetValue");
    }
  }
  else if (name.compare("soil_moisture_wetting_fronts") == 0) {
    int num_wf = this->state->num_wetting_fronts;

    if (num_wf <= 0) {
      LOG(LogLevel::FATAL,
          "soil_moisture_wetting_fronts received but num_wetting_fronts=%d",
          num_wf);
      throw std::runtime_error("Invalid num_wetting_fronts before soil_moisture_wetting_fronts copy");
    }

    if ((int)this->state->soil_moisture_wetting_fronts.size() < num_wf) {
      LOG(LogLevel::FATAL,
          "soil_moisture_wetting_fronts size=%zu smaller than num_wetting_fronts=%d after resize",
          this->state->soil_moisture_wetting_fronts.size(),
          num_wf);
      throw std::runtime_error("soil_moisture_wetting_fronts size mismatch in SetValue");
    }

    LOG(LogLevel::INFO,
        "SMP SetValue BEFORE memcpy: %s num_wf=%d nbytes=%d src_ptr=%p dest_ptr=%p",
        name.c_str(), num_wf, nbytes, src, dest);

    double* src_d = static_cast<double*>(src);
    for (int i = 0; i < num_wf; i++) {
      LOG(LogLevel::INFO,
          "SMP SetValue SRC soil_moisture_wetting_fronts[%d]=%e addr=%p",
          i, src_d[i], &src_d[i]);
    }

    memcpy(dest, src, nbytes);

    for (int i = 0; i < num_wf; i++) {
      LOG(LogLevel::INFO,
          "SMP SetValue DEST soil_moisture_wetting_fronts[%d]=%e addr=%p",
          i, this->state->soil_moisture_wetting_fronts[i], &this->state->soil_moisture_wetting_fronts[i]);
    }

    double max_smc = -std::numeric_limits<double>::infinity();
    bool have_smcmax = false;

    if (this->state->smcmax != nullptr && this->state->num_layers > 0) {
      for (int j = 0; j < this->state->num_layers; j++) {
        if (!std::isfinite(this->state->smcmax[j]) || this->state->smcmax[j] <= 0.0) {
          LOG(LogLevel::FATAL,
              "Invalid smcmax[%d]=%e while validating soil_moisture_wetting_fronts",
              j, this->state->smcmax[j]);
          throw std::runtime_error("Invalid smcmax in SetValue");
        }
        if (!have_smcmax || this->state->smcmax[j] > max_smc) {
          max_smc = this->state->smcmax[j];
          have_smcmax = true;
        }
      }
    }

    for (int i = 0; i < num_wf; i++) {
      double v = this->state->soil_moisture_wetting_fronts[i];

      if (!std::isfinite(v)) {
        LOG(LogLevel::FATAL,
            "soil_moisture_wetting_fronts[%d]=%e is not finite in SetValue",
            i, v);
        throw std::runtime_error("Non-finite soil_moisture_wetting_fronts in SetValue");
      }

      if (v < 0.0) {
        LOG(LogLevel::FATAL,
            "soil_moisture_wetting_fronts[%d]=%e must be >= 0 in SetValue",
            i, v);
        throw std::runtime_error("Negative soil_moisture_wetting_fronts in SetValue");
      }

      if (have_smcmax && v > max_smc) {
        LOG(LogLevel::FATAL,
            "soil_moisture_wetting_fronts[%d]=%e exceeds max_smc=%e in SetValue",
            i, v, max_smc);
        throw std::runtime_error("soil_moisture_wetting_fronts exceeds smcmax in SetValue");
      }
    }
  }
  else if (name.compare("soil_depth_wetting_fronts") == 0) {
    int num_wf = this->state->num_wetting_fronts;

    if (num_wf <= 0) {
      LOG(LogLevel::FATAL,
          "soil_depth_wetting_fronts received but num_wetting_fronts=%d",
          num_wf);
      throw std::runtime_error("Invalid num_wetting_fronts before soil_depth_wetting_fronts copy");
    }

    if ((int)this->state->soil_depth_wetting_fronts.size() < num_wf) {
      LOG(LogLevel::FATAL,
          "soil_depth_wetting_fronts size=%zu smaller than num_wetting_fronts=%d after resize",
          this->state->soil_depth_wetting_fronts.size(),
          num_wf);
      throw std::runtime_error("soil_depth_wetting_fronts size mismatch in SetValue");
    }

    LOG(LogLevel::INFO,
        "SMP SetValue BEFORE memcpy: %s num_wf=%d nbytes=%d src_ptr=%p dest_ptr=%p",
        name.c_str(), num_wf, nbytes, src, dest);

    double* src_d = static_cast<double*>(src);
    for (int i = 0; i < num_wf; i++) {
      LOG(LogLevel::INFO,
          "SMP SetValue SRC soil_depth_wetting_fronts[%d]=%e addr=%p",
          i, src_d[i], &src_d[i]);
    }

    memcpy(dest, src, nbytes);

    for (int i = 0; i < num_wf; i++) {
      LOG(LogLevel::INFO,
          "SMP SetValue DEST soil_depth_wetting_fronts[%d]=%e addr=%p",
          i, this->state->soil_depth_wetting_fronts[i], &this->state->soil_depth_wetting_fronts[i]);
    }

    for (int i = 0; i < num_wf; i++) {
      double z = this->state->soil_depth_wetting_fronts[i];

      if (!std::isfinite(z)) {
        LOG(LogLevel::FATAL,
            "soil_depth_wetting_fronts[%d]=%e is not finite in SetValue",
            i, z);
        throw std::runtime_error("Non-finite soil_depth_wetting_fronts in SetValue");
      }

      if (z < 0.0) {
        LOG(LogLevel::FATAL,
            "soil_depth_wetting_fronts[%d]=%e must be >= 0 in SetValue",
            i, z);
        throw std::runtime_error("Negative soil_depth_wetting_fronts in SetValue");
      }

      if (this->state->soil_depth > 0.0 && z > this->state->soil_depth) {
        LOG(LogLevel::FATAL,
            "soil_depth_wetting_fronts[%d]=%e exceeds soil_depth=%e in SetValue",
            i, z, this->state->soil_depth);
        throw std::runtime_error("soil_depth_wetting_fronts exceeds soil_depth in SetValue");
      }

      if (i > 0 && z < this->state->soil_depth_wetting_fronts[i - 1]) {
        LOG(LogLevel::FATAL,
            "soil_depth_wetting_fronts not monotonic at i=%d: prev=%e curr=%e in SetValue",
            i,
            this->state->soil_depth_wetting_fronts[i - 1],
            z);
        throw std::runtime_error("Non-monotonic soil_depth_wetting_fronts in SetValue");
      }
    }
  }
  else if (name.compare("smcmax") == 0) {
    memcpy(dest, src, nbytes);

    if (this->state->smcmax == nullptr || this->state->num_layers <= 0) {
      LOG(LogLevel::FATAL, "Invalid smcmax metadata in SetValue");
      throw std::runtime_error("Invalid smcmax metadata in SetValue");
    }

    for (int i = 0; i < this->state->num_layers; i++) {
      double v = this->state->smcmax[i];
      if (!std::isfinite(v) || v <= 0.0) {
        LOG(LogLevel::FATAL,
            "smcmax[%d]=%e is invalid in SetValue",
            i, v);
        throw std::runtime_error("Invalid smcmax in SetValue");
      }
    }
  }
  else if (name.compare("b") == 0) {
    memcpy(dest, src, nbytes);

    if (!std::isfinite(this->state->b) || this->state->b <= 0.0) {
      LOG(LogLevel::FATAL, "b=%e is invalid in SetValue", this->state->b);
      throw std::runtime_error("Invalid b in SetValue");
    }
  }
  else if (name.compare("satpsi") == 0) {
    memcpy(dest, src, nbytes);

    if (!std::isfinite(this->state->satpsi) || this->state->satpsi <= 0.0) {
      LOG(LogLevel::FATAL, "satpsi=%e is invalid in SetValue", this->state->satpsi);
      throw std::runtime_error("Invalid satpsi in SetValue");
    }
  }
  else {
    memcpy(dest, src, nbytes);
  }
}

void BmiSoilMoistureProfile::
SetValueAtIndices (std::string name, int *inds, int len, void *src)
{
  if (inds == nullptr || src == nullptr) {
    LOG(LogLevel::FATAL, "SetValueAtIndices received null pointer for variable %s", name.c_str());
    throw std::runtime_error("Null pointer in SetValueAtIndices");
  }

  if (this->state == nullptr) {
    LOG(LogLevel::FATAL, "SetValueAtIndices called before Initialize for variable %s", name.c_str());
    throw std::runtime_error("Null state in SetValueAtIndices");
  }

  ResetSize(name);

  void *dest = this->GetValuePtr(name);
  if (dest == nullptr) {
    LOG(LogLevel::FATAL, "SetValueAtIndices got null destination for variable %s", name.c_str());
    throw std::runtime_error("Null destination in SetValueAtIndices");
  }

  int itemsize = this->GetVarItemsize(name);
  int gridsize = this->GetGridSize(this->GetVarGrid(name));

  if (itemsize <= 0 || gridsize <= 0) {
    LOG(LogLevel::FATAL, "Invalid itemsize=%d or gridsize=%d for variable %s",
        itemsize, gridsize, name.c_str());
    throw std::runtime_error("Invalid size info in SetValueAtIndices");
  }

  for (int i = 0; i < len; i++) {
    if (inds[i] < 0 || inds[i] >= gridsize) {
      LOG(LogLevel::FATAL, "Index %d out of bounds for variable %s with gridsize=%d",
          inds[i], name.c_str(), gridsize);
      throw std::out_of_range("Index out of bounds in SetValueAtIndices");
    }

    int offset = inds[i] * itemsize;
    memcpy((char*)dest + offset, (char*)src + i * itemsize, itemsize);
  }

  if (name.compare("num_wetting_fronts") == 0)
    this->state->shape[1] = this->state->num_wetting_fronts;
}

std::string BmiSoilMoistureProfile::
GetComponentName()
{
  return "SoilMoistureProfiles BMI";
}


int BmiSoilMoistureProfile::
GetInputItemCount()
{
  return this->input_var_name_count;
}


int BmiSoilMoistureProfile::
GetOutputItemCount()
{
  return this->output_var_name_count;
}


std::vector<std::string> BmiSoilMoistureProfile::
GetInputVarNames()
{
  std::vector<std::string> names;
  
  for (int i=0; i<this->input_var_name_count; i++)
    names.push_back(this->input_var_names[i]);
  
  return names;
}


std::vector<std::string> BmiSoilMoistureProfile::
GetOutputVarNames()
{
  std::vector<std::string> names;

  for (int i=0; i<this->output_var_name_count; i++)
    names.push_back(this->output_var_names[i]);

  return names;
}


double BmiSoilMoistureProfile::
GetStartTime () {
  return 0.0;
}


double BmiSoilMoistureProfile::
GetEndTime () {
  return 0.0;
}


double BmiSoilMoistureProfile::
GetCurrentTime () {
  return 0.0;
}


std::string BmiSoilMoistureProfile::
GetTimeUnits() {
  return "s";
}


double BmiSoilMoistureProfile::
GetTimeStep () {
  return 0;
}


template<class Archive>
void BmiSoilMoistureProfile::
serialize(Archive& ar, const unsigned int version) {
  soil_moisture_profile::soil_profile_parameters* state = this->state;
  // size of array pointers assigned in initialization
  int size = state->ncells;

  // all three models (Conceptual, Layered, and Topmodel) create these three states
  ar & state->init_profile;
  ar & boost::serialization::make_array(state->soil_moisture_profile, size);
  ar & state->water_table_depth;

  // state regardless of model
  ar & state->soil_storage;
  ar & state->soil_moisture_fraction;
}


void BmiSoilMoistureProfile::
new_serialized() {
  // resize with space for size as a header
  this->m_serialized.resize(sizeof(uint64_t));
  boost::archive::binary_oarchive archive(this->m_serialized);
  try {
    archive << (*this);
    this->m_serialized_length = this->m_serialized.size();
    // store size of serialized data as header
    uint64_t serialized_size = this->m_serialized_length - sizeof(uint64_t);
    memcpy(this->m_serialized.data(), &serialized_size, sizeof(uint64_t));
  } catch (const std::exception &e) {
    LOG(LogLevel::WARNING, "Serializing SMP encounterd an error: %s", e.what());
    LOG(LogLevel::WARNING, "Set m_serialized_length = 0");
    this->m_serialized_length = 0;
    throw;
  }
}


void BmiSoilMoistureProfile::
load_serialized(char* data) {
  // get size from header of data
  uint64_t size;
  memcpy(&size, data, sizeof(uint64_t));
  // create stream from after header
  membuf stream(data + sizeof(uint64_t), size);
  boost::archive::binary_iarchive archive(stream);
  try {
    archive >> (*this);
  } catch (const std::exception &e) {
    LOG(LogLevel::SEVERE, "Deserializing SMP encounterd an error: %s", e.what());
    throw;
  }
  this->free_serialized();
}


void BmiSoilMoistureProfile::
free_serialized() {
  this->m_serialized.clear();
  this->m_serialized.shrink_to_fit();
  this->m_serialized_length = 0;
}


#endif
