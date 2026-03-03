#pragma once

#ifdef __cplusplus
extern "C" {
#endif

int register_bmi(void** model);

/* Lifecycle */
int bmi_initialize(void* model, const char* config_file);
int bmi_finalize(void* model);
int bmi_update(void* model);
int bmi_update_until(void* model, double time);

/* Model info*/
int bmi_get_component_name(void* model, char* name);
int bmi_get_input_item_count(void* model, int* count);
int bmi_get_output_item_count(void* model, int* count);
int bmi_get_input_var_names(void* model, char** names);
int bmi_get_output_var_names(void* model, char** names);


/* Time information */
int bmi_get_start_time(void* model, double* time);
int bmi_get_end_time(void* model, double* time);
int bmi_get_current_time(void* model, double* time);
int bmi_get_time_step(void* model, double* dt);
int bmi_get_time_units(void* model, char* units);

/* Variable metadata */
int bmi_get_var_grid(void* model, const char* name, int* grid);
int bmi_get_var_type(void* model, const char* name, char* type);
int bmi_get_var_units(void* model, const char* name, char* units);
int bmi_get_var_itemsize(void* model, const char* name, int* size);
int bmi_get_var_nbytes(void* model, const char* name, int* nbytes);
int bmi_get_var_location(void* model, const char* name, char* location);

/* Grid metadata */
int bmi_get_grid_type(void* model, int grid, char* type);
int bmi_get_grid_rank(void* model, int grid, int* rank);
int bmi_get_grid_size(void* model, int grid, int* size);
int bmi_get_grid_shape(void* model, int grid, int* shape);
int bmi_get_grid_spacing(void* model, int grid, double* spacing);
int bmi_get_grid_origin(void* model, int grid, double* origin);
int bmi_get_grid_x(void* model, int grid, double* x);
int bmi_get_grid_y(void* model, int grid, double* y);
int bmi_get_grid_z(void* model, int grid, double* z);
int bmi_get_grid_node_count(void* model, int grid, int* count);
int bmi_get_grid_edge_count(void* model, int grid, int* count);
int bmi_get_grid_face_count(void* model, int grid, int* count);
int bmi_get_grid_edge_nodes(void* model, int grid, int* edge_nodes);
int bmi_get_grid_face_edges(void* model, int grid, int* face_edges);
int bmi_get_grid_face_nodes(void* model, int grid, int* face_nodes);
int bmi_get_grid_nodes_per_face(void* model, int grid, int* nodes);

/* Get values */
int bmi_get_value_int(void* model, const char* name, int* dest);
int bmi_get_value_float(void* model, const char* name, float* dest);
int bmi_get_value_double(void* model, const char* name, double* dest);

int bmi_get_value_ptr_int(void* model, const char* name, int** dest);
int bmi_get_value_ptr_float(void* model, const char* name, float** dest);
int bmi_get_value_ptr_double(void* model, const char* name, double** dest);

int bmi_get_value_at_indices_int(void* model, const char* name,
                                 int* dest, const int* inds, int count);
int bmi_get_value_at_indices_float(void* model, const char* name,
                                   float* dest, const int* inds, int count);
int bmi_get_value_at_indices_double(void* model, const char* name,
                                    double* dest, const int* inds, int count);

/* Set values */
int bmi_set_value_int(void* model, const char* name, const int* src);
int bmi_set_value_float(void* model, const char* name, const float* src);
int bmi_set_value_double(void* model, const char* name, const double* src);

int bmi_set_value_at_indices_int(void* model, const char* name,
                                 const int* inds, int count, const int* src);
int bmi_set_value_at_indices_float(void* model, const char* name,
                                   const int* inds, int count, const float* src);
int bmi_set_value_at_indices_double(void* model, const char* name,
                                    const int* inds, int count, const double* src);

#ifdef __cplusplus
}
#endif
