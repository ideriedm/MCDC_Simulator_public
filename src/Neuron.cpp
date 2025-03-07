#include "doctest.h"

#include "Neuron.h"
#include "constants.h"
#include "Eigen/Dense"
#include <iostream>
#include "simerrno.h"
#include <cmath>
#include <typeinfo>

using namespace Eigen;
using namespace std;

int Neuron::nb_neurons = 0;
Neuron::Neuron()
{
    id = nb_neurons++;

    random_device dev;
    mt19937 rng(dev());

    constexpr uint8_t ub = 25; // upper bound for nb_dendrites
    constexpr uint8_t lb = 15; // lower bound for nb_dendrites

    uniform_int_distribution<mt19937::result_type> dist_dendrites(lb, ub);
    // Generate int number in [lb, ub]
    nb_dendrites = 20; // dist_dendrites(rng);

    // // Create a random span radius and set its value to this
    // generateSpanRadius();

    dendrites.clear();
}

Neuron::~Neuron()
{
    nb_neurons--;
}

Neuron::Neuron(vector<Dendrite> const &dendrites_, Dynamic_Sphere const &soma_) : Neuron()
{
    dendrites = dendrites_;
    assert(dendrites_.size() < std::numeric_limits<decltype(nb_dendrites)>::max());
    nb_dendrites = dendrites_.size();
    soma = soma_;
}

Neuron::Neuron(Vector3d const &soma_center, double const &soma_radius, int const &neuron_id) : Neuron()
{
    soma = Dynamic_Sphere(soma_center, soma_radius, neuron_id);
}

Neuron::Neuron(vector<Dendrite> const &dendrites_, Vector3d const &soma_center, double const &soma_radius = 10e-3) : Neuron()
{
    dendrites = dendrites_;
    nb_dendrites = dendrites_.size();
    soma = Dynamic_Sphere(soma_center, soma_radius, id);
}

Neuron::Neuron(Neuron const &neuron) : nb_dendrites(neuron.nb_dendrites), span_radius(neuron.span_radius), dendrites(neuron.dendrites), soma(neuron.soma)
{
    id = nb_neurons++;
}
double Neuron::minDistance(Walker const &walker) const
{
    vector<double> distances = Distances_to_Spheres(walker);
    double min = *min_element(begin(distances), end(distances));

    return min;
}

vector<double> Neuron::Distances_to_Spheres(Walker const &w) const
{

    Vector3d O;
    w.getVoxelPosition(O);
    return Distances_to_Spheres(O);
}

double Neuron::minDistance(Eigen::Vector3d const &pos) const
{

    vector<double> distances = Distances_to_Spheres(pos);
    double min = *min_element(begin(distances), end(distances));
    return min;
}

vector<double> Neuron::Distances_to_Spheres(Vector3d const &pos) const
{

    // First check distance to soma
    Vector3d m = pos - soma.center;
    double distance_to_sphere = m.norm() - soma.radius;
    vector<double> distances{distance_to_sphere};

    // Then iterate through dendrites
    for (uint8_t i = 0; i < dendrites.size(); ++i)
    {
        // Then iterate through the subbranches of each dendrite
        for (size_t j = 0; j < dendrites[i].subbranches.size(); j++)
        {
            // Then iterate through the spheres of a subbranch
            for (size_t k = 0; k < dendrites[i].subbranches[j].spheres.size(); k++)
            {
                Vector3d m = pos - dendrites[i].subbranches[j].spheres[k].center;
                double distance_to_sphere = abs(m.norm() - dendrites[i].subbranches[j].spheres[k].radius);
                distances.push_back(distance_to_sphere);
            }
        }
    }
    return distances;
}

