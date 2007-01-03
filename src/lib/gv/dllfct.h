#ifdef BUILD_DLL

// the dll exports
#define EXPORT __declspec(dllexport)
#else
// the exe imports
#define EXPORT __declspec(dllimport)
#endif

// functions to be imported/exported
EXPORT void gluDeleteTess (GLUtesselator* tess);
EXPORT void gluEndPolygon (GLUtesselator* tess);
EXPORT void gluGetTessProperty (GLUtesselator* tess, GLenum which, GLdouble* data);
EXPORT GLUtesselator* gluNewTess (void);
EXPORT void gluNextContour (GLUtesselator* tess, GLenum type);
EXPORT void gluTessBeginContour (GLUtesselator* tess);
EXPORT void gluTessBeginPolygon (GLUtesselator* tess, GLvoid* data);
EXPORT void gluTessCallback (GLUtesselator* tess, GLenum which, _GLUfuncptr CallBackFunc);
EXPORT void gluTessEndContour (GLUtesselator* tess);
EXPORT void gluTessEndPolygon (GLUtesselator* tess);
EXPORT void gluTessNormal (GLUtesselator* tess, GLdouble valueX, GLdouble valueY, GLdouble valueZ);
EXPORT void gluTessProperty (GLUtesselator* tess, GLenum which, GLdouble data);
EXPORT void gluTessVertex (GLUtesselator* tess, GLdouble *location, GLvoid* data);
