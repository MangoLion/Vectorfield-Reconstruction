#include <ctype.h>
#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include "../DataReaders/Reader.h"
#include "../structures.h"
#include "../globals.h"
#include "../Unused/Skeleton.h"
#include <math.h>
#include "vtkPoints.h"
#include "../PointLocator/kdTreeHelper.h"


/**
* Return difference between 2 3d angles, value is between 0 and PI
*/
double getAbsoluteDiff3Angles(float x1, float y1, float z1, float x2, float y2, float z2)
{
    float dot = x1 * x2 + y1 * y2 + z1 * z2; //between [x1, y1, z1] and [x2, y2, z2]
    float lenSq1 = x1 * x1 + y1 * y1 + z1 * z1;
    float lenSq2 = x2 * x2 + y2 * y2 + z2 * z2;
    //TODO: check range of input, probably slightly > 1 or < -1
    float input = dot / sqrt(lenSq1 * lenSq2);
    if (input > 1 || input < -1) {
        //std::cout << "Error: getAbsoluteDiff3Angles out of range!"<< input << std::endl;
        if (input > 1)
            input = 1;
        else
            input = -1;
    }

    float angle = acos(input);
    return angle;
}

/**
* Returns dot product between 2 3d angles
*/
float dot_product(float v1[3], float v2[3])
{
    float product = 0;

    // Loop for calculate cot product
    for (int i = 0; i < 3; i++)

        product = product + v1[i] * v2[i];
    return product;
}

/**
* Returns the standard deviation value for the current point p in the original vectorfield. 
* We compute the standard devation of all angles that are neighbors to the closest segment to p
*/
float recon_get_stdev(double p[3], float radar)
{
    radar = 10000;
    float xx = p[0], yy = p[1], zz = p[2];
    std::vector<node*> results;

    auto closestID = pointLocatorSL.KdTreeFindClosestPoint(p);
    if (closestID == -1)
        return 0;

    segment* s_closest = all_segments[closestID];
    node* n_middle = s_closest->middle;
    float min_dis = 10000000;

    for (int nn = -1; nn <= 1; nn++)
    {
        int next_index = s_closest->global_index + nn;
        if (next_index < 0)
        {
            nn++;
            next_index = s_closest->global_index + nn;
        }
        if (next_index > total_nodes || all_segments[next_index]->streamline_index != s_closest->streamline_index)
            continue;
        segment* s_selected = all_segments[next_index];

        for (int i = 0; i < s_selected->neighbors.size() + 1; i++)
        {

            node* n2;
            if (i == 0)
                n2 = s_selected->middle;
            else
                n2 = s_selected->neighbors[i - 1]->seg->middle;

            float tdis = abs(xx - n2->x) + abs(yy - n2->y) + abs(zz - n2->z);
            if (tdis > radar && i == 0)
                radar = tdis;

            if (tdis < radar)
            {
                tdis = sqrt((xx - n2->x) * (xx - n2->x) + (yy - n2->y) * (yy - n2->y) + (zz - n2->z) * (zz - n2->z));

                tdis = radar - tdis;
                n2->dis = tdis;

                if (results.size() > 0 && results[0]->dis > n2->dis)
                {
                    results.push_back(n2);
                }
                else if (results.size() == 0 || results[results.size() - 1]->dis < n2->dis)
                {
                    results.push_back(n2);
                }
                else
                {
                    for (int i = 0; i < results.size() - 1; i++)
                    {
                        node* n3 = results[i],
                            * n4 = results[i + 1];
                       
                        if (n3->dis < tdis && n4->dis > tdis)
                        {
                            results.push_back(n2);
                            break;
                        }
                    }
                }
            }
        }
    }

    float sum_change = 0;
    int valid_num = 0;
    float* values = new float[results.size()];
    for (int i = 0; i < results.size(); i++)
    {
        node* n = results.at(i);
        if (i > 0)
        {
            float angle_diff = getAbsoluteDiff3Angles(n->vx, n->vy, n->vz, n_middle->vx, n_middle->vy, n_middle->vz);
            //TODO: remove this once getAbsoluteDiff3Angles random range errors is fixed
            if (!isnan(angle_diff))
            {
                sum_change += angle_diff;
                values[valid_num] = angle_diff;
                valid_num++;
            }
            else
            {
            }

        }
    }

    float stdev = 0;

    
    if (valid_num == 0)
    {
        //stdev is 0 if no segments are in radar range
        //TODO: increase radar range if this happens?
    }
    else
    {
        float mean = sum_change / valid_num;
        float var = 0;
        for (int i = 0; i < valid_num; i++)
        {
            float v = values[i] - mean;
            var += v * v;
        }
        var /= valid_num;
        stdev = sqrt(var);
    }
    delete[] values;

    return stdev;
}



