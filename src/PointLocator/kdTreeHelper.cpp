#include <vtkActor.h>
#include <vtkCamera.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>

#include <vtkCellArray.h>
#include <vtkDoubleArray.h>

#include <vtkPointData.h>
#include <vtkPointSource.h>
#include <vtkPoints.h>

#include <vtkIdList.h>
#include <vtkIncrementalOctreePointLocator.h>
#include <vtkDataSet.h>
#include <vtkSmartPointer.h>

#include <vtkLookupTable.h>
#include <vtkNamedColors.h>

#include "kdTreeHelper.h"

//TODO: remove global variables
PointLocator pointLocatorSL, pointLocatorStdev;
bool CRITICAL_EXCEPTION = false;
//vtkSmartPointer<vtkIncrementalOctreePointLocator> pointTree;

PointLocator::PointLocator(vtkSmartPointer<vtkDataSet> polyData)
{
  pointTree = vtkSmartPointer<vtkIncrementalOctreePointLocator>::New();
  pointTree->SetDataSet(polyData);
  pointTree->BuildLocator();
}

void PointLocator::buildLocator(vtkSmartPointer<vtkDataSet> polyData)
{
  pointTree = vtkSmartPointer<vtkIncrementalOctreePointLocator>::New();
  pointTree->SetDataSet(polyData);
  pointTree->BuildLocator();
}

vtkSmartPointer<vtkIdList> PointLocator::KdTreeFindPointsWithinRadius(double radius, double *pt)
{
  auto result = vtkSmartPointer<vtkIdList>::New();
  pointTree->FindPointsWithinRadius(radius, pt,
                                    result);
  return result;
}

vtkSmartPointer<vtkIdList> PointLocator::KdTreeFindClosestKPoints(int k, double *pt)
{
  auto result = vtkSmartPointer<vtkIdList>::New();
  pointTree->FindClosestNPoints(k, pt,
                                result);
  return result;
}

vtkIdType PointLocator::KdTreeFindClosestPoint(double *pt)
{
  if (CRITICAL_EXCEPTION)
    return -1;
  return pointTree->FindClosestPoint(pt);
}