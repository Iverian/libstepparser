#ifndef HEXMESH_SRC_STEP_STEP_ENTITIES_H_
#define HEXMESH_SRC_STEP_STEP_ENTITIES_H_

#include <map>
#include <optional>
#include <set>
#include <string>

static const auto step_root = "ADVANCED_BREP_SHAPE_REPRESENTATION";

enum class StepCurve {
    LINE,
    CIRCLE,
    ELLIPSE,
    HYPERBOLA,
    PARABOLA,
    CIRCULAR_INVOLUTE,
    B_SPLINE_CURVE_WITH_KNOTS,
    RATIONAL_B_SPLINE_CURVE
};

enum class StepSurface {
    PLANE,
    CYLINDRICAL_SURFACE,
    CONICAL_SURFACE,
    SPHERICAL_SURFACE,
    TOROIDAL_SURFACE,
    B_SPLINE_SURFACE_WITH_KNOTS,
    RATIONAL_B_SPLINE_SURFACE
};

std::optional<StepCurve> find_curve(const std::string& str);
std::optional<StepSurface> find_surface(const std::string& str);
bool is_whitelisted(const std::string& str);

#endif // HEXMESH_SRC_STEP_STEP_ENTITIES_H_