TEST_CASE("isPosInsideNeuron")
{
    cout << "isPosInsideNeuron" << endl;
    Vector3d center(0.05, 0.05, 0.05);
    double radius_soma     = 10e-3;
    double radius_dendrite = 0.5e-3;
    Neuron neuron(center, radius_soma, 0);
    Dendrite dendrite;

    int branch_id = 0;
    vector<Dynamic_Sphere> spheres_list;
    for (size_t i = 0; i < 10; ++i)
    {
        Vector3d next_center(center[0] + radius_soma + i * radius_dendrite / 4, center[1], center[2]);
        Dynamic_Sphere sphere_to_add(next_center, radius_dendrite, 0, false, branch_id, i, 1);
        spheres_list.push_back(sphere_to_add);
    }

    Vector3d last_center(spheres_list[spheres_list.size()-1].center);
    Vector3d begin;
    vector <int> proximal_end = {};
    vector <int> distal_end   = {1, 2};
    Axon subbranch(branch_id, radius_dendrite, begin, begin, 0, false, 1, proximal_end, distal_end);
    subbranch.set_spheres(spheres_list);
    dendrite.add_subbranch(subbranch);

    // Add branching
    Vector3d branching_direction(1, 1, 0);
    branching_direction = branching_direction.normalized();

    for(size_t rep=0; rep < 2; rep++)
    {
        spheres_list.clear();
        for (size_t i = 0; i < 10; ++i)
        {
            Vector3d next_center(last_center[0] + i * radius_dendrite / 4 * branching_direction[0],
                                 last_center[1] + i * radius_dendrite / 4 * branching_direction[1],
                                 last_center[2] + i * radius_dendrite / 4 * branching_direction[2]);
            Dynamic_Sphere sphere_to_add(next_center, radius_dendrite, 0, false, branch_id + rep + 1, i, 1);
            spheres_list.push_back(sphere_to_add);
        }
        vector<int> proximal_end {0, int(2-rep)};
        vector<int> distal_end {int(3+rep), int(4+rep)};
        Axon subbranch_(branch_id + rep + 1, radius_dendrite, begin, begin, 0, false, 1, proximal_end, distal_end);
        subbranch_.set_spheres(spheres_list);
        dendrite.add_subbranch(subbranch_);
        branching_direction = - branching_direction;
    }
    neuron.add_dendrite(dendrite);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<double> udist(0,1);
    int dummy;
    vector<int> dummy1;
    SUBCASE("soma")
    {
        Vector3d somaCenter = neuron.soma.center;
        double somaRadius   = neuron.soma.radius;
        
        for(size_t nb_t=0; nb_t < 10; nb_t++)
        {
            // Check value inside soma
            double probaRadius = double(udist(gen));
            
            SUBCASE("inside soma")
            {      
                double theta = 2 * M_PI * udist(gen);
                double phi = acos(1 - 2 * udist(gen));
                double x = sin(phi) * cos(theta) * probaRadius * somaRadius + somaCenter[0];
                double y = sin(phi) * sin(theta) * probaRadius * somaRadius + somaCenter[1];
                double z = cos(phi) * probaRadius * somaRadius + somaCenter[2];
                Vector3d pos_temp = {x,y,z};

                bool inSoma_formula = pow(x-somaCenter[0], 2) + pow(y-somaCenter[1], 2) + pow(z-somaCenter[2], 2) <= somaRadius;
                bool inSoma_function= neuron.isPosInsideNeuron(pos_temp, barrier_tickness, false, dummy, dummy, dummy, dummy1);
                CHECK_EQ(inSoma_formula, inSoma_function);
            }
            SUBCASE("at soma surface")
            {
                double theta = 2 * M_PI * udist(gen);
                double phi = acos(1 - 2 * udist(gen));
                double x = sin(phi) * cos(theta) * somaRadius + somaCenter[0];
                double y = sin(phi) * sin(theta) * somaRadius + somaCenter[1];
                double z = cos(phi) * somaRadius + somaCenter[2];
                Vector3d pos_temp = {x,y,z};

                bool inSoma_formula = pow(x-somaCenter[0], 2) + pow(y-somaCenter[1], 2) + pow(z-somaCenter[2], 2) <= somaRadius;
                bool inSoma_function= neuron.isPosInsideNeuron(pos_temp, barrier_tickness, false, dummy, dummy, dummy, dummy1);
                CHECK_EQ(inSoma_formula, inSoma_function);
            }
        }     
    }
    SUBCASE("in dendrite")
    {
        for(size_t d=0; d < neuron.dendrites.size(); ++ d)
        {
            for(size_t b=0; b < neuron.dendrites[d].subbranches.size(); ++ b)
            {
                auto subbranch = neuron.dendrites[d].subbranches[b];
                for(size_t s=0; s < subbranch.spheres.size(); ++ s)
                {
                    Vector3d sphereCenter = subbranch.spheres[s].center;
                    double sphereRadius   = subbranch.spheres[s].radius;
                    SUBCASE("inside dendrite")
                    {
                        // Check value inside spheres
                        double probaRadius = double(udist(gen));
                        
                        double theta = 2 * M_PI * udist(gen);
                        double phi = acos(1 - 2 * udist(gen));
                        double x = sin(phi) * cos(theta) * probaRadius * sphereRadius + sphereCenter[0];
                        double y = sin(phi) * sin(theta) * probaRadius * sphereRadius + sphereCenter[1];
                        double z = cos(phi) * probaRadius * sphereRadius + sphereCenter[2];
                        Vector3d pos_temp = {x,y,z};

                        bool inSphere_formula = pow(x-sphereCenter[0], 2) + pow(y-sphereCenter[1], 2) + pow(z-sphereCenter[2], 2) <= sphereRadius;
                        bool inSphere_function= neuron.isPosInsideNeuron(pos_temp, barrier_tickness, false, dummy, dummy, dummy, dummy1);
                        CHECK_EQ(inSphere_formula, inSphere_function);
                    }
                    SUBCASE("at dendrite surface")
                    {
                        double theta = 2 * M_PI * udist(gen);
                        double phi = acos(1 - 2 * udist(gen));
                        double x = sin(phi) * cos(theta)  * sphereRadius + sphereCenter[0];
                        double y = sin(phi) * sin(theta)  * sphereRadius + sphereCenter[1];
                        double z = cos(phi) * sphereRadius + sphereCenter[2];
                        Vector3d pos_temp = {x,y,z};

                        bool inSphere_formula = pow(x-sphereCenter[0], 2) + pow(y-sphereCenter[1], 2) + pow(z-sphereCenter[2], 2) <= sphereRadius;
                        bool inSphere_function= neuron.isPosInsideNeuron(pos_temp, barrier_tickness, false, dummy, dummy, dummy, dummy1);
                        CHECK_EQ(inSphere_formula, inSphere_function);
                    }
                }
            }
        }
    }
}

