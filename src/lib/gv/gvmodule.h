/*
 * gvmodule.h - Define prototypes for wrapped functions that are missed by
 * the autowrapper.  This can happen if the function is not really a
 * function, but a macro.
 *
 * These functions are just used by the autowrapper; for each there should
 * be an override/_wrap block defined in gv.override.
 */

#ifndef __GVMODULE_H__
#define __GVMODULE_H__

G_BEGIN_DECLS

void copy_obj(GObject *obj, GObject *sub);

PyObject *gv_data_set_properties(PyGObject *self, PyObject *args);
PyObject *gv_view_area_get_width(PyObject *self, PyObject *args);
PyObject *gv_view_area_get_height(PyObject *self, PyObject *args);
PyObject *gv_shapes_num_shapes(PyObject *self, PyObject *args);
PyObject *gv_shapes_get_shape(PyObject *self, PyObject *args);
PyObject *gv_areas_num_areas(PyObject *self, PyObject *args);
PyObject *gv_areas_get_area(PyObject *self, PyObject *args);
PyObject *gv_points_num_points(PyObject *self, PyObject *args);
PyObject *gv_points_get_point(PyObject *self, PyObject *args);
PyObject *gv_polylines_num_lines(PyObject *self, PyObject *args);
PyObject *gv_polylines_get_line(PyObject *self, PyObject *args);
PyObject *gv_raster_get_min(PyObject *self, PyObject *args);
PyObject *gv_raster_get_max(PyObject *self, PyObject *args);
PyObject *gv_view_area_get_translation(PyObject *self);
PyObject *gv_have_ogr_support(PyObject *self);
PyObject *gv_shapes_get_change_info(PyObject *self, PyObject *args);
PyObject *gv_shapes_lines_for_vecplot(PyObject *self, PyObject *args);
PyObject *gv_raster_get_change_info(PyObject *self, PyObject *args);
PyObject *gv_raster_force_load(PyGObject *self, PyObject *args);
PyObject *gv_raster_get_gdal_band(PyGObject *self, PyObject *args);
PyObject *gv_raster_layer_get_height(PyGObject *self, PyObject *args);
PyObject *gv_raster_layer_get_mesh_lod(PyGObject *self, PyObject *args);
PyObject *gv_raster_layer_get_nodata(PyGObject *self, PyObject *args);
PyObject *gv_raster_layer_get_source_lut(PyGObject *self, PyObject *args);
PyObject *gv_raster_layer_get_source_const_value(PyGObject *self, PyObject *args);
PyObject *gv_view_area_get_eye_dir(PyGObject *self, PyObject *args);
PyObject *gv_view_area_get_eye_pos(PyGObject *self, PyObject *args);
PyObject *gv_rgba_to_rgb(PyObject *self, PyObject *args);

PyObject *gv_shape_create(PyObject *self, PyObject *args);
PyObject *gv_shape_from_xml(PyObject *self, PyObject *args);
PyObject *gv_shape_to_xml(PyObject *self, PyObject *args);
PyObject *gv_shape_destroy(PyObject *self, PyObject *args);
PyObject *gv_shape_ref(PyObject *self, PyObject *args);
PyObject *gv_shape_unref(PyObject *self, PyObject *args);
PyObject *gv_shape_get_ref(PyObject *self, PyObject *args);
PyObject *gv_shape_copy(PyObject *self, PyObject *args);
PyObject *gv_shape_get_shape_type(PyObject *self, PyObject *args);
PyObject *gv_shape_line_from_nodelists(PyObject *self, PyObject *args);
PyObject *gv_shape_get_property(PyObject *self, PyObject *args);
PyObject *gv_shape_get_properties(PyObject *self, PyObject *args);
PyObject *gv_shape_get_typed_properties(PyObject *self, PyObject *args);
PyObject *gv_shape_set_property(PyObject *self, PyObject *args);
PyObject *gv_shape_set_properties(PyObject *self, PyObject *args);
PyObject *gv_shape_get_rings(PyObject *self, PyObject *args);
PyObject *gv_shape_get_nodes(PyObject *self, PyObject *args);
PyObject *gv_shape_add_node(PyObject *self, PyObject *args);
PyObject *gv_shape_set_node(PyObject *self, PyObject *args);
PyObject *gv_shape_get_node(PyObject *self, PyObject *args);
PyObject *gv_shape_point_in_polygon(PyObject *self, PyObject *args);
PyObject *gv_shape_distance_from_polygon(PyObject *self, PyObject *args);
PyObject *gv_shape_clip_to_rect(PyObject *self, PyObject *args);
PyObject *gv_shape_add_shape(PyObject *self, PyObject *args);
PyObject *gv_shape_get_shape(PyObject *self, PyObject *args);
PyObject *gv_shape_collection_get_count(PyObject *self, PyObject *args);

G_END_DECLS

#endif /* __GVMODULE_H__ */

