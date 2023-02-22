#pragma once

#include <array>
#include <vector>
#include <xmemory>
#include <algorithm>
#include <span>

#include <vtkSmartPointer.h>
#include <vtkPoints.h>
#include <vtkCellArray.h>
#include <vtkTriangle.h>
#include <vtkQuad.h>
#include <vtkPolygon.h>
#include <vtkPolyData.h>

#include <gbs/surfaces>

#include "baseIntersection.h"

namespace gbs
{
    template <typename T, size_t dim>
    struct HalfEdge;
    template <typename T, size_t dim>
    struct HalfEdgeVertex;
    template <typename T, size_t dim>
    struct HalfEdgeFace;

    template <typename T, size_t dim>
    struct HalfEdgeVertex
    {
        std::array<T, dim> coords;
        std::shared_ptr<HalfEdge<T, dim>> edge;
    };

    template <typename T, size_t dim>
    struct HalfEdgeFace
    {
        std::shared_ptr<HalfEdge<T, dim>> edge;
    };

    template <typename T, size_t dim>
    struct HalfEdge
    {
        std::shared_ptr<HalfEdgeVertex<T, dim>> vertex;
        std::shared_ptr<HalfEdgeFace<T, dim>> face;
        std::shared_ptr<HalfEdge<T, dim>> next;
        std::shared_ptr<HalfEdge<T, dim>> previous;
        std::shared_ptr<HalfEdge<T, dim>> opposite;

        // HalfEdge(const std::array<T, dim> &coords) : vertex{std::make_shared<HalfEdgeVertex<T, dim>>({coords, this})} {}
    };

/**
 * @brief Builds half edge from vertex and tag, if the vertex is free it is tagged as belonging to the edge
 * 
 * @tparam T 
 * @tparam dim 
 * @param vertex 
 * @return std::shared_ptr< HalfEdge<T, dim> > 
 */

    template <typename T, size_t dim>
    auto make_shared_h_edge(const std::shared_ptr<HalfEdgeVertex<T, dim>> &vertex, const std::shared_ptr<HalfEdgeFace<T, dim>> &face = nullptr) -> std::shared_ptr< HalfEdge<T, dim> >
    {
        auto hedge  = std::make_shared< HalfEdge<T,dim> >(
            HalfEdge<T,dim>{
                .vertex = vertex,
                .face   = face
            }
        );

        if(!vertex->edge)
        {
            vertex->edge = hedge;
        }

        return hedge;
    }

    template <typename T, size_t dim>
    auto make_shared_h_vertex(const std::array<T, dim> &coords)
    {
        return std::make_shared<HalfEdgeVertex<T, dim>>(HalfEdgeVertex<T, dim>{coords,nullptr});
    }

/**
 * @brief Builds half edge and its vertices from coordinate
 * 
 * @tparam T 
 * @tparam dim 
 * @param coords 
 * @return std::shared_ptr< HalfEdge<T, dim> > 
 */
    template <typename T, size_t dim>
    auto make_shared_h_edge(const std::array<T, dim> &coords) -> std::shared_ptr< HalfEdge<T, dim> >
    {
        auto vertex = std::make_shared<HalfEdgeVertex<T, dim>>(HalfEdgeVertex<T, dim>{coords,nullptr});
        return make_shared_h_edge<T,dim>(vertex);
    }

    template <typename T, size_t dim, typename _Container>
    auto make_shared_h_vertices(const _Container &coords)
    {
        auto n = coords.size();
        std::vector< std::shared_ptr< HalfEdgeVertex<T,dim> > > vertices_cloud(n);
        std::transform(
            coords.begin(),
            coords.end(),
            vertices_cloud.begin(),
            [] (const auto &xyz){ return make_shared_h_vertex<T,dim>(xyz);}
        );
        return vertices_cloud;
    }