bool Neuron::isPosInsideNeuron(Eigen::Vector3d const &position, double const &distance_to_be_inside, bool const &swell_, int &in_soma_index, int &in_dendrite_index, int &in_subbranch_index, vector<int> &in_sph_index)
{
    // Temporary variables to store the index before knowing if the walker is in the soma or in a dendrite
    int in_dendrite_index_tmp, in_subbranch_index_tmp, in_soma_index_tmp;
    in_dendrite_index_tmp = in_subbranch_index_tmp = in_soma_index_tmp = -1;
    vector<int> in_sph_index_tmp, in_sph_index_tmp_tmp;
    
    // When the walker is in several subbranches, we should find the sphere in which 
    // the walker is the closest from the center
    double min_dist = 10;
    vector<double> distances;
    // if position is in box with Dendrite inside
    // distance_to_be_inside = position.sphere.radius + constant
    // id of the dendrite. {} if not in neuron
    vector<int> part_id = isNearDendrite(position, distance_to_be_inside);
    if (part_id.size() > 0)
    {
        for (size_t p=0; p < part_id.size(); p++)
        {
            for (size_t b=0; b < dendrites[part_id[p]].subbranches.size(); b++)
            {
                if (dendrites[part_id[p]].subbranches[b].isPosInsideAxon(position, barrier_tickness, in_sph_index_tmp_tmp, distances))
                {
                    // Find the minimum distance walker-center for this subbranch
                    double min = *min_element(begin(distances), end(distances));
                    if(min < min_dist)
                    {
                        min_dist = min;
                        in_sph_index_tmp = in_sph_index_tmp_tmp;
                        in_dendrite_index_tmp  = part_id[p];
                        in_subbranch_index_tmp = b;
                    }
                }
            }
        }
    }

    if (isNearSoma(position, distance_to_be_inside))
    {
        if (soma.isInside(position, distance_to_be_inside))
            in_soma_index_tmp = 0;
    }
    in_dendrite_index  = in_dendrite_index_tmp;
    in_subbranch_index = in_subbranch_index_tmp;
    in_sph_index       = in_sph_index_tmp;
    in_soma_index      = in_soma_index_tmp; 

    // If we are both in soma & dendrite
    if(in_soma_index_tmp == 0 && in_dendrite_index_tmp >=0)
    {
        Dynamic_Sphere dendrite_sphere(dendrites[in_dendrite_index_tmp].subbranches[in_subbranch_index_tmp].spheres[in_sph_index[0]]);
        // We are closer to the dendrite
        if((soma.center - position).norm()/soma.radius > (dendrite_sphere.center - position).norm()/dendrite_sphere.radius)
            in_soma_index      = -1;
        // We are closer to the soma
        else
        {
            in_dendrite_index  = -1;
            in_subbranch_index = -1;
            in_sph_index.clear();
        }
    }

    if(in_soma_index_tmp == 0 || in_dendrite_index_tmp >=0)
        return true;

    return false;
}

