#include <geom_model/curves.h>
#include <geom_model/oriented_edge.h>
#include <geom_model/surfaces.h>

#include <util/debug.h>
#include <util/to_string.h>

#include "step_parser.h"
#include "step_reader.h"

#include <fstream>
#include <iostream>

using namespace std;

StepParser& StepParser::parse()
{
    auto shell_list = get_shells();
    auto size = shell_list.size();

    geom_.resize(size);
    for (size_t i = 0; i < size; ++i) {
        cdbg << "shell: " << i + 1 << "/" << size << ": " << endl;

        auto face_list = get_faces(shell_list[i].first);
        auto fsize = face_list.size();
        vector<Face> faces;

        geom_[i].set_ax(shell_list[i].second);
        for (size_t j = 0; j < fsize; ++j) {
            cdbg << ">  " << j + 1 << "/" << fsize << endl;

            faces.emplace_back(get_face(face_list[j]));
        }
        geom_[i].set_faces(move(faces));
    }

    return *this;
}

vector<pair<size_t, Axis>> StepParser::get_shells()
{
    vector<pair<size_t, Axis>> result;
    for (auto& i : data_) {
        StepTokenizer tok(i.second);
        if (tok.next().get() == step_root) {
            // ADVANCED_BREP_SHAPE_REPRESENTATION
            auto [ref] = step_read<br_<i_<str_>, rlist_, i_<ref_>>>(tok, i.first);
            auto axis = get_axis(ref.back());

            for (auto it = cbegin(ref); it != prev(cend(ref)); ++it) {
                // MANIFOLD_SOLID_BREP
                auto [shell_id] = step_read<i_<str_>, br_<i_<str_>, ref_>>(at(*it), *it);
                result.emplace_back(shell_id, axis);
            }
        }
    }
    return result;
}

StepParser::id_list_t StepParser::get_faces(size_t id)
{
    auto [result] = step_read<i_<str_>, br_<i_<str_>, list_<ref_>>>(at(id), id);
    return result;
}

Face StepParser::get_face(size_t id)
{
    FaceBound outer;
    vector<FaceBound> inner;
    unique_ptr<AbstractSurface> surf;

    auto [bounds, surf_id, same_sense] = step_read<i_<str_>, br_<i_<str_>, rlist_, ref_, bool_>>(at(id), id);

    for (auto bound_id : bounds) {
        auto res = get_bound(bound_id);
        if (res.second) {
            CHECK_IF(!outer.empty(), err::unexpected_symbol, "Non unique outer bound");
            outer = res.first;
        } else
            inner.emplace_back(res.first);
    }
    CHECK_IF(outer.empty(), err::unexpected_symbol, "Expected one outer bound");

    return Face(get_surface(surf_id), same_sense, outer, inner);
}

pair<FaceBound, bool> StepParser::get_bound(size_t id)
{
    FaceBound result;

    auto [entity, loop_id] = step_read<str_, br_<i_<str_>, ref_, i_<bool_>>>(at(id), id);
    auto [loop] = step_read<i_<str_>, br_<i_<str_>, rlist_>>(at(loop_id), loop_id);

    for (auto oedge_id : loop)
        result.emplace_back(get_oedge(oedge_id));

    return make_pair(result, bool(entity == "FACE_OUTER_BOUND"));
}

OrientedEdge StepParser::get_oedge(size_t id)
{
    auto [edge_id, orientation] = step_read<i_<str_>, br_<i_<str_>, i_<str_>, i_<str_>, ref_, bool_>>(at(id), id);
    return {get_edge(edge_id), orientation};
}

Edge StepParser::get_edge(size_t id)
{
    Edge result;
    auto it = edge_.find(id);

    if (it != cend(edge_)) {
        result = it->second;
    } else {
        auto [start_id, end_id, curve_id] = step_read<i_<str_>, br_<i_<str_>, ref_, ref_, ref_, i_<bool_>>>(at(id), id);

        auto vbeg = get_vertex(start_id), vend = get_vertex(end_id);
        auto c = get_curve(curve_id);
        if (is_curve_bspline(curve_id)) {
            auto& bspline = dynamic_cast<BSplineCurve&>(*c);
            result = Edge(c, bspline.param_front(), bspline.param_back());
        } else {
            result = Edge(c, vbeg, vend);
        }
        edge_[id] = result;
    }

    return result;
}

