#include <vtkSmartPointer.h>

#include <vtkStreamTracer.h>

#include <vtkNamedColors.h>
#include <vtkActor.h>
#include <vtkMultiBlockPLOT3DReader.h>
#include <vtkMultiBlockDataSet.h>
#include <vtkPlaneSource.h>
#include <vtkPolyDataMapper.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkStructuredGridOutlineFilter.h>
#include <vtkProperty.h>

#include <vtkImageData.h>
#include <vtkSmartPointer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkInteractorStyleImage.h>
#include <vtkRenderer.h>
#include <vtkImageMapper.h>
#include <vtkImageSliceMapper.h>
#include <vtkImageSlice.h>
#include <vtkImageProperty.h>

//LONG LINE.cxx
#include <vtkActor.h>
#include <vtkCamera.h>
#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkDoubleArray.h>
#include <vtkLine.h>
#include <vtkNamedColors.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkSmartPointer.h>
#include <vector>
#include "globals.h"
#include "Duong/Streamlines.h"
#include "structures.h"
#include "NeighborGraph/Graph.h"

#include <vtkSmartPointer.h>
#include <vtkPointPicker.h>
#include <vtkCamera.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkObjectFactory.h>
#include <vtkCoordinate.h>
#include <vtkRendererCollection.h>

vtkSmartPointer<vtkUnsignedCharArray> cellData;
std::vector<matrix_entry> matrix;

// Define interaction style
class MouseInteractorStyle3 : public vtkInteractorStyleImage
{
public:
  static MouseInteractorStyle3 *New();

  virtual void OnLeftButtonDown()
  {
    std::cout << "Pressed left mouse button." << std::endl;
    int x = this->Interactor->GetEventPosition()[0];
    int y = this->Interactor->GetEventPosition()[1];
    std::cout << "(x,y) = (" << x << "," << y << ")" << std::endl;
    vtkSmartPointer<vtkCoordinate> coordinate =
        vtkSmartPointer<vtkCoordinate>::New();
    coordinate->SetCoordinateSystemToDisplay();
    coordinate->SetValue(x, y, 0);

    // This doesn't produce the right value if the sphere is zoomed in???
    double *world = coordinate->GetComputedWorldValue(this->Interactor->GetRenderWindow()->GetRenderers()->GetFirstRenderer());
    std::cout << "World coordinate: " << world[0] << ", " << world[1] << ", " << world[2] << std::endl;

    // Forward events
    vtkInteractorStyleImage::OnLeftButtonDown();
  }
};

vtkStandardNewMacro(MouseInteractorStyle3);

void filterNeighbors(int index)
{
  for (int i = 0; i < cellData->GetNumberOfTuples(); ++i)
  {
    float rgb[3];
    rgb[0] = 227.0;
    rgb[1] = 207.0;
    rgb[2] = 87.0; // banana
    cellData->InsertTuple(i, rgb);
  }
  segment *s_current = all_segments[index];
  /*for (int i = 0; i < streamlines.size() - 1; i++)
    {
        if (segment_starters[i]->global_index <= index && segment_starters[i + 1]->global_index > index)
        {
            int diff = index - segment_starters[i]->global_index;
            s_current = segment_starters[i];
            for (int j = 0; j < diff; j++)
            {
                s_current = s_current->next;
            }
        }
    }*/
  for (int i = 0; i < s_current->neighbors.size(); i++)
  {
    segment *neighbor = s_current->neighbors[i]->seg;
    float rgb[3];
    rgb[0] = 255;
    rgb[1] = 99;
    rgb[2] = 71;
    cellData->InsertTuple(neighbor->global_index, rgb);
  }
}
std::vector<vtkSmartPointer<vtkRenderWindowInteractor>> interactors;
void renderMatrix();