TEST_CASE("isNearSoma")
{
    cout << "isNearSoma" << endl;
    Vector3d center(0.05, 0.05, 0.05);
    double radius = 10e-3;
    int sphere_id = 0;
    Neuron neuron(center, radius, sphere_id);

    SUBCASE("inside soma")
    {
        Vector3d position(center[0] + radius - 2*EPS_VAL, center[1], center[2]);
        CHECK(neuron.isNearSoma(position, EPS_VAL));
    }
    SUBCASE("outside soma")
    {
        Vector3d position(center[0] + radius + 2*EPS_VAL, center[1], center[2]);
        CHECK(!neuron.isNearSoma(position, EPS_VAL));
    }
}

bool Neuron::isNearSoma(Vector3d const &position, double const &distance_to_be_inside) const
{
    int count_isnear = 0;
    // Check soma box
    for (unsigned int axis = 0; axis < 3; ++axis)
    {
        if ((position[axis] >= soma.center[axis] - soma.radius - distance_to_be_inside) &&
            (position[axis] <= soma.center[axis] + soma.radius + distance_to_be_inside))
        {
            ++count_isnear;
        }
    }
    if (count_isnear == 3)
        return true;
    else
        return false;
}

TEST_CASE("isNearDendrite")
{
    cout << "isNearDendrite" << endl;
    Vector3d center(0.05, 0.05, 0.05);
    double radius_soma = 10e-3;
    double radius_dendrite = 0.5e-3;

    Neuron neuron(center, radius_soma, 0);
    Dendrite dendrite;

    int branch_id = 0;
    vector<Dynamic_Sphere> spheres_list;
    for (size_t i = 0; i < 10; ++i)
    {
        Vector3d next_center(center[0] + radius_soma + i * radius_dendrite / 4, center[1], center[2]);
        Dynamic_Sphere sphere_to_add(next_center, radius_dendrite, 0, false, branch_id, i, 1);
        spheres_list.push_back(sphere_to_add);
    }

    Vector3d begin;
    vector <int> proximal_end = {};
    vector <int> distal_end   = {1, 2};
    Axon subbranch(branch_id, radius_dendrite, begin, begin, 0, false, 1, proximal_end, distal_end);
    subbranch.set_spheres(spheres_list);
    dendrite.add_subbranch(subbranch);
    neuron.add_dendrite(dendrite);

    spheres_list.clear();
    for (size_t i = 0; i < 10; ++i)
    {
        Vector3d next_center(center[0] - radius_soma + i * radius_dendrite / 4, center[1], center[2]);
        Dynamic_Sphere sphere_to_add(next_center, radius_dendrite, 0, false, branch_id, i, 1);
        spheres_list.push_back(sphere_to_add);
    }

    Dendrite dendrite2;
    Axon subbranch2(branch_id + 1, radius_dendrite, begin, begin, 0, false, 1, proximal_end, distal_end);
    subbranch2.set_spheres(spheres_list);
    dendrite2.add_subbranch(subbranch2);
    neuron.add_dendrite(dendrite2);

    vector<int> dendrite_ids;
    SUBCASE("Soma center")
    {
        Vector3d position(center[0], center[1], center[2]);
        dendrite_ids = neuron.isNearDendrite(position, EPS_VAL);
        // Should not be near any dendrite
        CHECK(dendrite_ids.size() == 0);
    }
    SUBCASE("Begin of dendrite 0")
    {
        Vector3d position(center[0] + radius_soma + 2*EPS_VAL, center[1], center[2]);
        dendrite_ids = neuron.isNearDendrite(position, EPS_VAL);
        CHECK(dendrite_ids.size() == 1);
        CHECK_EQ(dendrite_ids[0], 0);
    }
    SUBCASE("End of dendrite 0")
    {
        auto sphere_list = neuron.dendrites[0].subbranches[0].spheres;
        Vector3d position(sphere_list[sphere_list.size()-1].center);
        dendrite_ids = neuron.isNearDendrite(position, EPS_VAL);
        CHECK(dendrite_ids.size() == 1);
        CHECK_EQ(dendrite_ids[0], 0);
    }
    SUBCASE("Begin: dendrite 1")
    {
        Vector3d position(center[0] - radius_soma - 2*EPS_VAL, center[1], center[2]);
        dendrite_ids = neuron.isNearDendrite(position, EPS_VAL);
        CHECK(dendrite_ids.size() == 1);
        CHECK_EQ(dendrite_ids[0], 1);
    }
    SUBCASE("End: dendrite 1")
    {
        auto sphere_list = neuron.dendrites[1].subbranches[0].spheres;
        Vector3d position(sphere_list[sphere_list.size()-1].center);
        dendrite_ids = neuron.isNearDendrite(position, EPS_VAL);
        CHECK(dendrite_ids.size() == 1);
        CHECK_EQ(dendrite_ids[0], 1);
    }
}

