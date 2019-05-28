#ifndef STEPPARSE_SRC_STEP_STEPPARSE_HPP_
#define STEPPARSE_SRC_STEP_STEPPARSE_HPP_

#include <cmms/logging.hpp>
#include <gm/face.hpp>
#include <gm/oriented_edge.hpp>
#include <gm/shell.hpp>
#include <util/debug.hpp>

#include "step_entities.hpp"
#include "step_loader.hpp"

#include <string>

EXCEPT(null_pointer, "")
EXCEPT(id_not_loaded, "")
EXCEPT(bspline_vertex_not_match, "")

class StepParser {
public:
    static constexpr auto logger_id = "step_parse";

    using id_list_t = std::vector<size_t>;

    explicit StepParser(const StepLoader& data);

    StepParser& parse();

    std::vector<std::pair<size_t, gm::Axis>> get_shells();
    id_list_t get_faces(size_t id);

    gm::Face get_face(size_t id);

    std::pair<gm::FaceBound, bool> get_bound(size_t id);
    gm::OrientedEdge get_oedge(size_t id);
    gm::Edge get_edge(size_t id);

    gm::Point get_vertex(size_t id) const;

    gm::Vec get_dir(size_t id) const;
    gm::Point get_point(size_t id) const;
    gm::Vec get_vec(size_t id) const;
    gm::Axis get_axis(size_t id) const;

    std::shared_ptr<gm::AbstractCurve> get_curve(size_t id) const;
    std::shared_ptr<gm::AbstractSurface> get_surface(size_t id) const;
    bool is_curve_bspline(size_t curve_id) const;

    std::vector<gm::Shell> geom() const;

private:
    const std::string& at(size_t id) const;

    const StepLoader::data_t& data_;
    std::vector<gm::Shell> geom_;
    cmms::Logger log_;

    mutable std::map<size_t, gm::Edge> edge_;
    mutable std::map<size_t, std::shared_ptr<gm::AbstractCurve>> curve_;
    mutable std::map<size_t, std::shared_ptr<gm::AbstractSurface>> surface_;
};

#endif // STEPPARSE_SRC_STEP_STEPPARSE_HPP_
