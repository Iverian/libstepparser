#ifndef HEXMESH_SRC_STEP_STEP_PARSER_H_
#define HEXMESH_SRC_STEP_STEP_PARSER_H_

#include <geom_model/face.h>
#include <geom_model/oriented_edge.h>
#include <geom_model/shell.h>

#include <util/debug.h>

#include "step_entities.h"
#include "step_loader.h"

#include <string>

EXCEPT(null_pointer, "")
EXCEPT(id_not_loaded, "")
EXCEPT(bspline_vertex_not_match, "")

class StepParser {
public:
    using id_list_t = std::vector<size_t>;

    explicit StepParser(const StepLoader& data);

    StepParser& parse();

    std::vector<std::pair<size_t, Axis>> get_shells();
    id_list_t get_faces(size_t id);

    Face get_face(size_t id);

    std::pair<FaceBound, bool> get_bound(size_t id);
    OrientedEdge get_oedge(size_t id);
    Edge get_edge(size_t id);

    Point get_vertex(size_t id) const;

    Vec get_dir(size_t id) const;
    Point get_point(size_t id) const;
    Vec get_vec(size_t id) const;
    Axis get_axis(size_t id) const;

    std::shared_ptr<AbstractCurve> get_curve(size_t id) const;
    std::shared_ptr<AbstractSurface> get_surface(size_t id) const;
    bool is_curve_bspline(size_t curve_id) const;

    BoundaryRep geom() const;

private:
    const std::string& at(size_t id) const;

    const StepLoader::data_t& data_;
    BoundaryRep geom_;

    mutable std::map<size_t, Edge> edge_;
    mutable std::map<size_t, std::shared_ptr<AbstractCurve>> curve_;
    mutable std::map<size_t, std::shared_ptr<AbstractSurface>> surface_;
};

#endif // HEXMESH_SRC_STEP_STEP_PARSER_H_