vector<int> Neuron::isNearDendrite(Vector3d const &position, double const &distance_to_be_inside) const
{
    // Check each dendrite's box
    int count_isnear = 0;
    vector<int> dendrite_ids;
    for (unsigned int i = 0; i < dendrites.size(); ++i)
    {
        count_isnear = 0;
        for (unsigned int axis = 0; axis < 3; ++axis)
        {
            Vector2d axis_limits = dendrites[i].projections.axon_projections[axis];
            if ((position[axis] >= axis_limits[0] - distance_to_be_inside) &&
                (position[axis] <= axis_limits[1] + distance_to_be_inside))
            {
                ++count_isnear;
            }
        }
        // Inside the box around dendrite
        if (count_isnear == 3)
            dendrite_ids.push_back(i);
    }
    if (dendrite_ids.size() > 0)
        return dendrite_ids;

    return dendrite_ids;
}



bool Neuron::checkCollision(Walker &walker, Vector3d const &step_dir, double const &step_lenght, Collision &colision)
{
    bool isColliding = false;

    // If in the soma, check Collision with soma
    if(walker.in_soma_index == 0)
    {
        Dynamic_Sphere* sphere(new Dynamic_Sphere(soma));
        isColliding = checkCollision_branching(walker, sphere, step_dir, step_lenght, colision);
    }
    // If in dendrite, check collision with the correct sphere
    else
    {

        int in_sph;
        if(walker.in_sph_index.size() > 0)
            in_sph = walker.in_sph_index[0];
        else
            // TODO [ines] : there is problem here sometimes, why ??
            assert(0);

        Dynamic_Sphere* sphere(new Dynamic_Sphere(dendrites[walker.in_dendrite_index].subbranches[walker.in_subbranch_index].spheres[in_sph]));
        isColliding = checkCollision_branching(walker, sphere, step_dir, step_lenght, colision);
    }

    
 
    if (!isPosInsideNeuron(colision.colision_point, barrier_tickness, false, walker.in_soma_index, walker.in_dendrite_index, walker.in_subbranch_index, walker.in_sph_index))
    {
        walker.location = Walker::extra;
        cout << "extra" << endl;
    }


    // TODO [ines] : do the extracellular collisions
    // if (walker.location == Walker::extra)
    // {
    //     // Check if collision with soma. Can be internal or external collisions.
    //     if (soma.checkCollision(walker, step_dir, step_lenght, colision))
    //     {
    //         return true;
    //     }
    //     // Check if collision with dendrites from outside.
    //     for (uint8_t i = 0; i < dendrites.size(); ++i)
    //     {
    //         if (dendrites[i].checkCollision(walker, step_dir, step_lenght, colision))
    //             return true;
    //     }
    // }

    return isColliding;
}