    template <typename T, size_t dim, typename _Container>
    auto make_shared_h_edges(const _Container &coords)
    {
        auto n = coords.size();
        std::vector< std::shared_ptr< HalfEdge<T,dim> > > vertices_cloud(n);
        std::transform(
            coords.begin(),
            coords.end(),
            vertices_cloud.begin(),
            [] (const auto &xyz){ return make_shared_h_edge<T,dim>(xyz);}
        );
        return vertices_cloud;
    }

    template <typename T, size_t dim, typename _It>
    void make_loop(const  _It &begin,  const _It &end, std::shared_ptr< HalfEdgeFace<T, dim> > &p_face)
    {
        auto n = std::distance(begin, end);
        if(p_face)
        {
            p_face->edge = (*begin);
        }
        auto it = begin;
        while (it != end)
        {
            (*it)->face = p_face;
            (*it)->next     = std::next(it) != end ? *std::next(it)  : *begin;
            (*it)->previous = it != begin ? *std::prev(it) : *std::next(begin,n-1);
            std::advance(it,1);
        }
    }

/**
 * @brief Builds a face from a list of half edges. Edges are tagged as belonging to the newly created face.
 *  A nullptr is returned if an half edge already belongs to a face.
 * 
 * @tparam T 
 * @tparam dim 
 * @tparam _It 
 * @param begin 
 * @param end 
 * @return std::shared_ptr< HalfEdgeFace<T, dim> > 
 */
    template <typename T, size_t dim, typename _It>
    auto make_shared_h_face( const  _It &begin,  const _It &end)  -> std::shared_ptr< HalfEdgeFace<T, dim> >
    {

        auto n = std::distance(begin, end);
        if(n<2)
        {
            return nullptr;
        }

        auto p_face = std::make_shared<HalfEdgeFace<T, dim>>();
        make_loop(begin, end, p_face);

        return p_face;
        
    }

/**
 * @brief Builds a face from a list of half edges. Edges are tagged as belonging to the newly created face.
 *  A nullptr is returned if an half edge already belongs to a face.
 * 
 * @tparam T 
 * @tparam dim 
 * @param lst 
 * @return std::shared_ptr< HalfEdgeFace<T, dim> > 
 */
    template <typename T, size_t dim>
    auto make_shared_h_face( const auto &lst ) -> std::shared_ptr< HalfEdgeFace<T, dim> >
    {
        return make_shared_h_face<T,dim>(lst.begin(), lst.end());
    }

    template <typename T, size_t dim>
    auto make_opposite(const std::shared_ptr<HalfEdgeVertex<T, dim>> &vertex, const std::shared_ptr<HalfEdge<T, dim>> &edge)
    {
        auto opposite = make_shared_h_edge( edge->previous->vertex );
        opposite->opposite = edge;
        edge->opposite = opposite;
        return opposite;
    }

    template <typename T, size_t dim>
    auto add_face(
        const std::shared_ptr<HalfEdgeFace<T, dim>> &face, 
        const std::shared_ptr<HalfEdge<T, dim>> &edge, 
        const std::array<T, dim> &coords )  -> std::shared_ptr< HalfEdgeFace<T, dim> >
    {
        if(!edge || edge->opposite || edge->face != face)
        {
            return nullptr;
        }
        
        auto opposite = make_opposite( edge->previous->vertex, edge);

        auto lst = { make_shared_h_edge( edge->vertex ), opposite, make_shared_h_edge( coords )};
        
        return make_shared_h_face<T,dim>(lst);
    }

    template <typename T, size_t dim>
    auto add_face(
        const std::shared_ptr<HalfEdge<T, dim>> &edge, 
        const std::array<T, dim> &coords )  -> std::shared_ptr< HalfEdgeFace<T, dim> >
    {
        if(!edge || edge->opposite)
        {
            return nullptr;
        }
        
        auto opposite = make_opposite( edge->previous->vertex, edge);

        auto lst = { make_shared_h_edge( edge->vertex ), opposite, make_shared_h_edge( coords )};
        
        return make_shared_h_face<T,dim>(lst);
    }

