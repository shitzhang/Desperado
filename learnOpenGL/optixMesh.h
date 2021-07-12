#pragma once

#include "mesh.h"
#include "optixUtil.h"


#include <optixu/optixpp_namespace.h>
#include <optixu/optixu_matrix_namespace.h>

struct MeshBuffers
{
    optix::Buffer vertices;
    optix::Buffer indices;
    //optix::Buffer tri_indices;
    //optix::Buffer mat_indices;
    //optix::Buffer positions;
    //optix::Buffer normals;
    //optix::Buffer texcoords;
};

class OptiXMesh
{
public:
    OptiXMesh(std::shared_ptr<Mesh> mesh):p_mesh(mesh)
    {
        getMeshGeometry();
    }
    optix::GeometryGroup getMeshGeometry() {

        m_buffers.vertices = context->createBufferFromGLBO(RT_BUFFER_INPUT, p_mesh->getVBO());
        m_buffers.vertices->setFormat(RT_FORMAT_USER);
        m_buffers.vertices->setElementSize(sizeof(Vertex));
        m_buffers.vertices->setSize(p_mesh->vertices.size());

        m_buffers.indices = context->createBufferFromGLBO(RT_BUFFER_INPUT, p_mesh->getEBO());
        m_buffers.indices->setFormat(RT_FORMAT_UNSIGNED_INT3);
        m_buffers.indices->setSize(p_mesh->indices.size() / 3);

        optix::GeometryTriangles geoTris = context->createGeometryTriangles();  
        geoTris->setPrimitiveCount(p_mesh->indices.size() / 3);

        geoTris->setVertices(p_mesh->vertices.size(), m_buffers.vertices, 0, sizeof(Vertex), RT_FORMAT_FLOAT3);
        geoTris->setTriangleIndices(m_buffers.indices, RT_FORMAT_UNSIGNED_INT3);
        geoTris->setBuildFlags(RTgeometrybuildflags(0));

        const char* ptx = optixUtil::getPtxString(0, "optixGeometryTriangles.cu");
        geoTris->setAttributeProgram(context->createProgramFromPTXString(ptx, "triangle_attributes"));

        geoTris["index_buffer"]->setBuffer(m_buffers.indices);
        geoTris["vertex_buffer"]->setBuffer(m_buffers.vertices);

        geom_instance = context->createGeometryInstance(geoTris, material);

        optix::GeometryGroup ggTris = context->createGeometryGroup();
        ggTris->addChild(geom_instance);
        ggTris->setAcceleration(context->createAcceleration("Trbvh"));

        return ggTris; 
    }


    MeshBuffers m_buffers;
    std::shared_ptr<Mesh> p_mesh;
    // Input
    optix::Context               context;       // required
    optix::Material              material;      // optional single matl override

    optix::Program               intersection;  // optional 
    optix::Program               bounds;        // optional

    optix::Program               closest_hit;   // optional multi matl override
    optix::Program               any_hit;       // optional


    // Output
    optix::GeometryInstance      geom_instance;
    optix::float3                bbox_min;
    optix::float3                bbox_max;

    int                          num_triangles;
};