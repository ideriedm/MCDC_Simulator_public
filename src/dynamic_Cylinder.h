//!  Cylinder Obstacle Derived Class =============================================================/
/*!
*   \details   Cylinder class derived from an Obstacle. Defines infinite long cylinders
*              in the direction set by P,Q.
*   \author    Jasmine Nguyen-Duc
*   \date      September 2022 
*   \version   1.42
=================================================================================================*/


#ifndef DYNCYLINDER_H
#define DYNCYLINDER_H

#include "cylinder.h"


/// @brief 
class Dynamic_Cylinder : public Obstacle
{
public:

    static int count;
    int id;
    Eigen::Vector3d P,Q;    /*!< Cilinder Axis reference Points, P should be the "center"       */
    Eigen::Vector3d D;      /*!< Pre-computed and normalized P - Q vector                       */
    double radius;          /*!< Radius of the cylinder                                         */
    bool swell;
    double volume_inc_perc;
    double min_radius;
    /*!
     *  \brief Default constructor. Does nothing
     */
 
    Dynamic_Cylinder();

    ~Dynamic_Cylinder();


    Dynamic_Cylinder(Eigen::Vector3d P_, Eigen::Vector3d Q_, double radius_,double volume_inc_perc_, bool swell_, int id_, double scale = 1):
    id(id_), P(P_*scale), Q(Q_*scale), radius(radius_*scale), swell(swell_), volume_inc_perc(volume_inc_perc_){
        D  = (Q_-P_).normalized();
        Q = P+D;

        min_radius = radius = radius_*scale;
        if (swell){
            radius = sqrt(1+volume_inc_perc)*radius;
        }

    }
    Dynamic_Cylinder(Dynamic_Cylinder const &dyn_cyl);

    /*! \fn  checkCollision
     *  \param walker, Walker instance in the simulation.
     *  \param 3d step. Is assumed to be normalized.
     *  \param step_length, length used as the maximum step collision distance.
     *  \param collision, Collision instance to save the collision (if any) details.
     *  \return true only if there was a Collision::hit status. \see Collision.
     *  \brief Basic collision function. Returns the if there was any collision on against the obstacle.
     */

    bool checkCollision(Walker &walker, Eigen::Vector3d const& step, double const& step_lenght, Collision &colision);

    /*! \fn  minDistance
     *  \param walker, Walker instance in the simulation.
     *  \brief Returns the minimum distance from the walker to the cylinder. Used to set the reachable
     *  cylinders that a given walker can reach.
     */
    double minDistance(Walker const& w) const;


private:

    /*! \fn  handleCollition
     *  \param walker, Walker instance in the simulation.
     *  \param collision, Collision instance to save all the information.
     *  \param step, step vector where to move.
     *  \brief Returns true if it was any analytical collision to the infinite plane
     */
    inline bool handleCollition(Walker& walker, Collision &colision, Eigen::Vector3d const& step,double& a,double& b, double& c,double& discr,double const& step_length);

};

#endif // DYN_CYLINDER_H
