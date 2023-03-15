#pragma once

#include <array>
#include <vector>
#include <algorithm>
#include <span>

#include <gbs/surfaces>

#include "halfEdgeMeshData.h"
#include "halfEdgeMeshGetters.h"
#include "halfEdgeMeshEditors.h"
#include "baseIntersection.h"
#include "halfEdgeMeshGeomTests.h"
#include "baseGeom.h"

namespace gbs
{

    template<typename T, typename _Container>
    void boyerWatson(_Container &h_f_lst, const std::array<T,2> &xy, T tol = 1e-10)
    {
        auto begin = h_f_lst.begin();
        auto it{begin};
        auto end = h_f_lst.end();

        _Container h_f_lst_deleted;
        // Find triangle violation Delaunay condition
        while(it!=end)
        {
            it = std::find_if(
                it, end,
                [xy, tol](const auto &h_f)
                { 
                    return in_circle(xy, h_f) > tol; 
                });
            if(it!=end)
            {
                h_f_lst_deleted.push_back(*it);
                it = h_f_lst.erase( it );
            }
        }
        if(h_f_lst_deleted.size()==0) return; // if point is outside or confused with an existing point
        assert(are_face_ccw(h_f_lst));
        // Get cavity boundary
        auto h_e_lst = getOrientedFacesBoundary(h_f_lst_deleted);
        assert(are_edges_2d_ccw(h_e_lst));
        // fill cavity
        auto h_f_lst_new = add_vertex(h_e_lst, make_shared_h_vertex(xy));
        assert(are_face_ccw(h_f_lst_new));
        // append new faces
        h_f_lst.insert(h_f_lst.end(), h_f_lst_new.begin(), h_f_lst_new.end() );
    }


    // template < typename T> 
    // auto getCoordsMinMax(const std::list< std::shared_ptr<HalfEdgeFace<T, dim>> > &hf_lst)
    // {

    // }

    ///////////////////////


    template < typename T>
    auto delaunay2DBoyerWatson(const auto &coords, T tol)
    {
        auto faces_lst = getEncompassingMesh(coords);
        auto vertices = getVerticesVectorFromFaces<T,2>(faces_lst);
        // insert points
        for(const auto &xy : coords)
        {
            boyerWatson<T>(faces_lst, xy, tol);
        }

        // remove external mesh, i.ei faces attached to initial vertices
        for(const auto &vtx : vertices)
        {
            remove_faces(faces_lst, vtx);
        }

        return faces_lst;
    }

    template < typename T >
    auto delaunay2DBoyerWatson(const auto &coords_boundary,const auto &coords_inner, T tol)
    {
        auto faces_lst = getEncompassingMesh(coords_boundary);
        auto vertices = getVerticesVectorFromFaces<T,2>(faces_lst);
        // insert points
        for(const auto &xy : coords_boundary)
        {
            boyerWatson<T>(faces_lst, xy, tol);
        }

        // remove external mesh, i.ei faces attached to initial vertices
        for(const auto &vtx : vertices)
        {
            remove_faces(faces_lst, vtx);
        }

        for(const auto &xy : coords_inner)
        {
            boyerWatson<T>(faces_lst, xy, tol);
        }

        return faces_lst;
    }

    template < typename T>
    auto delaunay2DBoyerWatson(const std::vector< std::array<T,2> > &coords, T tol = 1e-10)
    {
        return delaunay2DBoyerWatson<T,std::vector< std::array<T,2> >>(coords, tol);
    }

// TODO remove
    template < typename T , auto _ExPo = std::execution::seq>
    auto base_delaunay2d_mesh(std::vector< std::shared_ptr< HalfEdgeVertex<T,2> > > &vertices_cloud)
    {
        auto n = vertices_cloud.size();

        std::list< std::shared_ptr< HalfEdgeFace<T,2> > > faces_lst;

        for(size_t i{}; i < n; i++)
        {
            auto a = vertices_cloud[i];
            // for(size_t j{i+1}; j < n; j++)
            for(size_t j{}; j < n; j++)
            {
                auto b = vertices_cloud[j];
                for( size_t k{j+1}; k < n ; k++)
                // for( size_t k{}; k < n ; k++)
                {
                    auto c   = vertices_cloud[k];

                    if(orient_2d(a->coords,b->coords,c->coords) > 0.) // avoids also null triangle
                    {
                    
                        auto it_inside_vtx =  std::find_if(
                            _ExPo,
                            vertices_cloud.begin(),
                            vertices_cloud.end(),
                            [a_coords=a->coords,b_coords=b->coords,c_coords=c->coords](const auto &d) { return in_circle(a_coords, b_coords, c_coords, d->coords) > 0.; }
                        );
                        auto is_delauney_triangle {vertices_cloud.end() == it_inside_vtx};

                        if(is_delauney_triangle)
                        {
                            // create potentially new face loop
                            auto he1 = make_shared_h_edge(a);
                            auto he2 = make_shared_h_edge(b);
                            auto he3 = make_shared_h_edge(c);
                            he1->previous = he3;
                            he2->previous = he1;
                            he3->previous = he2;

                            bool cross_tri{false}, is_perm{false};
                            for(const auto &h_face : faces_lst)
                            {
                                auto lst_vtx = {a, b, c};
                                // check if loop is a permutation of previously defined face
                                is_perm = std::is_permutation(lst_vtx.begin(), lst_vtx.end(), gbs::getFaceVertices(h_face).begin());
                                if(is_perm)
                                {
                                    break;
                                }
                                // check if edges are intersection face
                                cross_tri = 
                                    areFacesEdgesIntersect(*he1 , *h_face) ||
                                    areFacesEdgesIntersect(*he2 , *h_face) ||
                                    areFacesEdgesIntersect(*he3 , *h_face);
                                if(cross_tri)
                                {
                                    break;
                                }
                            }
                            if(!cross_tri && !is_perm)
                            {
                                auto lst1 = {he1, he2, he3};
                                faces_lst.push_back( make_shared_h_face<T,2>(lst1) );
                            }
                        }
                    
                    }
                }
            }
        }
        return faces_lst;
    }
}