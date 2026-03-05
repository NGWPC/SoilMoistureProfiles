#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#define BMI_SUCCESS (0)
#define BMI_FAILURE (1)

#define BMI_MAX_UNITS_NAME (2048)
#define BMI_MAX_TYPE_NAME (2048)
#define BMI_MAX_COMPONENT_NAME (2048)
#define BMI_MAX_VAR_NAME (2048)
#define BMI_MAX_LOCATION_NAME (2048) //OWP Custom

typedef struct Bmi_sacsma_struct{

    /* Opaque Fortran model pointer */
    void* data;

    /* Lifecycle */
    int (*initialize)(void* data, const char* config_file);
    int (*finalize)(void* data);
    int (*update)(void* data);
    int (*update_until)(void* data, double time);

    /* Model info */
    int (*get_component_name)(void* data, char* name);
    int (*get_input_item_count)(void* data, int* count);
    int (*get_output_item_count)(void* data, int* count);
    int (*get_input_var_names)(void* data, char** names);
    int (*get_output_var_names)(void* data, char** names);

    /* Time */
    int (*get_start_time)(void* data, double* time);
    int (*get_end_time)(void* data, double* time);
    int (*get_current_time)(void* data, double* time);
    int (*get_time_step)(void* data, double* dt);
    int (*get_time_units)(void* data, char* units);

    /* Variable metadata */
    int (*get_var_grid)(void* data, const char* name, int* grid);
    int (*get_var_type)(void* data, const char* name, char* type);
    int (*get_var_units)(void* data, const char* name, char* units);
    int (*get_var_itemsize)(void* data, const char* name, int* size);
    int (*get_var_nbytes)(void* data, const char* name, int* nbytes);
    int (*get_var_location)(void* data, const char* name, char* location);

    /* Grid metadata */
    int (*get_grid_type)(void* data, int grid, char* type);
    int (*get_grid_rank)(void* data, int grid, int* rank);
    int (*get_grid_size)(void* data, int grid, int* size);
    int (*get_grid_shape)(void* data, int grid, int* shape);
    int (*get_grid_spacing)(void* data, int grid, double* spacing);
    int (*get_grid_origin)(void* data, int grid, double* origin);
    int (*get_grid_x)(void* data, int grid, double* x);
    int (*get_grid_y)(void* data, int grid, double* y);
    int (*get_grid_z)(void* data, int grid, double* z);
    int (*get_grid_node_count)(void* data, int grid, int* count);
    int (*get_grid_edge_count)(void* data, int grid, int* count);
    int (*get_grid_face_count)(void* data, int grid, int* count);
    int (*get_grid_edge_nodes)(void* data, int grid, int* edge_nodes);
    int (*get_grid_face_edges)(void* data, int grid, int* face_edges);
    int (*get_grid_face_nodes)(void* data, int grid, int* face_nodes);
    int (*get_grid_nodes_per_face)(void* data, int grid, int* nodes);

    /* Get values */
    int (*get_value_int)(void* data, const char* name, int* dest);
    int (*get_value_float)(void* data, const char* name, float* dest);
    int (*get_value_double)(void* data, const char* name, double* dest);

    int (*get_value_ptr_int)(void* data, const char* name, int** dest);
    int (*get_value_ptr_float)(void* data, const char* name, float** dest);
    int (*get_value_ptr_double)(void* data, const char* name, double** dest);

    int (*get_value_at_indices_int)(void* data, const char* name,
                                    int* dest, const int* inds, int count);
    int (*get_value_at_indices_float)(void* data, const char* name,
                                      float* dest, const int* inds, int count);
    int (*get_value_at_indices_double)(void* data, const char* name,
                                       double* dest, const int* inds, int count);

    /* Set values */
    int (*set_value_int)(void* data, const char* name, const int* src);
    int (*set_value_float)(void* data, const char* name, const float* src);
    int (*set_value_double)(void* data, const char* name, const double* src);

    int (*set_value_at_indices_int)(void* data, const char* name,
                                    const int* inds, int count, const int* src);
    int (*set_value_at_indices_float)(void* data, const char* name,
                                      const int* inds, int count, const float* src);
    int (*set_value_at_indices_double)(void* data, const char* name,
                                       const int* inds, int count, const double* src);

} Bmi_sacsma_struct;

/* Registration helper */
Bmi_sacsma_struct* register_bmi_sac(Bmi_sacsma_struct* model);

#ifdef __cplusplus
}
#endif