/**
 * @brief reconstruct the vector value of the point (xx,yy) on the vector field by using the Selective Weighed Sum method, this method provides the most reliable reconstruction
 * it first uses the nearest streamline node's vector value as the base, then it finds the next closest node and compare the vector value, if the difference is below the average threshold then
 * it computes the weighted sum between two nodes, if the diff is too large then it discard the second node. The process is repeated untill all nodes within radar range is processed
 *
 * @param xx x coord of the point on the vector field
 * @param yy y coord of the point on the vector field
 * @param vx the resulting vx value after reconstruction
 * @param vy the resulting vy value after reconstruction
 * @param radar the radar diameter to scan for surrounding streamline nodes
 * @return int how many streamline nodes where found within the radar range, used for recursion. If the result is too few then increase radar range and vice versa
 */
float interpolate_streamline_selective_weighted_sum_stdev(float& stdev, float xx, float yy, float zz, float& vx, float& vy, float& vz)
{
    // map global parameters to local
    float radar = P_RADAR ,
        thresholdAngle = P_THRESHOLD_ANGLE,
        thresholdVelocity = P_THRESHOLD_VELOCITY;

    bool doNormalize = P_NORMALIZE, 
        doUseSegments = P_USE_SEGMENTS;


    float total_dis = 0;
    vx = 0;
    vy = 0;
    vz = 0;
    std::vector<node*> results;
    double pt[3] = { xx, yy, zz };

    // NO SEGMENTS----------------------------------
    if (!doUseSegments)
    {
        auto nearbyIDs = pointLocatorSL.KdTreeFindPointsWithinRadius(radar, pt);
        for (int n = 0; n < nearbyIDs->GetNumberOfIds(); n++)
        {
            auto ID = nearbyIDs->GetId(n);
            segment* seg = all_segments[ID];
            node* n2 = seg->middle;
            float tdis = sqrt((xx - n2->x) * (xx - n2->x) + (yy - n2->y) * (yy - n2->y) + (zz - n2->z) * (zz - n2->z));
            if (tdis >= radar)
                continue;
            tdis = radar - tdis;
            n2->dis = tdis;
            total_dis += tdis;

            if (results.size() > 0 && results[0]->dis > n2->dis)
            {
                results.insert(results.begin(), n2);
            }
            else if (results.size() == 0 || results[results.size() - 1]->dis < n2->dis)
            {
                results.push_back(n2);
            }
            else
            {
                for (int i = 0; i < results.size() - 1; i++)
                {
                    node* n3 = results[i],
                        * n4 = results[i + 1];
                    if (n3->dis < tdis && n4->dis > tdis)
                    {
                        results.insert(results.begin() + i, n2);
                        break;
                    }
                }
            }
        }
    }

    // IF USE SEGMENTS----------------------------------
    if (doUseSegments)
    {
        //std::cerr << xx << ", " << yy << ", " << zz << std::endl;
        auto closestID = pointLocatorSL.KdTreeFindClosestPoint(pt);
        if (closestID == -1)
            return 0;

        segment* s_closest = all_segments[closestID];
        float min_dis = 10000000;
        //std::cerr << s_closest->neighbors.size() << std::endl;
        for (int nn = -1; nn <= 1; nn++)
        {
            int next_index = s_closest->global_index + nn;
            if (next_index < 0)
            {
                nn++;
                next_index = s_closest->global_index + nn;
            }
            if (next_index > total_nodes || all_segments[next_index]->streamline_index != s_closest->streamline_index)
                continue;
            segment* s_selected = all_segments[next_index];
            // std::cerr << nn;
            for (int i = 0; i < s_selected->neighbors.size() + 1; i++)
            {
                node* n2;
                if (i == 0)
                    n2 = s_selected->middle;
                else
                    n2 = s_selected->neighbors[i - 1]->seg->middle;

                float tdis = abs(xx - n2->x) + abs(yy - n2->y) + abs(zz - n2->z);
                if (tdis > radar && i == 0)
                    radar = tdis;

                if (tdis < radar)
                {
                    tdis = sqrt((xx - n2->x) * (xx - n2->x) + (yy - n2->y) * (yy - n2->y) + (zz - n2->z) * (zz - n2->z));
                    //if (tdis >= radar)
                    // continue;
                    tdis = radar - tdis;
                    n2->dis = tdis;
                    total_dis += tdis;

                    if (results.size() > 0 && results[0]->dis > n2->dis)
                    {
                        results.insert(results.begin(), n2);
                    }
                    else if (results.size() == 0 || results[results.size() - 1]->dis < n2->dis)
                    {
                        results.push_back(n2);
                    }
                    else
                    {
                        for (int i = 0; i < results.size() - 1; i++)
                        {
                            node* n3 = results[i],
                                * n4 = results[i + 1];
                            if (n3->dis < tdis && n4->dis > tdis)
                            {
                                results.insert(results.begin() + i, n2);
                                break;
                            }
                        }
                    }
                }
            }
        }
    }

    ////////////////////////

    //Take into account neighbor reconstructed points
    if (pointLocatorStdev.ready && RECON_USE_NEIGHBORS)
    {

        vtkSmartPointer<vtkIdList>nearbyIDs;
        //#pragma omp critical
        bool go = true;
        do {
            try {
                //#pragma omp critical
                nearbyIDs = pointLocatorStdev.KdTreeFindClosestKPoints(RECON_K_NEIGHBORS, pt);
            }
            catch (...) {
                std::cout << "ACCESS VIOLATION!!!!" << std::endl;
                std::cerr << "ACCESS VIOLATION!!!!" << std::endl;
                go = false;
            }
        } while (!go);
        //std::cout << nearbyIDs->GetNumberOfIds() << ", ";
        int ccc = 0;
        //std::cout << results.size()<<", ";
        for (int n = 0; n < nearbyIDs->GetNumberOfIds(); n++)
        {
            auto ID = nearbyIDs->GetId(n);
            vtkIdType idx;
#pragma omp critical
            idx = pointLocatorStdev.idMap[ID];

            auto vel = pointLocatorStdev.velocity->GetTuple(idx);
            double* pt = pointLocatorStdev.pointTree->GetLocatorPoints()->GetPoint(ID);
            node* nn = new node(pt[0], pt[1], pt[2], vel[0], vel[1], vel[2]);
            nn->temp = true;
            float tdis = sqrt((xx - nn->x) * (xx - nn->x) + (yy - nn->y) * (yy - nn->y) + (zz - nn->z) * (zz - nn->z));
            if (tdis > radar) {
                delete nn;
                continue;
            }

            //TODO: ENABLE THIS FOR HEATMAP
           // int heatmapIdx = idx;
            //pointLocatorStdev.heatmap->SetComponent(heatmapIdx, 0, pointLocatorStdev.heatmap->GetTuple(heatmapIdx)[0] + 1);

            tdis = radar - tdis;

            nn->dis = tdis;
            total_dis += tdis;
            ccc++;
            delete nn;
            //results.push_back(nn);
        }
    }
    ////////////////////////

    float sum_change = 0;
    int valid_num = 0;
    float* values = new float[results.size()];
    float avg_vx = 0, avg_vy = 0, avg_vz = 0;

    //calc avg angle
    for (int i = 0; i < results.size(); i++)
    {
        node* n = results.at(i);
        //oat angle_diff = getAbsoluteDiff3Angles(n->vx, n->vy, n->vz, vx, vy, vz);
        float weight = n->dis / total_dis;
        avg_vx += n->vx * weight;
        avg_vy += n->vy * weight;
        avg_vz += n->vz * weight;
    }


    for (int i = 0; i < results.size(); i++)
    {
        node* n = results.at(i);
        float weight = n->dis / total_dis;

        //always include closest point
        if (i > 0)
        {
            float angle_diff = getAbsoluteDiff3Angles(n->vx, n->vy, n->vz, avg_vx, avg_vy, avg_vz);
            if (isnan(angle_diff) || angle_diff > RECON_EXCLUDE_THRESHOLD) {
                //cout << "excluded" << endl;
                RECON_EXCLUDED++;
                continue;
            }

            angle_diff_avg += angle_diff;
            angle_diff_c++;
            sum_change += angle_diff;
            values[valid_num] = angle_diff;
            valid_num++;

        }
        vx += n->vx * weight;
        vy += n->vy * weight;
        vz += n->vz * weight;
        if (n->temp)
            delete n;
    }


    stdev = 0;
    if (valid_num == 0)
    {
        //std::cerr << "NO VALID SEGMENTS!" << std::endl;
    }
    else
    {
        float mean = sum_change / valid_num;
        float var = 0;
        for (int i = 0; i < valid_num; i++)
        {
            float v = values[i] - mean;
            var += v * v;
        }
        var /= valid_num;
        stdev = sqrt(var);
    }
    delete[] values;
    //normalize
    if (doNormalize)
    {
        float mag = sqrt(vx * vx + vy * vy + vz * vz);
        vx /= mag;
        vy /= mag;
        vz /= mag;
    }

    //----------------------------------
    if (results.size() == 0)
    {
        //std::cerr << "Increase radar";
        auto orgRadar = radar;
        P_RADAR *= 1.5;
        auto result = interpolate_streamline_selective_weighted_sum_stdev(stdev, xx, yy, zz, vx, vy, vz);
        P_RADAR = orgRadar;
        return result;
    }

    //std::cout << std::endl;
    return stdev; //results.size();
}

