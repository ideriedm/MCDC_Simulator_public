#include "Dendrite.h"

#include <iostream>

#include <cmath>
#include <typeinfo>

using namespace Eigen;
using namespace std;

int Dendrite::nb_dendrites = 0;

Dendrite::Dendrite()
{
    id          = nb_dendrites++;
}

Dendrite::Dendrite(std::vector<Axon> const& subbranches_):Dendrite()
{
    subbranches = subbranches_;
}

Dendrite::Dendrite(Dendrite const& dendrite)
{
    subbranches = dendrite.subbranches;
    projections = dendrite.projections;
    projections_max = dendrite.projections_max;
}

Dendrite::~Dendrite()
{
    nb_dendrites--;
}

bool Dendrite::checkCollision(Walker &walker, Vector3d const&step_dir, double const&step_lenght, Collision &collision)
{
    bool bool_collision = false;
    for(size_t i=0; i < subbranches.size(); i++)
    {
        bool_collision = subbranches[i].checkCollision(walker, step_dir, step_lenght, collision);
        if(bool_collision)
            return true;
    }
    return false;
}

bool Dendrite::isPosInsideDendrite(Vector3d const& position,  double const& barrier_thickness, bool const& swell_)
{
    vector<int> dummy;
    for(size_t s=0; s < subbranches.size(); ++s)
    {
        if(subbranches[s].isPosInsideAxon(position, barrier_thickness, dummy))
            return true;
    }
    return false;
}

double Dendrite::minDistance(Vector3d const& pos) const
{
    // TO IMPLEMENT
    assert(0);
}

double Dendrite::minDistance(Walker const& walker) const
{
    // TO IMPLEMENT
    assert(0);
}

void Dendrite::add_subbranch(Axon& subbranch)
{
    subbranches.push_back(subbranch);
    subbranch.id = subbranches.size() - 1;
}

void Dendrite::set_dendrite(vector<Axon> const& subbranches_)
{
    subbranches = subbranches_;
    add_projection();
}

int Dendrite::get_nb_subbranches()
{
    return subbranches.size();
}

double Dendrite::volumeDendrite() const
{
    double volume = 0;
    for (size_t i=0; i < subbranches.size(); i++)
        volume += subbranches[i].volumeAxon();
    
    return volume;
}

void Dendrite::add_projection()
{
    for(int axis=0; axis < 3 ; axis++)
    {
            // Contains the minimum axis projection of the subbranches axon_projections
            double min_axis_projection     = 1000;
            // Contains the minimum axis projection of the subbranches axon_projections_max
            // double min_axis_projection_max = 1000;
            // Contains the maximum axis projection of the subbranches axon_projections
            double max_axis_projection     = 0;
            // Contains the maximum axis projection of the subbranches axon_projections_max
            // double max_axis_projection_max = 0;
            for(size_t b=0; b < subbranches.size(); b++)
            {        
                // double min_axis_projection_tmp     = subbranches[0].projections.axon_projections[axis][0];
                // double min_axis_projection_max_tmp = subbranches[0].projections_max.axon_projections[axis][0];
                // if(min_axis_projection_tmp < min_axis_projection)
                //     min_axis_projection = min_axis_projection_tmp;
                // if(min_axis_projection_max_tmp < min_axis_projection_max)
                //     min_axis_projection_max = min_axis_projection_max_tmp;

                // double max_axis_projection_tmp     = subbranches[0].projections.axon_projections[axis][1];
                // double max_axis_projection_max_tmp = subbranches[0].projections_max.axon_projections[axis][1];
                // if(max_axis_projection_tmp > max_axis_projection)
                //     max_axis_projection = max_axis_projection_tmp;
                // if(max_axis_projection_max_tmp > max_axis_projection_max)
                //     max_axis_projection_max = max_axis_projection_max_tmp;
                int size = subbranches[b].spheres.size()-1;
                double center0 = subbranches[b].spheres[0].center[axis];
                double centerEnd = subbranches[b].spheres[size].center[axis];
                double radius = subbranches[b].spheres[size].radius;
                double eps    = 0.1*radius;

                if(min_axis_projection > center0)
                    min_axis_projection = center0 - radius - eps;
                if(min_axis_projection > centerEnd)
                    min_axis_projection = centerEnd - radius - eps;

                if(max_axis_projection < center0)
                    max_axis_projection = center0 + radius + eps;
                if(max_axis_projection < centerEnd)
                    max_axis_projection = centerEnd + radius + eps;
            }
        projections.axon_projections.push_back({min_axis_projection, max_axis_projection});  
        projections_max.axon_projections.push_back({min_axis_projection, max_axis_projection}); 
    }
}