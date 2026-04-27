#include "areafeature.h"
#include "util.h"

#ifndef CALLBACK
#define CALLBACK
#endif

//void CALLBACK errorCallback(GLenum errorCode)
//{
//   const GLubyte *estring;
//
//   estring = gluErrorString(errorCode);
//   fprintf(stderr, "Tessellation Error: %s\n", estring);
//   exit(0);
//}
//
//void CALLBACK combineCallback(GLdouble coords[3], 
//                     GLdouble *vertex_data[4],
//                     GLfloat weight[4], GLdouble **dataOut )
//{
//   GLdouble *vertex = (GLdouble *) malloc(2 * sizeof(GLdouble));
//
//   vertex[0] = coords[0];
//   vertex[1] = coords[1];
//   *dataOut = vertex;
//}

AreaFeature::AreaFeature():hasChecked(false)
{
}

AreaFeature::~AreaFeature()
{
}

int AreaFeature::CompleteContours(std::map<int, EdgeVector> &edge_vectors_map)
{
    int ref_count = ref_record_ids.size();
    std::vector<int> contour_edges;
    // iterate through each edge, we record the first edge's first
    // vertex, we then go through each last vertex of each edge
    // to check if it equals the first edge's first vertex. if it
    // does we have a contour.
    sg2d_t *first_vertex = edge_vectors_map[ref_record_ids[0]].GetNode(0);
    for (int i = 0; i < ref_count; i++) 
	{
        int node_count = edge_vectors_map[ref_record_ids[i]].GetSize();
        sg2d_t *last_vertex = edge_vectors_map[ref_record_ids[i]].GetNode(node_count-1);

        contour_edges.push_back(ref_record_ids[i]);
        
		if ((first_vertex->long_lat[0] == last_vertex->long_lat[0]) &&
            (first_vertex->long_lat[1] == last_vertex->long_lat[1])) 
		{
            // we've found a complete contour! push it back and
            // get a new beginning vertex.
            contours.push_back(contour_edges);
            contour_edges.clear();
            if (i > (ref_count - 1))
                first_vertex = edge_vectors_map[ref_record_ids[i]].GetNode(0);
        }
    }

    return 0;
}
