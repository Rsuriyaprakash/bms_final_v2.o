/**
 * @file graph_management.h
 * @author Suriya Prakash R
 * @version 2.0.0
 * @brief Hospital graph and route-management APIs.
 */

#ifndef BMS_GRAPH_MANAGEMENT_H
#define BMS_GRAPH_MANAGEMENT_H

#include <stdbool.h>
#include <stdint.h>

#include "common.h"

#ifdef __cplusplus
extern "C" {
#endif

#define BMS_GRAPH_NO_ROUTE_DISTANCE (UINT32_MAX)

typedef struct
{
    bool occupied;
    BmsHospital_t hospital;
} BmsGraphHospitalNode_t;

typedef BmsStatus_t (*BmsGraphVisit_t)(const BmsHospital_t *hospital,
                                      void *context);

typedef struct
{
    BmsGraphHospitalNode_t nodes[BMS_GRAPH_MAX_HOSPITALS];
    uint32_t adjacency[BMS_GRAPH_MAX_HOSPITALS][BMS_GRAPH_MAX_HOSPITALS];
    uint32_t hospitalCount;
} BmsHospitalGraph_t;

BmsStatus_t GraphInitialize(BmsHospitalGraph_t *graph);
BmsStatus_t GraphAddHospital(BmsHospitalGraph_t *graph,
                             const BmsHospital_t *hospital);
BmsStatus_t GraphRemoveHospital(BmsHospitalGraph_t *graph,
                                BmsHospitalId_t hospitalId);
BmsStatus_t GraphAddRoute(BmsHospitalGraph_t *graph,
                          BmsHospitalId_t sourceHospitalId,
                          BmsHospitalId_t destinationHospitalId,
                          uint32_t distanceKm);
BmsStatus_t GraphRemoveRoute(BmsHospitalGraph_t *graph,
                             BmsHospitalId_t sourceHospitalId,
                             BmsHospitalId_t destinationHospitalId);
BmsStatus_t GraphFindNearestHospital(const BmsHospitalGraph_t *graph,
                                     BmsHospitalId_t sourceHospitalId,
                                     BmsHospitalId_t *nearestHospitalId,
                                     uint32_t *distanceKm);
BmsStatus_t GraphGetRouteDistance(const BmsHospitalGraph_t *graph,
                                  BmsHospitalId_t sourceHospitalId,
                                  BmsHospitalId_t destinationHospitalId,
                                  uint32_t *distanceKm);
BmsStatus_t GraphBreadthFirstSearch(const BmsHospitalGraph_t *graph,
                                    BmsHospitalId_t startHospitalId,
                                    BmsGraphVisit_t visitor,
                                    void *context);
BmsStatus_t GraphDepthFirstSearch(const BmsHospitalGraph_t *graph,
                                  BmsHospitalId_t startHospitalId,
                                  BmsGraphVisit_t visitor,
                                  void *context);
BmsStatus_t GraphBroadcastEmergencyAlert(const BmsHospitalGraph_t *graph,
                                         BmsHospitalId_t sourceHospitalId,
                                         uint32_t maximumDistanceKm,
                                         BmsGraphVisit_t visitor,
                                         void *context);
void GraphClear(BmsHospitalGraph_t *graph);

#ifdef __cplusplus
}
#endif

#endif /* BMS_GRAPH_MANAGEMENT_H */