    template <typename T, size_t dim>
    auto getFaceEdge( const std::shared_ptr<HalfEdgeFace<T, dim>> &face, const std::shared_ptr<HalfEdgeVertex<T, dim>> &vertex) -> std::shared_ptr<HalfEdge<T, dim>>
    {
        auto edge = face->edge;
        while (edge->vertex != vertex)
        {
            edge = edge->next;
            if(edge == face->edge) // loop completed
            {
                return nullptr;
            }
        }
        return edge;
    }

    template <typename T, size_t dim>
    auto getFaceEdges( const HalfEdgeFace<T, dim> &face)
    {
        std::list< std::shared_ptr< HalfEdge<T, dim> > > edges_lst;
        auto edge = face.edge;
        while (edge)
        {
            edges_lst.push_back( edge );
            edge = edge->next;
            if(edge == face.edge)
            {
                break;
            }
        }
        return edges_lst;
    }

    template <typename T, size_t dim>
    auto getFaceEdges( const std::shared_ptr<HalfEdgeFace<T, dim>> &face)
    {
        return getFaceEdges(*face);
    }

    template <typename T, size_t dim>
    auto getFaceVertices( const std::shared_ptr<HalfEdgeFace<T, dim>> &face)
    {
        std::list< std::shared_ptr< HalfEdgeVertex<T, dim> > > vtx_lst;
        auto edge = face->edge;
        while (edge)
        {
            vtx_lst.push_back( edge->vertex );
            edge = edge->next;
            if(edge == face->edge)
            {
                break;
            }
        }
        return vtx_lst;
    }

    template <typename T, size_t dim>
    auto getFaceCoords( const std::shared_ptr<HalfEdgeFace<T, dim>> &face)
    {
        std::list< std::array<T,dim> > coords_lst;
        auto edge = face->edge;
        while (edge)
        {
            coords_lst.push_back( edge->vertex->coords );
            edge = edge->next;
            if(edge == face->edge)
            {
                break;
            }
        }
        return coords_lst;
    }

    /**
     * @brief Get the Common Edge of h_f1 with h_f2, nullptr if none
     * 
     * @tparam T 
     * @tparam dim 
     * @param h_f1 
     * @param h_f2 
     * @return std::shared_ptr<HalfEdge<T, dim>> 
     */
    template <typename T, size_t dim>
    auto getCommonEdge(const std::shared_ptr<HalfEdgeFace<T, dim>> &h_f1,const std::shared_ptr<HalfEdgeFace<T, dim>> &h_f2) -> std::shared_ptr<HalfEdge<T, dim>>
    {
        for(const auto & h_e : getFaceEdges(h_f1))
        {
            if(h_e->opposite && h_e->opposite->face == h_f2) return h_e;
        }
        return nullptr;
    }
    /**
     * @brief Get the Common Edges of h_f1 with h_f2, (nullptr, nullptr) if none
     * 
     * @tparam T 
     * @tparam dim 
     * @param h_f1 
     * @param h_f2 
     * @return std::pair< std::shared_ptr<HalfEdge<T, dim>>, std::shared_ptr<HalfEdge<T, dim>> > 
     */
    template <typename T, size_t dim>
    auto getCommonEdges(const std::shared_ptr<HalfEdgeFace<T, dim>> &h_f1,const std::shared_ptr<HalfEdgeFace<T, dim>> &h_f2) -> std::pair< std::shared_ptr<HalfEdge<T, dim>>, std::shared_ptr<HalfEdge<T, dim>> >
    {
        if( auto h_e1 = getCommonEdge(h_f1, h_f2) )
        {
            return std::make_pair(h_e1, h_e1->opposite);
        }
        return std::make_pair(nullptr, nullptr);
    }

    template <typename T, size_t dim>
    void associate(std::shared_ptr<HalfEdgeVertex<T, dim>> &h_v, std::shared_ptr<HalfEdge<T, dim>> &h_e)
    {
        h_e->vertex = h_v;
        h_v->edge = h_e;
    }