//void start_visualizer(std::vector<std::vector<node>> streamlines)
void start_visualizer()
{

  std::cerr << "BEGIN";

  int num = 0;
  for (int i = 0; i < streamlines.size(); i++)
  {
    num += streamlines[i].size();
  }
  // Create a vtkPoints object and store the points in it
  vtkSmartPointer<vtkPoints> points =
      vtkSmartPointer<vtkPoints>::New();

  // Create a cell array to store the lines in and add the lines to it
  vtkSmartPointer<vtkCellArray>
      lines =
          vtkSmartPointer<vtkCellArray>::New();

  std::cerr << "size: " << streamlines.size() << std::endl;
  int counter = 0;
  for (int i = 0; i < streamlines.size(); i++)
  {
    //std::cerr << segment_starters[i]->global_index << std::endl;
    std::vector<node> streamline = streamlines[i];
    //std::cerr << "size: " << streamline.size() << std::endl;
    for (int j = 0; j < streamline.size(); j++)
    {
      node n = streamline[j];
      //std::cerr << n.x << ", ";
      points->InsertNextPoint(n.x, n.y, n.z); //InsertNextPoint(i, 1, j); //

      if (j < streamline.size() - 1)
      {
        vtkSmartPointer<vtkLine> line =
            vtkSmartPointer<vtkLine>::New();
        line->GetPointIds()->SetId(0, j + counter);
        line->GetPointIds()->SetId(1, j + 1 + counter);
        lines->InsertNextCell(line);
      }
      //std::cerr << " cm ";
    }
    counter += streamline.size(); //streamline.size();
                                  //std::cerr << "next sl" << endl;
  }
  std::cerr << "SETTING CELL DATA" << std::endl;
  vtkSmartPointer<vtkUnsignedCharArray> cellData2;
  cellData2->SetNumberOfComponents(3);
  cellData2->SetNumberOfTuples(lines->GetNumberOfCells());

  std::cerr << "FILTERING" << std::endl;
  filterNeighbors(10);

  // Create a polydata to store everything in
  vtkSmartPointer<vtkPolyData> linesPolyData =
      vtkSmartPointer<vtkPolyData>::New();
  std::cerr << points->GetNumberOfPoints() << ", " << lines->GetNumberOfCells() << std::endl;
  // Add the points to the dataset
  linesPolyData->SetPoints(points);

  // Add the lines to the dataset
  linesPolyData->SetLines(lines);

  linesPolyData->GetCellData()->SetScalars(cellData);

  // Setup actor and mapper
  vtkSmartPointer<vtkNamedColors> colors =
      vtkSmartPointer<vtkNamedColors>::New();

  vtkSmartPointer<vtkPolyDataMapper> mapper =
      vtkSmartPointer<vtkPolyDataMapper>::New();
  mapper->SetInputData(linesPolyData);
  mapper->ScalarVisibilityOn();
  mapper->SetScalarModeToUseCellData();

  vtkSmartPointer<vtkActor> actor =
      vtkSmartPointer<vtkActor>::New();
  actor->SetMapper(mapper);
  actor->GetProperty()->SetLineWidth(4);
  //actor->GetProperty()->SetColor(colors->GetColor3d("Peacock").GetData());

  // Setup render window, renderer, and interactor
  vtkSmartPointer<vtkRenderer> renderer =
      vtkSmartPointer<vtkRenderer>::New();
  vtkSmartPointer<vtkRenderWindow> renderWindow =
      vtkSmartPointer<vtkRenderWindow>::New();
  renderWindow->SetWindowName("Long Line");
  renderWindow->AddRenderer(renderer);
  vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor =
      vtkSmartPointer<vtkRenderWindowInteractor>::New();
  renderWindowInteractor->SetRenderWindow(renderWindow);

  renderer->AddActor(actor);
  //return;
  renderer->ResetCamera();

  renderer->GetActiveCamera()->Azimuth(30);
  renderer->GetActiveCamera()->Elevation(30);

  renderer->ResetCameraClippingRange();

  renderer->SetBackground(colors->GetColor3d("Silver").GetData());

  renderWindow->Render();

  //    renderWindowInteractor->Start();

  interactors.push_back(renderWindowInteractor);

  std::cerr << "RENDERING MATRIX" << std::endl;
  renderMatrix();

  interactors[1]
      ->Start();
}
void CreateRandomImage(vtkImageData *image, const unsigned int dimension);

void renderMatrix()
{
  int dim;
  matrix = compile_matrix(dim);

  // Big image
  vtkSmartPointer<vtkImageData> image = vtkSmartPointer<vtkImageData>::New();
  CreateRandomImage(image, dim);

  vtkSmartPointer<vtkImageSliceMapper> imageSliceMapper = vtkSmartPointer<vtkImageSliceMapper>::New();
  imageSliceMapper->SetInputData(image);
  imageSliceMapper->BorderOn(); // This line tells the mapper to draw the full border pixels.
  vtkSmartPointer<vtkImageSlice> imageSlice = vtkSmartPointer<vtkImageSlice>::New();
  imageSlice->SetMapper(imageSliceMapper);
  imageSlice->GetProperty()->SetInterpolationTypeToNearest();

  vtkSmartPointer<vtkRenderer> renderer = vtkSmartPointer<vtkRenderer>::New();
  renderer->AddViewProp(imageSlice);
  renderer->ResetCamera();

  vtkSmartPointer<vtkRenderWindow> renderWindow = vtkSmartPointer<vtkRenderWindow>::New();
  renderWindow->AddRenderer(renderer);

  vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor =
      vtkSmartPointer<vtkRenderWindowInteractor>::New();
  vtkSmartPointer<MouseInteractorStyle3> style =
      vtkSmartPointer<MouseInteractorStyle3>::New();
  renderWindowInteractor->SetInteractorStyle(style);

  renderWindowInteractor->SetRenderWindow(renderWindow);
  renderWindow->Render();
  renderWindowInteractor->Initialize();

  interactors.push_back(renderWindowInteractor);
}

void CreateRandomImage(vtkImageData *image, const unsigned int dimension)
{
  image->SetDimensions(dimension, dimension, 1);
  image->SetOrigin(.5, .5, 0);
  image->AllocateScalars(VTK_UNSIGNED_CHAR, 3);

  for (unsigned int x = 0; x < dimension; x++)
  {
    for (unsigned int y = 0; y < dimension; y++)
    {
      unsigned char *pixel = static_cast<unsigned char *>(image->GetScalarPointer(x, y, 0));
      pixel[0] = 255;
      pixel[1] = 255;
      pixel[2] = 255;
    }
  }

  for (unsigned int i = 0; i < matrix.size(); i++)
  {
    matrix_entry entry = matrix[i];
    unsigned char *pixel = static_cast<unsigned char *>(image->GetScalarPointer(entry.indexA, entry.indexB, 0));
    pixel[0] = 0;
    pixel[1] = 0;
    pixel[2] = 0;
  }

  image->Modified();
}
