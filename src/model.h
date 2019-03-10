#ifndef __MODEL_H__
#define __MODEL_H__

#include <vector>
#include <string>
#include <tinyrenderer/geometry.h>

class Model {
private:
    std::vector<Vec3f> verts;
    std::vector<std::vector<Vec3i> > faces; // attention, this Vec3i means vertex/uv/normal ids

    std::vector<Vec3f> norms;
    std::vector<Vec2f> texcoords;
public:
    Model(const char *filename);
    void get_bbox(Vec3f &min, Vec3f &max);  // bounding box for all the vertices, including isolated ones

    int nverts();                           // number of vertices
    int nfaces();                           // number of triangles

    Vec3f &point(int i);                    // coordinates of the vertex i
    int     vert(int fi, int li);           // index of the vertex for the triangle fi and local index (corner) li
    Vec2f     uv(int fi, int li);           // tex coord for the corner li of the triangle fi
    Vec3f normal(int fi, int li);           // normal vector for the corner li of the triangle fi
};

#endif //__MODEL_H__