    template <typename T, size_t dim>
    void flip(std::shared_ptr<HalfEdgeFace<T, dim>> &h_f1, std::shared_ptr<HalfEdgeFace<T, dim>> &h_f2)
    {
        assert(getFaceEdges(h_f1).size() == 3);
        assert(getFaceEdges(h_f2).size() == 3);
        auto [h_e1_1, h_e1_2] = getCommonEdges(h_f1, h_f2);
        if(h_e1_1 == nullptr || h_e1_2 == nullptr) return;
        auto h_e2_1 = h_e1_1->next;
        auto h_e3_1 = h_e2_1->next;
        auto h_e2_2 = h_e1_2->next;
        auto h_e3_2 = h_e2_2->next;

        auto h_v1 = h_e1_1->vertex;
        auto h_v2 = h_e2_1->vertex;
        auto h_v3 = h_e3_1->vertex;
        auto h_v4 = h_e2_2->vertex;

        associate(h_v4, h_e1_1);
        associate(h_v2, h_e1_2);

        std::list< std::shared_ptr<HalfEdge<T, dim>> > lst1{h_e1_1, h_e3_2, h_e2_1};
        std::list< std::shared_ptr<HalfEdge<T, dim>> > lst2{h_e1_2, h_e3_1, h_e2_2};

        make_loop(lst1.begin(), lst1.end(), h_f1);
        make_loop(lst2.begin(), lst2.end(), h_f2);

    }

    template <typename T, size_t dim>
    void link_edges(const std::shared_ptr<HalfEdge<T, dim>> &h_e1, const std::shared_ptr<HalfEdge<T, dim>> &h_e2)
    {
        assert(h_e1);
        assert(h_e2);
        h_e1->opposite = h_e2;
        h_e2->opposite = h_e1;
    }

    template <typename T, size_t dim>
    auto add_vertex(const std::shared_ptr<HalfEdgeFace<T, dim>> &h_f, const std::shared_ptr<HalfEdgeVertex<T, dim>> &h_v)
    {
        auto h_e_lst = getFaceEdges(h_f);
        std::list<std::shared_ptr<HalfEdgeFace<T, dim>>> h_f_lst;

        std::shared_ptr<HalfEdge<T, dim>> h_e_prev{};
        h_v->edge = nullptr; // first new half edge takes ownership
        for(const auto &h_e : h_e_lst)
        {
            assert(h_e->previous);
            auto h_e1 = make_shared_h_edge(h_v);
            auto h_e2 = make_shared_h_edge(h_e->previous->vertex);
            if(h_e_prev)
            {
                link_edges(h_e2, h_e_prev);
            }
            h_e_prev = h_e1;
            auto lst = {h_e,h_e1, h_e2};
            h_f_lst.push_back(
                make_shared_h_face<T,dim>( lst )
            );
        }
        link_edges(h_f_lst.back()->edge->next, h_f_lst.front()->edge->previous);
        return h_f_lst;
    }

    template <typename T, size_t dim>
    auto getPreviousFace(const std::shared_ptr<HalfEdge<T, dim>> &edge) -> std::shared_ptr< HalfEdgeFace<T,dim> >
    {
        if(edge->next)
        {
            auto opp = edge->next->opposite;
            if(opp)
            {
                return opp->face;
            }
        }
        return nullptr;
    }

    template <typename T, size_t dim>
    auto getNextFace(const std::shared_ptr<HalfEdge<T, dim>> &edge) -> std::shared_ptr< HalfEdgeFace<T,dim> >
    {
        auto opp = edge->opposite;
        if(opp)
        {
            return opp->face;
        }
        return nullptr;
    }

    // template <typename T, size_t dim>
    // auto getFaces(const std::shared_ptr<HalfEdge<T, dim>> &edge) -> std::list< std::shared_ptr< HalfEdgeFace<T,dim> > >
    // {
    //     std::list< std::shared_ptr< HalfEdgeFace<T,dim> > > face;