TEST_CASE("checkCollision_branching")
{
    cout << "checkCollision_branching" << endl;
    Vector3d center(0.05, 0.05, 0.05);
    double radius_soma = 10e-3;
    double radius_dendrite = 0.5e-3;

    vector<Dynamic_Sphere> spheres_list;
    for (size_t i = 0; i < 10; ++i)
    {
        Vector3d next_center(center[0] + radius_soma + i * radius_dendrite / 4, center[1], center[2]);
        spheres_list.push_back(Dynamic_Sphere(next_center, radius_dendrite, i));
    }
    Neuron neuron(center, radius_soma, 0);
    Dendrite dendrite;
    int branch_id = 0;
    Vector3d begin;
    vector<int> proximal_end = {};
    vector<int> distal_end = {1, 2};
    Axon subbranch(branch_id, radius_dendrite, begin, begin, 0, false, 1, proximal_end, distal_end);
    subbranch.set_spheres(spheres_list);
    dendrite.add_subbranch(subbranch);
    neuron.add_dendrite(dendrite);

    Collision colision;
    colision.type = Collision::null;
    colision.t = INFINITY_VALUE;

    SUBCASE("no collision")
    {
        Vector3d direction(1, 0, 0);
        double step_length = 5e-4;

        Walker w;
        w.setInitialPosition(center[0] + radius_soma, center[1], center[2]);
        w.location = w.initial_location = Walker::intra;
        w.in_soma_index = 0;

        bool collided = neuron.checkCollision_branching(w, &neuron.dendrites[0].subbranches[0].spheres[0], direction, step_length, colision);
        // The collision with the current sphere is considered
        CHECK(collided);
        CHECK_EQ(colision.type, Collision::hit);
        CHECK_EQ(colision.t, radius_dendrite);
    }
    SUBCASE("trivial bounce cycle")
    {
        Vector3d direction(0, 1, 0);
        double step_length = 6e-4;

        Walker w;
        w.setInitialPosition(center[0] + radius_soma, center[1], center[2]);
        w.location = w.initial_location = Walker::intra;
        w.in_soma_index = 0;

        SUBCASE("trivial hit")
        {
            bool collided = neuron.checkCollision_branching(w, &neuron.dendrites[0].subbranches[0].spheres[0], direction, step_length, colision);
            CHECK(collided);
            CHECK_EQ(colision.type, Collision::hit);
            CHECK_EQ(colision.t, doctest::Approx(radius_dendrite));
            CHECK_EQ(colision.bounced_direction, -direction);
            CHECK_EQ(colision.colision_point, w.pos_v + radius_dendrite * direction);
            CHECK_EQ(w.status, Walker::free);
        }
        SUBCASE("trivial bounce")
        {
            w.status = Walker::bouncing;
            w.setRealPosition(w.pos_r + radius_dendrite * direction);
            w.setVoxelPosition(w.pos_v + radius_dendrite * direction);

            colision.t = radius_dendrite;
            colision.bounced_direction = -direction;
            colision.colision_point = w.pos_v;

            bool collided = neuron.checkCollision_branching(w, &neuron.dendrites[0].subbranches[0].spheres[0], direction, step_length - radius_dendrite, colision);

            CHECK(!collided);
            CHECK_EQ(colision.type, Collision::null);
            CHECK_EQ(colision.t, doctest::Approx(radius_dendrite));
            CHECK_EQ(colision.bounced_direction, -direction);
            CHECK_EQ(colision.colision_point, w.pos_v);
            CHECK_EQ(w.status, Walker::bouncing);
        }
    }
    // TODO: [ines] more complex cases
}