Point StepParser::get_vertex(size_t id) const
{
    auto [point_id] = step_read<i_<str_>, br_<i_<str_>, ref_>>(at(id), id);
    return get_point(point_id);
}

Vec StepParser::get_dir(size_t id) const
{
    auto [result] = step_read<i_<str_>, br_<i_<str_>, vec_>>(at(id), id);
    return result;
}

Point StepParser::get_point(size_t id) const { return Point(get_dir(id)); }

Vec StepParser::get_vec(size_t id) const
{
    auto [direction_id, magnitude] = step_read<i_<str_>, br_<i_<str_>, ref_, float_>>(at(id), id);
    return magnitude * get_dir(direction_id).normalize();
}

Axis StepParser::get_axis(size_t id) const
{
    auto [center_id, z_id, ref_id] = step_read<i_<str_>, br_<i_<str_>, ref_, ref_, ref_>>(at(id), id);

    auto z = get_dir(z_id), ref = get_dir(ref_id);
    auto center = get_point(center_id);
    return Axis::from_zx(z, ref, center);
}

shared_ptr<AbstractCurve> StepParser::get_curve(size_t id) const
{
    shared_ptr<AbstractCurve> result = nullptr;

    auto pos = curve_.find(id);
    if (pos != cend(curve_)) {
        result = pos->second;
    } else {
        StepTokenizer tok(at(id));
        if (auto curve_id = find_curve(tok.next().get()); curve_id.has_value()) {
            switch (*curve_id) {
            case StepCurve::LINE: {
                auto [c_id, dir_id] = step_read<br_<i_<str_>, ref_, ref_>>(tok, id);
                result = make_shared<Line>(get_vec(dir_id), get_point(c_id));
                break;
            }
            case StepCurve::CIRCLE: {
                auto [axis_id, rad] = step_read<br_<i_<str_>, ref_, float_>>(tok, id);
                result = make_shared<Circle>(rad, get_axis(axis_id));
                break;
            }
            case StepCurve::ELLIPSE: {
                auto [axis_id, rx, ry] = step_read<br_<i_<str_>, ref_, float_, float_>>(tok, id);
                result = make_shared<Ellipse>(rx, ry, get_axis(axis_id));
                break;
            }
            case StepCurve::PARABOLA: {
                auto [axis_id, f] = step_read<br_<i_<str_>, ref_, float_>>(tok, id);
                result = make_shared<Parabola>(f, get_axis(axis_id));
                break;
            }
            case StepCurve::HYPERBOLA: {
                auto [axis_id, rx, ry] = step_read<br_<i_<str_>, ref_, float_, float_>>(tok, id);
                result = make_shared<Hyperbola>(rx, ry, get_axis(axis_id));
                break;
            }
            case StepCurve::B_SPLINE_CURVE_WITH_KNOTS: {
                vector<Point> cp;

                auto [deg, cp_ref, k_mult, k_val] = step_read<
                    br_<i_<str_>, int_, rlist_, i_<str_, bool_, bool_>, list_<int_>, list_<float_>, i_<str_>>>(tok, id);

                for (auto r : cp_ref)
                    cp.emplace_back(get_point(r));
                result = make_shared<BSplineCurve>(deg, k_mult, k_val, cp);
                break;
            }
            case StepCurve::RATIONAL_B_SPLINE_CURVE: {
                vector<Point> cp;

                auto [deg, cp_ref, km, kv, w]
                    = step_read<i_<str_, str_, str_, str_>, br_<int_, rlist_, i_<str_, bool_, bool_>>, i_<str_>,
                                br_<list_<int_>, list_<float_>, i_<str_>>, i_<str_, str_, str_, str_, str_, str_, str_>,
                                br_<list_<float_>>>(tok, id);

                for (auto r : cp_ref)
                    cp.emplace_back(get_point(r));
                result = make_shared<BSplineCurve>(deg, km, kv, cp, w);
                break;
            }
            }
            curve_[id] = result;
        }
    }
    CHECK_IF(!result, err::null_pointer, "returning null curve");
    return result;
}

bool StepParser::is_curve_bspline(size_t curve_id) const
{
    StepTokenizer tok(at(curve_id));
    auto val = find_curve(tok.next().get());
    return val.has_value()
        && (*val == StepCurve::RATIONAL_B_SPLINE_CURVE || *val == StepCurve::B_SPLINE_CURVE_WITH_KNOTS);
}