    // }

    template <typename T, size_t dim>
    auto getNeighboringFaces( const std::shared_ptr<HalfEdgeFace<T, dim>> &h_f)
    {
        std::list< std::shared_ptr< HalfEdgeFace<T,dim> > > neighbors;
        auto edges = getFaceEdges(h_f);
        for(const auto &h_e : edges)
        {
            if(h_e->opposite)
            {
                assert(h_e->opposite->face);
                neighbors.push_back((h_e->opposite->face));
            }
        }
        return neighbors;
    }

//https://www.graphics.rwth-aachen.de/software/openmesh/intro/
    template <typename T, size_t dim>
    auto getNeighbors( const std::shared_ptr<HalfEdgeVertex<T, dim>> &vertex)
    {
        std::list< std::shared_ptr< HalfEdgeVertex<T, dim> > > vtx_lst;
        auto edge = vertex->edge;

        while (!edge->face)
        {
            /* code */
        }
        

        // while (edge-> && edge != vertex->edge)
        // {
        //     vtx_lst(edge);
            
        // }
        
    }


//     }

//     auto getIncomings()
//     {

//     }

    // template <typename T, size_t dim>
    // auto getEdgeLoop( const std::shared_ptr<HalfEdge<T, dim>> &ed_start) -> std::list< std::shared_ptr<HalfEdge<T, dim>> >
    // {

    //     std::list< std::shared_ptr< HalfEdge<T, dim> > > edges_lst;

    //     auto ed{ed_start};

    //     while(ed)
    //     {
    //         edges_lst.push_back( ed );
    //         ed = ed->next;
    //         if(ed_start == ed)
    //         {
    //             break;
    //         }
    //     }

    //     if(edges_lst.back()->next != edges_lst.front()->previous) // only closed loop
    //     {
    //         edges_lst.clear();
    //     }

    //     return edges_lst;
    // }


    // template <typename T, size_t dim>
    // auto getVertexMainLoop( const std::shared_ptr<HalfEdgeVertex<T, dim>> &vertex) -> std::list< std::shared_ptr<HalfEdge<T, dim>> >
    // {
    //     auto edges_lst = getEdgeLoop(vertex->edge);
    //     // permutation to start with the half edge coming from the vertex
    //     auto front = edges_lst.front();
    //     edges_lst.push_back(  front ); 
    //     edges_lst.pop_front();
    //     return edges_lst;
    // }

    // template <typename T, size_t dim>
    // auto getVertexLoops( const std::shared_ptr<HalfEdgeVertex<T, dim>> &vertex) -> std::list< std::list< std::shared_ptr<HalfEdge<T, dim>> > >
    // {
    //     std::list< std::list< std::shared_ptr< HalfEdge<T, dim> > > > edges_lst;

    //     auto main_loop = getVertexMainLoop(vertex);


    //     auto next_opp = main_loop.front()->opposite;
        
    //     while (next_opp &&  next_opp != main_loop.back() )
    //     {
    //         auto next_loop = getEdgeLoop( next_opp->next );

    //     }
        
        
    //     auto prev_opp = main_loop.back()->opposite;



    // }

    // template <typename T, size_t dim>
    // auto getConnectedVertices( const std::shared_ptr<HalfEdgeVertex<T, dim>> &vertex) -> std::list< std::shared_ptr< HalfEdgeVertex<T, dim> > >
    // {
    //     std::list< std::shared_ptr< HalfEdgeVertex<T, dim> > > vertices_lst;

    //     auto main_loop = getVertexMainLoop(vertex);
    //     if(main_loop.size())
    //     {
    //         auto it = main_loop.begin();
    //         vertices_lst.push_back(*(it))
    //         std::advance(it,main_loop.size()-1);
    //         vertices_lst.push_back(*(it))
    //     }



