#include "obstacle.h"
#include <math.h>

Obstacle::Obstacle():percolation(0),T2(0),id(-1)
{
}

bool Obstacle::checkCollision(Walker &walker, Eigen::Array3d &step, const double &step_lenght, Collision &colision)
{
    return false;
}

void Obstacle::elasticBounceAgainsPlane(Eigen::Vector3d &ray_origin, Eigen::Vector3d &normal, double &t, Eigen::Vector3d &step)
{

    Eigen::Vector3d ray =  -step;
    double rn = ray.dot(normal);

    // Caso 3) ni cerca ni paralela
    step = -ray + 2.0*normal*rn;

    //step = (rn>0.0)?normal:(-normal);

}

void Obstacle::elasticBounceAgainsPlane_intra(Eigen::Vector3d &ray_origin, Eigen::Vector3d &normal, double &t, Eigen::Vector3d &step)
{

    // should be the same as -step
    Eigen::Vector3d ray =  (-t*step).normalized();//
    // ||ray||*||normal||*cos(alpha) = cos(alpha), because both vectors are unitary 
    double rn = ray.dot(normal);
    // -ray should be the same as step
    step = -ray + 2.0*normal*rn;
}

void Obstacle::elasticBounceAgainsPlane_extra(Eigen::Vector3d &ray_origin, Eigen::Vector3d &normal, double &t, Eigen::Vector3d &step)
{

    Eigen::Vector3d ray =  (-t*step).normalized();//

    double rn = ray.dot(normal);
    step = -ray + 2.0*normal*rn;

}

double Obstacle::minDistance(Walker &w)
{
    return 0;

}
