
#include <iostream>
#include <stdio.h>
#include <cassert>
#include <math.h>
#include <stdlib.h>
#include <cstring>
#include <vector>

using namespace std;

/* Include for VTK*/
#include <vtkSmartPointer.h>
#include <vtkPolyData.h>
#include <vtkStructuredPointsReader.h>
#include <vtkStructuredPoints.h>
#include <vtkDataArray.h>
#include <vtkImageData.h>
#include <vtkPointData.h>
#include <vtkStreamTracer.h>
#include <vtkPolyDataReader.h>
#include <vtkIdTypeArray.h>
#include <vtkIdList.h>
#include <vtkFloatArray.h>
#include <vtkGenericDataObjectWriter.h>
#include <vtkStructuredPointsWriter.h>
#include <string>

void writeSLFile(std::string inputFilename, vtkSmartPointer<vtkPolyData> polyData)
{
  vtkSmartPointer<vtkGenericDataObjectWriter> writer =
      vtkSmartPointer<vtkGenericDataObjectWriter>::New();
  writer->SetFileName(inputFilename.c_str());
  writer->SetInputData(polyData);
  writer->Update();
  writer->Write();
}

void writeVTKFile(std::string inputFilename, vtkSmartPointer<vtkStructuredPoints> vtkField)
{
  vtkSmartPointer<vtkStructuredPointsWriter> writer =
      vtkSmartPointer<vtkStructuredPointsWriter>::New();
  writer->SetFileName(inputFilename.c_str());
  writer->SetInputData(vtkField);
  writer->Write();
}

/* Read a vector field stored in the VTK structured point format.
   @return vtkStructuredPoints - Store dimensions, spacing, origin and vector field  
*/
vtkSmartPointer<vtkStructuredPoints> readVTKFile(std::string inputFilename)
{
  vtkSmartPointer<vtkStructuredPointsReader> reader =
      vtkSmartPointer<vtkStructuredPointsReader>::New();
  reader->SetFileName(inputFilename.c_str());
  reader->Update();
  vtkSmartPointer<vtkStructuredPoints> structurePoint = reader->GetOutput();

  return structurePoint;
}

/* Test the readVTKFile function*/
void testReadVTKFile(std::string inputFileName)
{
  int dims[3];
  double spacing[3];
  double origin[3];

  vtkSmartPointer<vtkStructuredPoints> structurePoint = readVTKFile(inputFileName);

  // Get dimension
  structurePoint->GetDimensions(dims);

  // Get spacing
  structurePoint->GetSpacing(spacing);

  // Get the origin
  structurePoint->GetOrigin(origin);

  // Print out the description about the data domain
  std::cout << "Data origin: [" << origin[0] << "," << origin[1] << "," << origin[2] << "]\n";
  std::cout << "The data dimensions: " << dims[0] << " x " << dims[1] << " x " << dims[2] << std::endl;
  std::cout << "Grid Spacing: [" << spacing[0] << "," << spacing[1] << "," << spacing[2] << "]\n";

  // Print out the vector value at the center point

  // Get the velocity field
  vtkSmartPointer<vtkDataArray> velocity = structurePoint->GetPointData()->GetArray("velocity");

  int i, j, k; // The indexes of the the center point
  i = dims[0] / 2;
  j = dims[1] / 2;
  k = dims[2] / 2;

  // VTK StructurePoint is 1D array. Need to convert the 3D to 1D index
  int tupleIdx = k * dims[0] * dims[1] + j * dims[0] + i;

  // Get the velocity vector at the center point
  double *velo = velocity->GetTuple(tupleIdx);

  std::cout << "The velocity vector at the center point: " << velo[0] << " " << velo[1] << " " << velo[2] << endl;
}