    //     // auto ed = vertex->edge;
    //     // auto ed_prev = ed->opposite;
    //     // while (ed_prev)
    //     // {
    //     //     vertices_lst.push_front(ed_prev->vertex);
    //     //     if(ed_prev->previous)
    //     //     {
    //     //         ed_prev = ed_prev->previous->opposite;
    //     //     }
    //     //     else
    //     //     {
    //     //         break;
    //     //     }
    //     // }

    //     // auto ed_next = ed->next;
    //     // while (ed_next)
    //     // {
    //     //     vertices_lst.push_back(ed_next->vertex);
    //     //     if( ed_next->opposite)
    //     //     {
    //     //         ed_next = ed_next->opposite->next;
    //     //     }
    //     //     else
    //     //     {
    //     //         break;
    //     //     }
    //     // }
        
    //     return vertices_lst;
    // }

    template <typename T, size_t dim>
    struct HalfEdgeMesh
    {
        std::vector<std::shared_ptr<HalfEdge<T, dim>>> edges;
        std::vector<std::shared_ptr<HalfEdgeVertex<T, dim>>> vertices;
        std::vector<std::shared_ptr<HalfEdgeFace<T, dim>>> faces;

        // void addEdge( const std::array<T,dim> &coord )
        // {
        //     edges.push_back( std::make)
        // }

        void addFreeVertices(std::vector<std::array<T, dim>> &free_vertices)
        {
            auto n = vertices.size();
            vertices.insert(vertices.end(),free_vertices.size(), nullptr);

            std::transform(
                free_vertices.begin(), free_vertices.end(),
                std::next(vertices.begin(),n),
                [](const auto &coords)
                {
                    return std::make_shared<HalfEdgeVertex<T, dim>>(
                        HalfEdgeVertex<T, dim>{coords, nullptr}
                    );
                }
            );
        }
        
        /**
         * @brief Must be cw
         * 
         * @param i 
         * @param j 
         * @param k 
         * @return true 
         * @return false 
         */
        bool makeFace( size_t i, size_t j, size_t k)
        {
            // auto vtx1 = vertices[i];
            // auto vtx2 = vertices[j];
            // auto vtx3 = vertices[k];

            // auto p_face = std::make_shared<HalfEdgeFace<T, dim>>()

            // HalfEdge<T, dim> ed1{
            //     .vertex = vtx1,
            //     .face   = p_face,
            // };
            // HalfEdge<T, dim> ed2{
            //     .vertex = vtx2,
            //     .face   = p_face,
            // };
            // HalfEdge<T, dim> ed3{
            //     .vertex = vtx3,
            //     .face   = p_face,
            // };

            // // auto p_ed1 = std::make_shared< HalfEdge<T, dim> > (  HalfEdge<T, dim>{, .} 


            return true;
        }
    };

    template <typename T, size_t dim>
    auto extract_vertices_map(const auto &faces_lst ) -> std::map< std::shared_ptr< HalfEdgeVertex<T,dim> >, size_t >
    {
        std::map< std::shared_ptr< HalfEdgeVertex<T,dim> >, size_t > vertices_map;
        size_t index{};
        for( const auto &f : faces_lst)
        {
            auto vtx_lst = getFaceVertices(f);
            for( const auto &vtx : vtx_lst)
            {
                if(!vertices_map.contains(vtx))
                {
                    vertices_map[vtx] = index;
                    index++;
                }
            }
        }

        return vertices_map;
    }

    template <typename T, size_t dim>
    auto extract_edges_map(const auto &faces_lst ) -> std::map< std::shared_ptr< HalfEdgeVertex<T,dim> >, size_t >
    {
        std::map< std::shared_ptr< HalfEdgeEdge<T,dim> >, size_t > edges_map;
        size_t index{};
        for( const auto &f : faces_lst)
        {
            auto hed_lst = getFaceEdges(f);
            for( const auto &hed : hed_lst)
            {
                if(!edges_map.contains(hed))
                {
                    edges_map[hed] = index;
                    index++;
                }
            }
        }

        return edges_map;
    }

