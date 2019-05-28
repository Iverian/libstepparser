#include <gm/curves.hpp>
#include <gm/oriented_edge.hpp>
#include <gm/surfaces.hpp>
#include <util/debug.hpp>
#include <util/to_string.hpp>

#include "spdlog\common.h"
#include "step_parser.hpp"
#include "step_reader.hpp"

#include <cmms/logging.hpp>
#include <spdlog/spdlog.h>

#include <fstream>
#include <iostream>

using namespace std;

StepParser& StepParser::parse()
{
    auto shell_list = get_shells();
    auto size = shell_list.size();

    geom_.resize(size);
    for (size_t i = 0; i < size; ++i) {
        auto face_list = get_faces(shell_list[i].first);
        auto fsize = face_list.size();
        vector<gm::Face> faces;

        log_->debug("parsing {} / {} shell with {} faces", i + 1, size, fsize);
        geom_[i].set_ax(shell_list[i].second);
        for (size_t j = 0; j < fsize; ++j) {
            log_->debug("parsing {} / {} face", j + 1, fsize);

            faces.emplace_back(get_face(face_list[j]));
        }
        geom_[i].set_faces(move(faces));
    }

    return *this;
}

vector<pair<size_t, gm::Axis>> StepParser::get_shells()
{
    vector<pair<size_t, gm::Axis>> result;
    for (auto& i : data_) {
        StepTokenizer tok(i.second);
        if (tok.next().get() == step_root) {
            // ADVANCED_BREP_SHAPE_REPRESENTATION
            auto [ref]
                = step_read<br_<i_<str_>, rlist_, i_<ref_>>>(tok, i.first);
            auto axis = get_axis(ref.back());

            for (auto it = cbegin(ref); it != prev(cend(ref)); ++it) {
                // MANIFOLD_SOLID_BREP
                auto [shell_id]
                    = step_read<i_<str_>, br_<i_<str_>, ref_>>(at(*it), *it);
                result.emplace_back(shell_id, axis);
            }
        }
    }
    return result;
}

StepParser::id_list_t StepParser::get_faces(size_t id)
{
    auto [result]
        = step_read<i_<str_>, br_<i_<str_>, list_<ref_>>>(at(id), id);
    return result;
}

gm::Face StepParser::get_face(size_t id)
{
    gm::FaceBound outer;
    vector<gm::FaceBound> inner;
    unique_ptr<gm::AbstractSurface> surf;

    auto [bounds, surf_id, same_sense]
        = step_read<i_<str_>, br_<i_<str_>, rlist_, ref_, bool_>>(at(id), id);

    for (auto bound_id : bounds) {
        auto res = get_bound(bound_id);
        if (res.second) {
            CHECK_IF(!outer.empty(), err::unexpected_symbol,
                     "Non unique outer bound");
            outer = res.first;
        } else
            inner.emplace_back(res.first);
    }
    CHECK_IF(outer.empty(), err::unexpected_symbol,
             "Expected one outer bound");

    return gm::Face(get_surface(surf_id), same_sense, outer, inner);
}

pair<gm::FaceBound, bool> StepParser::get_bound(size_t id)
{
    gm::FaceBound result;

    auto [entity, loop_id]
        = step_read<str_, br_<i_<str_>, ref_, i_<bool_>>>(at(id), id);
    auto [loop]
        = step_read<i_<str_>, br_<i_<str_>, rlist_>>(at(loop_id), loop_id);

    for (auto oedge_id : loop)
        result.emplace_back(get_oedge(oedge_id));

    return make_pair(result, bool(entity == "FACE_OUTER_BOUND"));
}

gm::OrientedEdge StepParser::get_oedge(size_t id)
{
    auto [edge_id, orientation]
        = step_read<i_<str_>, br_<i_<str_>, i_<str_>, i_<str_>, ref_, bool_>>(
            at(id), id);
    return {get_edge(edge_id), orientation};
}

gm::Edge StepParser::get_edge(size_t id)
{
    auto it = edge_.find(id);

    if (it != cend(edge_)) {
        return it->second;
    } else {
        auto [start_id, end_id, curve_id]
            = step_read<i_<str_>, br_<i_<str_>, ref_, ref_, ref_, i_<bool_>>>(
                at(id), id);

        auto vbeg = get_vertex(start_id), vend = get_vertex(end_id);
        auto c = get_curve(curve_id);
        if (is_curve_bspline(curve_id)) {
            auto& bspline = dynamic_cast<gm::BSplineCurve&>(*c);
            auto result = gm::Edge(c, bspline.pfront(), bspline.pback());
            edge_[id] = result;

            return result;
        } else {
            auto result = gm::Edge(c, vbeg, vend);
            edge_[id] = result;

            return result;
        }
    }
}

