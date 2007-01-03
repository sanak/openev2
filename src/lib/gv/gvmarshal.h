
#ifndef __gvmarshal_MARSHAL_H__
#define __gvmarshal_MARSHAL_H__

#include	<glib-object.h>

G_BEGIN_DECLS

/* VOID:POINTER,POINTER (marshal_list.txt:1) */
extern void gvmarshal_VOID__POINTER_POINTER (GClosure     *closure,
                                             GValue       *return_value,
                                             guint         n_param_values,
                                             const GValue *param_values,
                                             gpointer      invocation_hint,
                                             gpointer      marshal_data);

G_END_DECLS

#endif /* __gvmarshal_MARSHAL_H__ */