    template <typename T, size_t dim>
    auto extract_edges_boundary(const auto &faces_lst )
    {
        auto edges_map = extract_edges_map(faces_lst);
        auto bound_start = std::find_if(
            edges_map.begin(), edges_map.end(),
            [](const auto &hed_id){return !hed_id.fisrt->opposite;}
        );

        auto hed = *bound_start;
        do{

        }while (hed != *bound_start);
        
    }

    template <typename T, size_t dim>
    auto make_polydata(const auto &faces_lst)
    {
        // valid only up to 3D
        static_assert(dim<4);

        // // build vertices map and store vtk points
        vtkNew<vtkPoints> points;
        auto vertices_map = extract_vertices_map<T,dim>(faces_lst);
        points->Allocate(vertices_map.size());
        points->SetNumberOfPoints(vertices_map.size());
        for(const auto &vtx : vertices_map)
        {
            points->SetPoint( vtx.second, make_vtkPoint(vtx.first->coords).data());
        }
        
        // Store cells
        vtkNew<vtkCellArray> cells;

        for( const auto &f : faces_lst)
        {
            auto vtx_lst = getFaceVertices(f);
            vtkSmartPointer<vtkCell> cell;
            auto n = vtx_lst.size();
            switch (n)
            {
            case 3:
                {
                    cell = vtkSmartPointer<vtkTriangle>::New();
                }
                break;
            case 4:
                {
                    cell = vtkSmartPointer<vtkQuad>::New();
                }
            default:
                    cell = vtkSmartPointer<vtkPolygon>::New();
                    cell->GetPointIds()->SetNumberOfIds(n);
                break;
            }
            std::transform(
                vtx_lst.begin(), vtx_lst.end(),
                cell->GetPointIds()->begin(),
                [&vertices_map](const auto vtx)
                { return vertices_map[vtx]; });
            cells->InsertNextCell(cell);
        }

        vtkNew<vtkPolyData> polyData;

        // Add the geometry and topology to the polydata
        polyData->SetPoints(points);
        polyData->SetPolys(cells);

        return polyData;

    }

    template <typename T, size_t dim>
    auto make_polydata(const auto &faces_lst, const Surface<T,dim> &srf)
    {
        // valid only up to 3D
        static_assert(dim<4);

        // // build vertices map and store vtk points
        vtkNew<vtkPoints> points;
        auto vertices_map = extract_vertices_map<T,2>(faces_lst);
        points->Allocate(vertices_map.size());
        points->SetNumberOfPoints(vertices_map.size());
        for(const auto &vtx : vertices_map)
        {
            auto [u,v] = vtx.first->coords;
            points->SetPoint(vtx.second, make_vtkPoint(srf(u, v)).data());
        }
        
        // Store cells
        vtkNew<vtkCellArray> cells;

        for( const auto &f : faces_lst)
        {
            auto vtx_lst = getFaceVertices(f);
            vtkSmartPointer<vtkCell> cell;
            auto n = vtx_lst.size();
            switch (n)
            {
            case 3:
                {
                    cell = vtkSmartPointer<vtkTriangle>::New();
                }
                break;
            case 4:
                {
                    cell = vtkSmartPointer<vtkQuad>::New();
                }
            default:
                    cell = vtkSmartPointer<vtkPolygon>::New();
                    cell->GetPointIds()->SetNumberOfIds(n);
                break;
            }
            std::transform(
                vtx_lst.begin(), vtx_lst.end(),
                cell->GetPointIds()->begin(),
                [&vertices_map](const auto vtx)
                { return vertices_map[vtx]; });
            cells->InsertNextCell(cell);
        }

        vtkNew<vtkPolyData> polyData;

        // Add the geometry and topology to the polydata
        polyData->SetPoints(points);
        polyData->SetPolys(cells);

        return polyData;

    }
}