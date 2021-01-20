
#include <vtkIdList.h>
#include <vtkIncrementalOctreePointLocator.h>
#include <vtkDataSet.h>
#include <vtkDataArray.h>
#include <vtkSmartPointer.h>
#include <vector>
#ifndef KDTREEHELPER_h
#define KDTREEHELPER_h
class PointLocator
{
private:
public:
  bool ready = false;
  vtkSmartPointer<vtkIncrementalOctreePointLocator> pointTree;
  vtkSmartPointer<vtkDataArray> velocity;
  vtkSmartPointer<vtkDataArray> heatmap;
  std::vector<vtkIdType> idMap;
  PointLocator(){};
  PointLocator(vtkSmartPointer<vtkDataSet> polyData);
  void buildLocator(vtkSmartPointer<vtkDataSet> polyData);
  vtkSmartPointer<vtkIdList> KdTreeFindPointsWithinRadius(double radious, double *pt);
  vtkSmartPointer<vtkIdList> KdTreeFindClosestKPoints(int k, double *pt);
  vtkIdType KdTreeFindClosestPoint(double *pt);
};

extern PointLocator pointLocatorSL,
    pointLocatorStdev;
/*extern vtkSmartPointer<vtkIncrementalOctreePointLocator> pointTree;
void setKdTreeDataset(vtkSmartPointer<vtkDataSet> polyData);
vtkSmartPointer<vtkIdList> KdTreeFindPointsWithinRadius(double radious, double *pt);
vtkSmartPointer<vtkIdList> KdTreeFindClosestKPoints(int k, double *pt);
vtkIdType KdTreeFindClosestPoint(double *pt);*/
extern bool CRITICAL_EXCEPTION;
extern int RECON_K_NEIGHBORS;
#endif