shared_ptr<AbstractSurface> StepParser::get_surface(size_t id) const
{
    shared_ptr<AbstractSurface> result = nullptr;

    auto pos = surface_.find(id);
    if (pos != cend(surface_)) {
        result = pos->second;
    } else {
        StepTokenizer tok(at(id));
        if (auto surf_id = find_surface(tok.next().get()); surf_id.has_value()) {
            switch (*surf_id) {
            case StepSurface::PLANE: {
                auto [axis_id] = step_read<br_<i_<str_>, ref_>>(tok, id);
                result = make_shared<Plane>(get_axis(axis_id));
                break;
            }
            case StepSurface::CYLINDRICAL_SURFACE: {
                auto [axis_id, r] = step_read<br_<i_<str_>, ref_, float_>>(tok, id);
                result = make_shared<CylindricalSurface>(r, get_axis(axis_id));
                break;
            }
            case StepSurface::CONICAL_SURFACE: {
                auto [axis_id, r, a] = step_read<br_<i_<str_>, ref_, float_, float_>>(tok, id);
                result = make_shared<ConicalSurface>(r, a, get_axis(axis_id));
                break;
            }
            case StepSurface::SPHERICAL_SURFACE: {
                auto [axis_id, r] = step_read<br_<i_<str_>, ref_, float_>>(tok, id);
                result = make_shared<SphericalSurface>(r, get_axis(axis_id));
                break;
            }
            case StepSurface::TOROIDAL_SURFACE: {
                auto [axis_id, r1, r0] = step_read<br_<i_<str_>, ref_, float_, float_>>(tok, id);
                result = make_shared<ToroidalSurface>(r0, r1, get_axis(axis_id));
                break;
            }
            case StepSurface::B_SPLINE_SURFACE_WITH_KNOTS: {
                vector<vector<Point>> cp;
                auto [du, dv, cp_ref, ku_mult, kv_mult, ku_val, kv_val]
                    = step_read<br_<i_<str_>, int_, int_, mat_<ref_>, i_<str_, bool_, bool_, bool_>, list_<int_>,
                                    list_<int_>, list_<float_>, list_<float_>, i_<str_>>>(tok, id);
                for (auto& row : cp_ref) {
                    vector<Point> v;
                    for (auto& i : row)
                        v.emplace_back(get_point(i));
                    cp.emplace_back(v);
                }
                result = make_shared<BSplineSurface>(du, dv, ku_mult, ku_val, kv_mult, kv_val, cp);
                break;
            }
            case StepSurface::RATIONAL_B_SPLINE_SURFACE: {
                vector<vector<Point>> cp;

                auto [du, dv, cp_ref, kum, kvm, kuv, kvv, w]
                    = step_read<i_<str_, str_, str_, str_>, br_<int_, int_, mat_<ref_>, i_<str_, bool_, bool_, bool_>>,
                                i_<str_>, br_<list_<int_>, list_<int_>, list_<float_>, list_<float_>, i_<str_>>,
                                i_<str_, str_, str_, str_>, br_<mat_<float_>>,
                                i_<str_, str_, str_, str_, str_, str_, str_, str_>>(tok, id);

                for (auto& row : cp_ref) {
                    vector<Point> v;
                    for (auto& i : row)
                        v.emplace_back(get_point(i));
                    cp.emplace_back(v);
                }

                result = make_shared<BSplineSurface>(du, dv, kum, kuv, kvm, kvv, cp, w);
            }
            }
            surface_[id] = result;
        }
    }
    CHECK_IF(!result, err::null_pointer, "returning null surface");
    return result;
}

const string& StepParser::at(size_t id) const
{
    CHECK_IF(data_.find(id) == cend(data_), err::id_not_loaded, "id (" + to_string(id) + ") is not loaded");
    return data_.at(id);
}

StepParser::StepParser(const StepLoader& data)
    : data_(data.data())
    , geom_()
    , edge_()
    , curve_()
    , surface_()
{
}

BoundaryRep StepParser::geom() const { return geom_; }

BoundaryRep step_parse(istream& is)
{
    StepLoader load(is);
    StepParser parse(load);
    return parse.parse().geom();
}

BoundaryRep step_parse(const string& str)
{
    fstream is(str, ios_base::in);
    return step_parse(is);
}
