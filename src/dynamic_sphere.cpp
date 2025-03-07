#include "dynamic_sphere.h"
#include "sphere.h"
#include "constants.h"
#include <Eigen/Dense>
#include <iostream>
#include "simerrno.h"
using namespace Eigen;
using namespace std;



Dynamic_Sphere::Dynamic_Sphere(Vector3d soma_center, double soma_radius, int const& sphere_id): Sphere(soma_center, soma_radius)
{
    swell           = false;
    volume_inc_perc = 0;
    id              = sphere_id;
}


Dynamic_Sphere::Dynamic_Sphere(Dynamic_Sphere const& sph): 
id(sph.id), swell(sph.swell), volume_inc_perc(sph.volume_inc_perc),  ax_id(sph.ax_id),
min_radius(sph.min_radius), neighboring_spheres(sph.neighboring_spheres)
{   
    center = sph.center;
    radius = sph.radius;
}

void Dynamic_Sphere::set_center(Eigen::Vector3d center_)
{
    this->center = center_; 

}


void Dynamic_Sphere::add_neighbor(Dynamic_Sphere* const neighbor)
{
    neighboring_spheres.push_back(neighbor);
}

bool Dynamic_Sphere::isInside(Walker &w){

    //Minimum distance to the sphere wall.
    double d_ = minDistance(w);
   // return d_>0.0?d_:0.0;
    return d_ <= 0;
}

bool Dynamic_Sphere::isInside(Eigen::Vector3d pos, double distance_to_be_inside) const{
    
    double d_ = (pos - this->center).norm();
    d_ = d_-this->radius;
    
   // return d_>0.0?d_:0.0;
    return d_ <= distance_to_be_inside;
}

bool Dynamic_Sphere::distSmallerThan(Eigen::Vector3d pos, double distance){
    double d_ = (pos - this->center).norm();
    
    return d_ <= distance;
}
