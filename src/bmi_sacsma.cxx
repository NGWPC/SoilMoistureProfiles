#include "bmi_struct_sacsma.hxx"
#include "bmi_sacsma.hxx"   // contains all extern "C" Fortran wrappers

Bmi_sacsma_struct* register_bmi_sac(Bmi_sacsma_struct* model) {

    if (!model) return nullptr;

    /* Call Fortran register_bmi */
    if (register_bmi(&(model->data)) != 0)
        return nullptr;

    /* Assign all function pointers */
    model->initialize                   = bmi_initialize;
    model->finalize                     = bmi_finalize;
    model->update                       = bmi_update;
    model->update_until                  = bmi_update_until;

    model->get_component_name           = bmi_get_component_name;
    model->get_input_item_count         = bmi_get_input_item_count;
    model->get_output_item_count        = bmi_get_output_item_count;
    model->get_input_var_names          = bmi_get_input_var_names;
    model->get_output_var_names         = bmi_get_output_var_names;

    model->get_start_time               = bmi_get_start_time;
    model->get_end_time                 = bmi_get_end_time;
    model->get_current_time             = bmi_get_current_time;
    model->get_time_step                = bmi_get_time_step;
    model->get_time_units               = bmi_get_time_units;

    model->get_var_grid                 = bmi_get_var_grid;
    model->get_var_type                 = bmi_get_var_type;
    model->get_var_units                = bmi_get_var_units;
    model->get_var_itemsize             = bmi_get_var_itemsize;
    model->get_var_nbytes               = bmi_get_var_nbytes;
    model->get_var_location             = bmi_get_var_location;

    model->get_grid_type                = bmi_get_grid_type;
    model->get_grid_rank                = bmi_get_grid_rank;
    model->get_grid_size                = bmi_get_grid_size;
    model->get_grid_shape               = bmi_get_grid_shape;
    model->get_grid_spacing             = bmi_get_grid_spacing;
    model->get_grid_origin              = bmi_get_grid_origin;
    model->get_grid_x                   = bmi_get_grid_x;
    model->get_grid_y                   = bmi_get_grid_y;
    model->get_grid_z                   = bmi_get_grid_z;
    model->get_grid_node_count          = bmi_get_grid_node_count;
    model->get_grid_edge_count          = bmi_get_grid_edge_count;
    model->get_grid_face_count          = bmi_get_grid_face_count;
    model->get_grid_edge_nodes          = bmi_get_grid_edge_nodes;
    model->get_grid_face_edges          = bmi_get_grid_face_edges;
    model->get_grid_face_nodes          = bmi_get_grid_face_nodes;
    model->get_grid_nodes_per_face      = bmi_get_grid_nodes_per_face;

    model->get_value_int                = bmi_get_value_int;
    model->get_value_float              = bmi_get_value_float;
    model->get_value_double             = bmi_get_value_double;

    model->get_value_ptr_int            = bmi_get_value_ptr_int;
    model->get_value_ptr_float          = bmi_get_value_ptr_float;
    model->get_value_ptr_double         = bmi_get_value_ptr_double;

    model->get_value_at_indices_int     = bmi_get_value_at_indices_int;
    model->get_value_at_indices_float   = bmi_get_value_at_indices_float;
    model->get_value_at_indices_double  = bmi_get_value_at_indices_double;

    model->set_value_int                = bmi_set_value_int;
    model->set_value_float              = bmi_set_value_float;
    model->set_value_double             = bmi_set_value_double;

    model->set_value_at_indices_int     = bmi_set_value_at_indices_int;
    model->set_value_at_indices_float   = bmi_set_value_at_indices_float;
    model->set_value_at_indices_double  = bmi_set_value_at_indices_double;

    return model;
}