bool Neuron::checkCollision_branching(Walker &walker, Dynamic_Sphere* const& sphere, Vector3d const &step, double const &step_lenght, Collision &colision)
{

    string message;
    Vector3d O;
    walker.getVoxelPosition(O);

    // distances to intersections
    std::vector<double> dist_intersections;
    // values indicating whether the walker is inside or outside a sphere
    std::vector<double> cs;

    std::vector<int> sph_ids;
    std::vector<double> all_cs;


    // distances to collision
    double t1;
    double t2;
    double c;
    // Find the intersection with the sphere in which the walker is
    bool intersect = intersection_sphere_vector(t1, t2, *sphere, step, step_lenght, O, c);

    // Technically, it should always intersect (TODO: [ines] check)
    if (intersect)
    {
        all_cs.push_back(c);
        
        // if the collision are too close or negative.
        if (Walker::bouncing)
        {
            if (t1 >= EPS_VAL)
            {
                dist_intersections.push_back(t1);
                cs.push_back(c);
            }
            if (t2 >= EPS_VAL)
            {
                dist_intersections.push_back(t2);
                cs.push_back(c);
            }
        }
        else
        {
            if (t1 >= 0)
            {
                dist_intersections.push_back(t1);
                cs.push_back(c);
            }
            if (t2 >= 0)
            {
                dist_intersections.push_back(t2);
                cs.push_back(c);
            }
        }    
    }
    else
    {
        double dist_to_collision;
        if(t1 >= 0)
            dist_to_collision = t1;
        else if (t2 >= 0)
            dist_to_collision = t2;
        colision.type = Collision::hit;
        colision.t = fmin(dist_to_collision, step_lenght);
        colision.colision_point = walker.pos_v + colision.t * step;
        Vector3d normal = (colision.colision_point - sphere->center).normalized();
        Vector3d temp_step = step;
        elasticBounceAgainsPlane_intra(walker.pos_v, normal, colision.t, temp_step);
        colision.bounced_direction = temp_step.normalized();
        return true;
    }

    // If they are some intersections to consider
    if (dist_intersections.size() > 0)
    {
        // Set the distance to the collision point
        double dist_to_collision = dist_intersections[0];
        colision.t = fmin(dist_to_collision, step_lenght);
        colision.colision_point = walker.pos_v + colision.t * step;
        
        // If the distance is <= step_length => there is a collision
        if (dist_to_collision <= step_lenght + barrier_tickness)
        {
        
            // All collisions are considered as hit 
            // (walls between intersecting spheres as well as outer boundary)
            colision.type = Collision::hit;
            colision.rn = cs[0];

            // if extra, cannot hit sphere from inside
            if (walker.initial_location== Walker::extra)
            {
                if (cs[0] < -1e-15)
                    colision.col_location = Collision::inside;
                else
                    colision.col_location = Collision::outside;
                
            }
            else
                colision.col_location = Collision::inside;
            
            // Check if the collision point is part of the neighboring spheres
            bool inner_collision = false;
            for(size_t i=0; i < sphere->neighboring_spheres.size(); ++i)
                inner_collision = inner_collision || sphere->neighboring_spheres[i]->isInside(colision.colision_point, EPS_VAL);
            
            // If yes, it is a collision without reflection (the walker keeps the same direction)
            if(inner_collision)
            {
                colision.bounced_direction = step;
                return true;
            }
            // If not, it is a "real collision" with the outer boundary => reflection
            else
            {
                //WARNING: Cuidar este patch
                // Implementa Percolacion
                if(percolation > 0.0)
                {
                    double _percolation_ (double(rand())/RAND_MAX);

                    if(percolation - _percolation_ > EPS_VAL )
                    {
                        count_perc_crossings++;
                        return false;
                    }
                }

                Vector3d normal = (colision.colision_point - sphere->center).normalized();
                Vector3d temp_step = step;
                elasticBounceAgainsPlane_intra(walker.pos_v, normal, colision.t, temp_step);
                colision.bounced_direction = temp_step.normalized();
                return true;
            }

            
        }
        // if the distance is > step_length => there is no collision
        else
        {
            colision.type = Collision::null;
            return false;
        }
    }
    else
    {
        // assert(0);
        colision.type = Collision::null;
        return false;
    }

}


// vector<int> Neuron::closest_subbranch(Vector3d const &position, int const &dendrite_id, int const &subbranch_id, double const &step_length)
// {
//     const auto &subbranch = dendrites[dendrite_id].subbranches[subbranch_id];
//     int nb_subbranches = dendrites[dendrite_id].subbranches.size();
//     int size_subbranch = subbranch.spheres.size() - 1;
//     double eps = 0.0005;
//     double distance_to_proximal_end = (subbranch.spheres[0].center - position).norm() - eps;
//     double distance_to_distal_end = (subbranch.spheres[size_subbranch].center - position).norm() - eps;

//     if (distance_to_proximal_end <= step_length)
//     {
//         return subbranch.proximal_branching;
//     }
//     else if (distance_to_distal_end <= step_length)
//     {
//         // The subbranch id starts at 1 but the indices in c++ start at 0
//         vector<int> distal_branching = subbranch.distal_branching;

