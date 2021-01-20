#include <vtkSmartPointer.h>
#include <vtkPolyData.h>
#include <vtkStructuredPoints.h>
#ifndef VF_h
#define VF_h
vtkSmartPointer<vtkStructuredPoints> readVTKFile(std::string inputFilename);

void writeVTKFile(std::string inputFilename, vtkSmartPointer<vtkStructuredPoints> vtkField);
void writeSLFile(std::string inputFilename, vtkSmartPointer<vtkPolyData> polyData);
#endif