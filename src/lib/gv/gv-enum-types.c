
/* Generated data (by glib-mkenums) */

#include "gv-enum-types.h"

/* enumerations from "gvraster.h" */
#include "gvraster.h"
GType
gv_sample_method_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GEnumValue values[] = {
      { GvSMAverage, "GvSMAverage", "average" },
      { GvSMSample, "GvSMSample", "sample" },
      { GvSMAverage8bitPhase, "GvSMAverage8bitPhase", "average8bitphase" },
      { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("GvSampleMethod", values);
  }
  return etype;
}
GType
gv_auto_scale_alg_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GEnumValue values[] = {
      { GvASAAutomatic, "GvASAAutomatic", "automatic" },
      { GvASAPercentTailTrim, "GvASAPercentTailTrim", "percenttailtrim" },
      { GvASAStdDeviation, "GvASAStdDeviation", "stddeviation" },
      { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("GvAutoScaleAlg", values);
  }
  return etype;
}

/* enumerations from "gvrasterlayer.h" */
#include "gvrasterlayer.h"
GType
gv_raster_layer_mode_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GEnumValue values[] = {
      { GV_RLM_AUTO, "GV_RLM_AUTO", "auto" },
      { GV_RLM_SINGLE, "GV_RLM_SINGLE", "single" },
      { GV_RLM_RGBA, "GV_RLM_RGBA", "rgba" },
      { GV_RLM_COMPLEX, "GV_RLM_COMPLEX", "complex" },
      { GV_RLM_PSCI, "GV_RLM_PSCI", "psci" },
      { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("GvRasterLayerMode", values);
  }
  return etype;
}

/* enumerations from "gvrasterlut.h" */
#include "gvrasterlut.h"
GType
gv_raster_lut_enhancement_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GEnumValue values[] = {
      { GV_RASTER_LUT_ENHANCE_UNKNOWN, "GV_RASTER_LUT_ENHANCE_UNKNOWN", "unknown" },
      { GV_RASTER_LUT_ENHANCE_LINEAR, "GV_RASTER_LUT_ENHANCE_LINEAR", "linear" },
      { GV_RASTER_LUT_ENHANCE_LOG, "GV_RASTER_LUT_ENHANCE_LOG", "log" },
      { GV_RASTER_LUT_ENHANCE_ROOT, "GV_RASTER_LUT_ENHANCE_ROOT", "root" },
      { GV_RASTER_LUT_ENHANCE_SQUARE, "GV_RASTER_LUT_ENHANCE_SQUARE", "square" },
      { GV_RASTER_LUT_ENHANCE_EQUALIZE, "GV_RASTER_LUT_ENHANCE_EQUALIZE", "equalize" },
      { GV_RASTER_LUT_ENHANCE_GAMMA, "GV_RASTER_LUT_ENHANCE_GAMMA", "gamma" },
      { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("GvRasterLutEnhancement", values);
  }
  return etype;
}

/* enumerations from "gvshapeslayer.h" */
#include "gvshapeslayer.h"
GType
gv_draw_mode_gv_draw_mode_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GEnumValue values[] = {
      { NORMAL, "NORMAL", "normal" },
      { SELECTED, "SELECTED", "selected" },
      { PICKING, "PICKING", "picking" },
      { NORMAL_GET_BOX, "NORMAL_GET_BOX", "normal-get-box" },
      { PQUERY_POINTS, "PQUERY_POINTS", "pquery-points" },
      { PQUERY_LABELS, "PQUERY_LABELS", "pquery-labels" },
      { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("gv_draw_mode", values);
  }
  return etype;
}

/* enumerations from "gvviewlink.h" */
#include "gvviewlink.h"
GType
gv_link_cursor_mode_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GEnumValue values[] = {
      { GV_LINK_CURSOR_OFF, "GV_LINK_CURSOR_OFF", "off" },
      { GV_LINK_CURSOR_ON_DEFAULT, "GV_LINK_CURSOR_ON_DEFAULT", "on-default" },
      { GV_LINK_CURSOR_ON_LOGICAL, "GV_LINK_CURSOR_ON_LOGICAL", "on-logical" },
      { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("GvLinkCursorMode", values);
  }
  return etype;
}

/* Generated data ends here */

