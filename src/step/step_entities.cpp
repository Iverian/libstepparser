#include "step_entities.h"

#include <map>
#include <set>

using namespace std;

optional<StepCurve> find_curve(const string& str)
{
    static const map<string, StepCurve> curves{
        {"LINE", StepCurve::LINE},
        {"CIRCLE", StepCurve::CIRCLE},
        {"ELLIPSE", StepCurve::ELLIPSE},
        {"HYPERBOLA", StepCurve::HYPERBOLA},
        {"PARABOLA", StepCurve::PARABOLA},
        {"B_SPLINE_CURVE_WITH_KNOTS", StepCurve::B_SPLINE_CURVE_WITH_KNOTS},
        {"(", StepCurve::RATIONAL_B_SPLINE_CURVE}};
    auto it = curves.find(str);
    return it != curves.cend() ? optional<StepCurve>{it->second} : nullopt;
}

optional<StepSurface> find_surface(const string& str)
{
    static const map<string, StepSurface> surfaces{
        {"PLANE", StepSurface::PLANE},
        {"CYLINDRICAL_SURFACE", StepSurface::CYLINDRICAL_SURFACE},
        {"CONICAL_SURFACE", StepSurface::CONICAL_SURFACE},
        {"SPHERICAL_SURFACE", StepSurface::SPHERICAL_SURFACE},
        {"TOROIDAL_SURFACE", StepSurface::TOROIDAL_SURFACE},
        {"B_SPLINE_SURFACE_WITH_KNOTS",
         StepSurface::B_SPLINE_SURFACE_WITH_KNOTS},
        {"(", StepSurface::RATIONAL_B_SPLINE_SURFACE}};
    auto it = surfaces.find(str);
    return it != surfaces.cend() ? optional<StepSurface>{it->second} : nullopt;
}

bool is_whitelisted(const string& str)
{
    static const set<string> whitelist = {
        // root
        step_root, "MANIFOLD_SOLID_BREP",
        // geometry
        "CARTESIAN_POINT", "VECTOR", "DIRECTION", "AXIS2_PLACEMENT_3D",
        "AXIS2_PLACEMENT_2D",
        // curves
        "LINE", "CIRCLE", "ELLIPSE", "HYPERBOLA", "PARABOLA",
        "CIRCULAR_INVOLUTE", "B_SPLINE_CURVE_WITH_KNOTS",
        // surfaces
        "PLANE", "CYLINDRICAL_SURFACE", "CONICAL_SURFACE", "SPHERICAL_SURFACE",
        "TOROIDAL_SURFACE", "B_SPLINE_SURFACE_WITH_KNOTS",
        // topology
        "VERTEX_POINT", "EDGE_CURVE", "ORIENTED_EDGE", "EDGE_LOOP",
        "OPEN_PATH", "ORIENTED_PATH", "FACE_BOUND", "FACE_OUTER_BOUND",
        "FACE_SURFACE", "ADVANCED_FACE", "CLOSED_SHELL",
        "ORIENTED_CLOSED_SHELL", "OPEN_SHELL", "ORIENTED_OPEN_SHELL",
        // stuff
        "("};
    return whitelist.find(str) != whitelist.cend();
}