gm::Point StepParser::get_vertex(size_t id) const
{
    auto [point_id] = step_read<i_<str_>, br_<i_<str_>, ref_>>(at(id), id);
    return get_point(point_id);
}

gm::Vec StepParser::get_dir(size_t id) const
{
    auto [result] = step_read<i_<str_>, br_<i_<str_>, vec_>>(at(id), id);
    return result;
}

gm::Point StepParser::get_point(size_t id) const
{
    return gm::Point(get_dir(id));
}

gm::Vec StepParser::get_vec(size_t id) const
{
    auto [direction_id, magnitude]
        = step_read<i_<str_>, br_<i_<str_>, ref_, float_>>(at(id), id);
    return magnitude * unit(get_dir(direction_id));
}

gm::Axis StepParser::get_axis(size_t id) const
{
    auto [center_id, z_id, ref_id]
        = step_read<i_<str_>, br_<i_<str_>, ref_, ref_, ref_>>(at(id), id);

    auto z = get_dir(z_id), ref = get_dir(ref_id);
    auto center = get_point(center_id);
    return gm::Axis::from_zx(z, ref, center);
}

shared_ptr<gm::AbstractCurve> StepParser::get_curve(size_t id) const
{
    shared_ptr<gm::AbstractCurve> result = nullptr;

    auto pos = curve_.find(id);
    if (pos != cend(curve_)) {
        result = pos->second;
    } else {
        StepTokenizer tok(at(id));
        if (auto curve_id = find_curve(tok.next().get());
            curve_id.has_value()) {
            switch (*curve_id) {
            case StepCurve::LINE: {
                auto [c_id, dir_id]
                    = step_read<br_<i_<str_>, ref_, ref_>>(tok, id);
                result
                    = make_shared<gm::Line>(get_vec(dir_id), get_point(c_id));
                break;
            }
            case StepCurve::CIRCLE: {
                auto [axis_id, rad]
                    = step_read<br_<i_<str_>, ref_, float_>>(tok, id);
                result = make_shared<gm::Circle>(rad, get_axis(axis_id));
                break;
            }
            case StepCurve::ELLIPSE: {
                auto [axis_id, rx, ry]
                    = step_read<br_<i_<str_>, ref_, float_, float_>>(tok, id);
                result = make_shared<gm::Ellipse>(rx, ry, get_axis(axis_id));
                break;
            }
            case StepCurve::PARABOLA: {
                auto [axis_id, f]
                    = step_read<br_<i_<str_>, ref_, float_>>(tok, id);
                result = make_shared<gm::Parabola>(f, get_axis(axis_id));
                break;
            }
            case StepCurve::HYPERBOLA: {
                auto [axis_id, rx, ry]
                    = step_read<br_<i_<str_>, ref_, float_, float_>>(tok, id);
                result = make_shared<gm::Hyperbola>(rx, ry, get_axis(axis_id));
                break;
            }
            case StepCurve::B_SPLINE_CURVE_WITH_KNOTS: {
                vector<gm::Point> cp;

                auto [deg, cp_ref, k_mult, k_val] = step_read<
                    br_<i_<str_>, int_, rlist_, i_<str_, bool_, bool_>,
                        list_<int_>, list_<float_>, i_<str_>>>(tok, id);

                for (auto r : cp_ref)
                    cp.emplace_back(get_point(r));
                result = make_shared<gm::BSplineCurve>(deg, k_mult, k_val, cp);
                break;
            }
            case StepCurve::RATIONAL_B_SPLINE_CURVE: {
                vector<gm::Point> cp;

                auto [deg, cp_ref, km, kv, w]
                    = step_read<i_<str_, str_, str_, str_>,
                                br_<int_, rlist_, i_<str_, bool_, bool_>>,
                                i_<str_>,
                                br_<list_<int_>, list_<float_>, i_<str_>>,
                                i_<str_, str_, str_, str_, str_, str_, str_>,
                                br_<list_<float_>>>(tok, id);

                for (auto r : cp_ref)
                    cp.emplace_back(get_point(r));
                result = make_shared<gm::BSplineCurve>(deg, km, kv, cp, w);
                break;
            }
            }
            curve_[id] = result;
        }
    }
    CHECK_IF(!result, err::null_pointer, "returning null curve");

    log_->debug("processed curve: {}", *result);
    return result;
}

