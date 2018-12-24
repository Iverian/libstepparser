#ifndef HEXMESH_INCLUDE_STEP_PARSE_H_
#define HEXMESH_INCLUDE_STEP_PARSE_H_

#include <geom_model/shell.h>

#include <iostream>
#include <string>

BoundaryRep step_parse(const std::string &str);
BoundaryRep step_parse(std::istream &is);

#endif // HEXMESH_INCLUDE_STEP_PARSE_H_