//         // Distal end of the neuron
//         if ((distal_branching[0] >= nb_subbranches) || (distal_branching[1] >= nb_subbranches))
//             return {-2};

//         return distal_branching;
//     }
//     else
//         return {-1};
// }

// vector<int> Neuron::close_dendrites_from_soma(Vector3d const &position, double const &step_length)
// {
//     vector<int> closer_dendrite;
//     for (size_t i = 0; i < dendrites.size(); ++i)
//     {
//         // TODO : there was a segfault here once ... [ines]
//         double distance_to_axon = (position - dendrites[i].subbranches[0].spheres[0].center).norm();
//         if (distance_to_axon <= step_length + dendrites[i].subbranches[0].spheres[0].radius)
//         {
//             closer_dendrite.push_back(i);
//         }
//     }
//     return closer_dendrite;
// }

// TODO: do other cases
TEST_CASE("intersection_sphere_vector")
{
    cout << "intersection_sphere_vector" << endl;
    Vector3d center(0.05, 0.05, 0.05);
    double radius = 5e-4;
    int sphere_id = 0;
    Dynamic_Sphere sphere(center, radius, sphere_id);

    Vector3d direction(1, 0, 0);
    double step_length = 5e-4;

    Vector3d traj_origin(0.05, 0.05, 0.05);

    double intercept1, intercept2, c;
    CHECK(Neuron::intersection_sphere_vector(intercept1, intercept2, sphere, direction, step_length, traj_origin, c));

    CHECK_EQ(intercept1, doctest::Approx(radius));
    CHECK_EQ(intercept2, doctest::Approx(-radius));
    // CHECK_EQ(c, ); // TODO: [ines]

    traj_origin = {0.04, 0.05, 0.05};
    CHECK(Neuron::intersection_sphere_vector(intercept1, intercept2, sphere, direction, step_length, traj_origin, c));
    CHECK_EQ(intercept1, doctest::Approx(0.01 + radius));
    CHECK_EQ(intercept2, doctest::Approx(0.01 - radius));

    traj_origin = {0.0, 0.05, 0.05};
    CHECK(Neuron::intersection_sphere_vector(intercept1, intercept2, sphere, direction, step_length, traj_origin, c));
    CHECK_EQ(intercept1, doctest::Approx(0.05 + radius));
    CHECK_EQ(intercept2, doctest::Approx(0.05 - radius));

}

bool Neuron::intersection_sphere_vector(double &intercept1, double &intercept2, Dynamic_Sphere const &sphere, Vector3d const &step_dir, double const &step_length, Vector3d const &traj_origin, double &c)
{
    Vector3d m = traj_origin - sphere.center;
    double rad = sphere.radius;

    double a = 1;
    double b = m.dot(step_dir);
    c = m.dot(m) - rad * rad;

    double discr = b * b - a * c;

    if (discr < 0.0)
        return false;

    intercept1 = (-b + sqrt(discr)) / (a);
    intercept2 = (-b - sqrt(discr)) / (a);

    return true;
}

void Neuron::set_dendrites(std::vector<Dendrite> &dendrites_to_add)
{
    if (dendrites_to_add.size() != 0)
        for(size_t i=0; i < dendrites_to_add.size(); ++i)
            add_dendrite(dendrites_to_add[i]);
}

void Neuron::add_dendrite(Dendrite &dendrite_to_add)
{
    dendrite_to_add.add_projection();
    dendrites.push_back(dendrite_to_add);
}

void Neuron::generateSpanRadius(double const &lower_bound, double const &upper_bound)
{
    random_device dev;
    mt19937 rng(dev());
    uniform_real_distribution<double> dist_span_radius(lower_bound, upper_bound);
    // Generate int number in [lb, ub], in [mm]
    span_radius = dist_span_radius(rng);
}

vector<double> Neuron::get_Volume() const
{
    double VolumeSoma = 0;
    double VolumeDendrites = 0;
    // Calculate the volume of the soma
    VolumeSoma += 4.0 / 3.0 * M_PI * pow(soma.radius, 3);

    // Calculate the cylindrical volume of each dendrite
    for (uint8_t j = 0; j < dendrites.size(); j++)
    {
        VolumeDendrites += dendrites[j].volumeDendrite();
    }

    return {VolumeSoma, VolumeDendrites};
}