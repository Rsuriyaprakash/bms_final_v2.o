/** @file graph_management.c @brief Hospital graph implementation. */
#include "graph_management.h"
#include "logging.h"
#include <string.h>

static int32_t FindIndex(const BmsHospitalGraph_t *g,BmsHospitalId_t id){
uint32_t i;
if(g==NULL){
	return -1;
}
for(i=0U;i<BMS_GRAPH_MAX_HOSPITALS;++i){
	if(g->nodes[i].occupied&&(g->nodes[i].hospital.hospitalId==id)){
		return(int32_t)i;
	}
}
return-1;
}
BmsStatus_t GraphInitialize(BmsHospitalGraph_t *g){uint32_t i,j;if(g==NULL)return BMS_STATUS_INVALID_ARGUMENT;(void)memset(g,0,sizeof(*g));for(i=0U;i<BMS_GRAPH_MAX_HOSPITALS;++i)for(j=0U;j<BMS_GRAPH_MAX_HOSPITALS;++j)g->adjacency[i][j]=(i==j)?0U:BMS_GRAPH_NO_ROUTE_DISTANCE;return BMS_STATUS_OK;}
BmsStatus_t GraphAddHospital(BmsHospitalGraph_t *g,const BmsHospital_t *h){uint32_t i;if((g==NULL)||(h==NULL))return BMS_STATUS_INVALID_ARGUMENT;if(FindIndex(g,h->hospitalId)>=0)return BMS_STATUS_ALREADY_EXISTS;for(i=0U;i<BMS_GRAPH_MAX_HOSPITALS;++i)if(!g->nodes[i].occupied){g->nodes[i].occupied=true;g->nodes[i].hospital=*h;++g->hospitalCount;
BMS_LOG_DEBUG("GRAPH",
              "Graph hospital added: hospitalId=%u index=%u",
              (unsigned int)h->hospitalId,
              (unsigned int)i);
return BMS_STATUS_OK;}return BMS_STATUS_GRAPH_FULL;}
BmsStatus_t GraphRemoveHospital(BmsHospitalGraph_t *g,BmsHospitalId_t id){int32_t idx;uint32_t i;if(g==NULL)return BMS_STATUS_INVALID_ARGUMENT;idx=FindIndex(g,id);if(idx<0)return BMS_STATUS_NOT_FOUND;g->nodes[idx].occupied=false;for(i=0U;i<BMS_GRAPH_MAX_HOSPITALS;++i){g->adjacency[idx][i]=BMS_GRAPH_NO_ROUTE_DISTANCE;g->adjacency[i][idx]=BMS_GRAPH_NO_ROUTE_DISTANCE;}g->adjacency[idx][idx]=0U;--g->hospitalCount;return BMS_STATUS_OK;}
BmsStatus_t GraphAddRoute(BmsHospitalGraph_t *g,BmsHospitalId_t a,BmsHospitalId_t b,uint32_t d){int32_t i,j;if((g==NULL)||(d==0U))return BMS_STATUS_INVALID_ARGUMENT;i=FindIndex(g,a);j=FindIndex(g,b);if((i<0)||(j<0))return BMS_STATUS_NOT_FOUND;g->adjacency[i][j]=d;g->adjacency[j][i]=d;
BMS_LOG_INFO("GRAPH",
             "Route added: from=%u to=%u distance=%u",
             (unsigned int)a,
             (unsigned int)b,
             (unsigned int)d);
return BMS_STATUS_OK;}
BmsStatus_t GraphRemoveRoute(BmsHospitalGraph_t *g,BmsHospitalId_t a,BmsHospitalId_t b){int32_t i,j;if(g==NULL)return BMS_STATUS_INVALID_ARGUMENT;i=FindIndex(g,a);j=FindIndex(g,b);if((i<0)||(j<0))return BMS_STATUS_NOT_FOUND;g->adjacency[i][j]=BMS_GRAPH_NO_ROUTE_DISTANCE;g->adjacency[j][i]=BMS_GRAPH_NO_ROUTE_DISTANCE;
BMS_LOG_INFO("GRAPH",
             "Route removed: from=%u to=%u",
             (unsigned int)a,
             (unsigned int)b);
return BMS_STATUS_OK;}
BmsStatus_t GraphFindNearestHospital(const BmsHospitalGraph_t *g,BmsHospitalId_t source,BmsHospitalId_t *nearest,uint32_t *distance){int32_t s;uint32_t i,best=BMS_GRAPH_NO_ROUTE_DISTANCE;BmsHospitalId_t id=0U;if((g==NULL)||(nearest==NULL)||(distance==NULL))return BMS_STATUS_INVALID_ARGUMENT;s=FindIndex(g,source);if(s<0)return BMS_STATUS_NOT_FOUND;for(i=0U;i<BMS_GRAPH_MAX_HOSPITALS;++i)if(g->nodes[i].occupied&&(g->adjacency[s][i]>0U)&&(g->adjacency[s][i]<best)){best=g->adjacency[s][i];id=g->nodes[i].hospital.hospitalId;}if(id==0U)return BMS_STATUS_ROUTE_NOT_FOUND;*nearest=id;*distance=best;return BMS_STATUS_OK;}
BmsStatus_t GraphGetRouteDistance(const BmsHospitalGraph_t*g,BmsHospitalId_t a,BmsHospitalId_t b,uint32_t*d){int32_t i,j;if((g==NULL)||(d==NULL))return BMS_STATUS_INVALID_ARGUMENT;i=FindIndex(g,a);j=FindIndex(g,b);if((i<0)||(j<0))return BMS_STATUS_NOT_FOUND;if(g->adjacency[i][j]==BMS_GRAPH_NO_ROUTE_DISTANCE)return BMS_STATUS_ROUTE_NOT_FOUND;*d=g->adjacency[i][j];return BMS_STATUS_OK;}
static BmsStatus_t Traverse(const BmsHospitalGraph_t *g,BmsHospitalId_t start,BmsGraphVisit_t visitor,void *ctx,bool dfs){bool seen[BMS_GRAPH_MAX_HOSPITALS]={false};uint32_t work[BMS_GRAPH_MAX_HOSPITALS],head=0U,tail=0U,i;int32_t s=FindIndex(g,start);if((g==NULL)||(visitor==NULL))return BMS_STATUS_INVALID_ARGUMENT;if(s<0)return BMS_STATUS_NOT_FOUND;work[tail++]=(uint32_t)s;while(head<tail){uint32_t cur=dfs?work[--tail]:work[head++];if(seen[cur])continue;seen[cur]=true;if(visitor(&g->nodes[cur].hospital,ctx)!=BMS_STATUS_OK)return BMS_STATUS_INTERNAL_ERROR;for(i=0U;i<BMS_GRAPH_MAX_HOSPITALS;++i)if(g->nodes[i].occupied&&!seen[i]&&(g->adjacency[cur][i]!=BMS_GRAPH_NO_ROUTE_DISTANCE)&&(cur!=i))work[tail++]=i;}return BMS_STATUS_OK;}
BmsStatus_t GraphBreadthFirstSearch(const BmsHospitalGraph_t*g,BmsHospitalId_t s,BmsGraphVisit_t v,void*c){return Traverse(g,s,v,c,false);}
BmsStatus_t GraphDepthFirstSearch(const BmsHospitalGraph_t*g,BmsHospitalId_t s,BmsGraphVisit_t v,void*c){return Traverse(g,s,v,c,true);}
BmsStatus_t GraphBroadcastEmergencyAlert(const BmsHospitalGraph_t*g,BmsHospitalId_t source,uint32_t max,BmsGraphVisit_t v,void*c){int32_t s;uint32_t i;if((g==NULL)||(v==NULL))return BMS_STATUS_INVALID_ARGUMENT;s=FindIndex(g,source);if(s<0)return BMS_STATUS_NOT_FOUND;for(i=0U;i<BMS_GRAPH_MAX_HOSPITALS;++i)if(g->nodes[i].occupied&&(i!=(uint32_t)s)&&(g->adjacency[s][i]<=max)){BmsStatus_t st=v(&g->nodes[i].hospital,c);if(st!=BMS_STATUS_OK)return st;}return BMS_STATUS_OK;}
void GraphClear(BmsHospitalGraph_t*g){if(g!=NULL)(void)GraphInitialize(g);}