/**
 * @brief reconstruct the vector value of the point (xx,yy) on the vector field by using the Selective Weighed Sum method, this method provides the most reliable reconstruction
 * it first uses the nearest streamline node's vector value as the base, then it finds the next closest node and compare the vector value, if the difference is below the average threshold then
 * it computes the weighted sum between two nodes, if the diff is too large then it discard the second node. The process is repeated untill all nodes within radar range is processed
 *
 * @param xx x coord of the point on the vector field
 * @param yy y coord of the point on the vector field
 * @param vx the resulting vx value after reconstruction
 * @param vy the resulting vy value after reconstruction
 * @param radar the radar diameter to scan for surrounding streamline nodes
 * @return int how many streamline nodes where found within the radar range, used for recursion. If the result is too few then increase radar range and vice versa
 */
int interpolate_streamline_selective_weighted_sum(float xx, float yy, float zz, float& vx, float& vy, float& vz)
{
    // map global parameters to local
    float radar = P_RADAR,
        thresholdAngle = P_THRESHOLD_ANGLE,
        thresholdVelocity = P_THRESHOLD_VELOCITY;

    bool doNormalize = P_NORMALIZE,
        doUseSegments = P_USE_SEGMENTS;

    float total_dis = 0;
    vx = 0;
    vy = 0;
    vz = 0;
    std::vector<node*> results;
    double pt[3] = { xx, yy, zz };

    if (!doUseSegments)
    {
        auto nearbyIDs = pointLocatorSL.KdTreeFindPointsWithinRadius(radar, pt);
        for (int n = 0; n < nearbyIDs->GetNumberOfIds(); n++)
        {
            auto ID = nearbyIDs->GetId(n);
            segment* seg = all_segments[ID];
            node* n2 = seg->middle;
            float tdis = sqrt((xx - n2->x) * (xx - n2->x) + (yy - n2->y) * (yy - n2->y) + (zz - n2->z) * (zz - n2->z));
            if (tdis >= radar)
                continue;
            tdis = radar - tdis;
            n2->dis = tdis;
            total_dis += tdis;

            if (results.size() > 0 && results[0]->dis > n2->dis)
            {
                results.insert(results.begin(), n2);
            }
            else if (results.size() == 0 || results[results.size() - 1]->dis < n2->dis)
            {
                results.push_back(n2);
            }
            else
            {
                for (int i = 0; i < results.size() - 1; i++)
                {
                    node* n3 = results[i],
                        * n4 = results[i + 1];
                    if (n3->dis < tdis && n4->dis > tdis)
                    {
                        results.insert(results.begin() + i, n2);
                        break;
                    }
                }
            }
        }
    }

    if (doUseSegments)
    {
        //std::cerr << xx << ", " << yy << ", " << zz << std::endl;
        auto closestID = pointLocatorSL.KdTreeFindClosestPoint(pt);
        if (closestID == -1)
            return 0;

        segment* s_closest = all_segments[closestID];
        float min_dis = 10000000;
        for (int nn = -1; nn <= 1; nn++)
        {
            int next_index = s_closest->global_index + nn;
            if (next_index < 0)
            {
                nn++;
                next_index = s_closest->global_index + nn;
            }
            if (next_index > total_nodes || all_segments[next_index]->streamline_index != s_closest->streamline_index)
                continue;
            segment* s_selected = all_segments[next_index];
            // std::cerr << nn;
            for (int i = 0; i < s_selected->neighbors.size() + 1; i++)
            {

                node* n2;
                if (i == 0)
                    n2 = s_selected->middle;
                else
                    n2 = s_selected->neighbors[i - 1]->seg->middle;

                float tdis = abs(xx - n2->x) + abs(yy - n2->y) + abs(zz - n2->z);
                if (tdis > radar && i == 0)
                    radar = tdis;

                if (tdis < radar)
                {
                    tdis = sqrt((xx - n2->x) * (xx - n2->x) + (yy - n2->y) * (yy - n2->y) + (zz - n2->z) * (zz - n2->z));
                    //if (tdis >= radar)
                    // continue;
                    tdis = radar - tdis;
                    n2->dis = tdis;
                    total_dis += tdis;

                    if (results.size() > 0 && results[0]->dis > n2->dis)
                    {
                        results.insert(results.begin(), n2);
                    }
                    else if (results.size() == 0 || results[results.size() - 1]->dis < n2->dis)
                    {
                        results.push_back(n2);
                    }
                    else
                    {
                        for (int i = 0; i < results.size() - 1; i++)
                        {
                            //std::cout << "a "<<i<<"/"<< results.size()<<std::endl;
                            node* n3 = results[i],
                                * n4 = results[i + 1];
                            //std::cout << "b";
                            if (n3->dis < tdis && n4->dis > tdis)
                            {
                                results.insert(results.begin() + i, n2);
                                break;
                            }
                        }
                    }
                }
            }
        }
    }

    for (int i = 0; i < results.size(); i++)
    {
        node* n = results.at(i);
        float weight = n->dis / total_dis;
        if (i > 0)
        {
            float angle_diff = getAbsoluteDiff3Angles(n->vx, n->vy, n->vz, vx, vy, vz);

            if (!isnan(angle_diff))
            {
                angle_diff_avg += angle_diff;
                angle_diff_c++;
            }
            else
            {
                //std::cerr << "error computing " << n->vx << ", " << n->vy << ", " << n->vz << ", " << vx << ", " << vy << ", " << vz << std::endl;
            }

            if (thresholdAngle > 0)
                if (angle_diff > thresholdAngle || isnan(angle_diff))
                {
                    continue;
                }
        }
        vx += n->vx * weight;
        vy += n->vy * weight;
        vz += n->vz * weight;
        {
            float mag = sqrt(vx * vx + vy * vy + vz * vz);
            vx /= mag;
            vy /= mag;
            vz /= mag;
        }
    }

    if (results.size() == 0)
    {
        auto orgRadar = radar;
        P_RADAR *= 1.5;
        auto result = interpolate_streamline_selective_weighted_sum(xx, yy, zz, vx, vy, vz);
        P_RADAR = orgRadar;
        return result;
    }

    return results.size();
}