bool StepParser::is_curve_bspline(size_t curve_id) const
{
    StepTokenizer tok(at(curve_id));
    auto val = find_curve(tok.next().get());
    return val.has_value()
        && (*val == StepCurve::RATIONAL_B_SPLINE_CURVE
            || *val == StepCurve::B_SPLINE_CURVE_WITH_KNOTS);
}

shared_ptr<gm::AbstractSurface> StepParser::get_surface(size_t id) const
{
    shared_ptr<gm::AbstractSurface> result = nullptr;

    auto pos = surface_.find(id);
    if (pos != cend(surface_)) {
        result = pos->second;
    } else {
        StepTokenizer tok(at(id));
        if (auto surf_id = find_surface(tok.next().get());
            surf_id.has_value()) {
            switch (*surf_id) {
            case StepSurface::PLANE: {
                auto [axis_id] = step_read<br_<i_<str_>, ref_>>(tok, id);
                result = make_shared<gm::Plane>(get_axis(axis_id));
                break;
            }
            case StepSurface::CYLINDRICAL_SURFACE: {
                auto [axis_id, r]
                    = step_read<br_<i_<str_>, ref_, float_>>(tok, id);
                result = make_shared<gm::CylindricalSurface>(
                    r, get_axis(axis_id));
                break;
            }
            case StepSurface::CONICAL_SURFACE: {
                auto [axis_id, r, a]
                    = step_read<br_<i_<str_>, ref_, float_, float_>>(tok, id);
                result
                    = make_shared<gm::ConicalSurface>(r, a, get_axis(axis_id));
                break;
            }
            case StepSurface::SPHERICAL_SURFACE: {
                auto [axis_id, r]
                    = step_read<br_<i_<str_>, ref_, float_>>(tok, id);
                result
                    = make_shared<gm::SphericalSurface>(r, get_axis(axis_id));
                break;
            }
            case StepSurface::TOROIDAL_SURFACE: {
                auto [axis_id, r1, r0]
                    = step_read<br_<i_<str_>, ref_, float_, float_>>(tok, id);
                result = make_shared<gm::ToroidalSurface>(r0, r1,
                                                          get_axis(axis_id));
                break;
            }
            case StepSurface::B_SPLINE_SURFACE_WITH_KNOTS: {
                vector<vector<gm::Point>> cp;
                auto [du, dv, cp_ref, ku_mult, kv_mult, ku_val, kv_val]
                    = step_read<br_<i_<str_>, int_, int_, mat_<ref_>,
                                    i_<str_, bool_, bool_, bool_>, list_<int_>,
                                    list_<int_>, list_<float_>, list_<float_>,
                                    i_<str_>>>(tok, id);
                for (auto& row : cp_ref) {
                    vector<gm::Point> v;
                    for (auto& i : row)
                        v.emplace_back(get_point(i));
                    cp.emplace_back(v);
                }
                result = make_shared<gm::BSplineSurface>(
                    du, dv, ku_mult, ku_val, kv_mult, kv_val, cp);
                break;
            }
            case StepSurface::RATIONAL_B_SPLINE_SURFACE: {
                vector<vector<gm::Point>> cp;

                auto [du, dv, cp_ref, kum, kvm, kuv, kvv, w] = step_read<
                    i_<str_, str_, str_, str_>,
                    br_<int_, int_, mat_<ref_>, i_<str_, bool_, bool_, bool_>>,
                    i_<str_>,
                    br_<list_<int_>, list_<int_>, list_<float_>, list_<float_>,
                        i_<str_>>,
                    i_<str_, str_, str_, str_>, br_<mat_<float_>>,
                    i_<str_, str_, str_, str_, str_, str_, str_, str_>>(tok,
                                                                        id);

                for (auto& row : cp_ref) {
                    vector<gm::Point> v;
                    for (auto& i : row)
                        v.emplace_back(get_point(i));
                    cp.emplace_back(v);
                }

                result = make_shared<gm::BSplineSurface>(du, dv, kum, kuv, kvm,
                                                         kvv, cp, w);
            }
            }
            surface_[id] = result;
        }
    }
    CHECK_IF(!result, err::null_pointer, "returning null surface");

    log_->debug("processed surface: {}", *result);
    return result;
}

const string& StepParser::at(size_t id) const
{
    CHECK_IF(data_.find(id) == cend(data_), err::id_not_loaded,
             "id (" + to_string(id) + ") is not loaded");
    return data_.at(id);
}

StepParser::StepParser(const StepLoader& data)
    : data_(data.data())
    , geom_()
    , log_(cmms::setup_logger(logger_id))
    , edge_()
    , curve_()
    , surface_()
{
}

vector<gm::Shell> StepParser::geom() const
{
    return geom_;
}
