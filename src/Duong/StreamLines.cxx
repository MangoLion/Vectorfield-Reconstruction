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
#include "../globals.h"
#include "Streamlines.h"
#include "../structures.h"
#include "../NeighborGraph/Graph.h"

#include <vtkSmartPointer.h>
#include <vtkPointPicker.h>
#include <vtkCamera.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkObjectFactory.h>
#include <vtkCoordinate.h>
#include <vtkRendererCollection.h>
#include <vtkUnsignedCharArray.h>
#include <cmath>

float radar_range = 1;
int clickX = -1, clickY = -1;
vtkSmartPointer<vtkRenderer> renderer;
vtkSmartPointer<vtkPolyDataMapper> mapper;
void updateImage(vtkImageData *image, const unsigned int dimension, std::vector<matrix_entry> matrix);
void filterNeighbors(int index, vtkSmartPointer<vtkUnsignedCharArray> &cellData);
void renderMatrix(vtkSmartPointer<vtkUnsignedCharArray> &cellData, int dim, std::vector<matrix_entry> &matrix);
// Define interaction style
class MouseInteractorStyle3 : public vtkInteractorStyleImage
{
public:
    static MouseInteractorStyle3 *New();
    vtkImageData *image;
    unsigned int dimension;
    std::vector<matrix_entry> matrix;
    vtkSmartPointer<vtkUnsignedCharArray> cellData;
    void set(vtkImageData *image_, const unsigned int dimension_, std::vector<matrix_entry> matrix_, vtkSmartPointer<vtkUnsignedCharArray> &cellData_)
    {
        image = image_;
        dimension = dimension_;
        matrix = matrix_;
        std::cout << "CELL DATA SIZE: " << cellData_->GetNumberOfTuples() << std::endl;
        //std::cout << "MATRIX SIZE: " << matrix->size() << std::endl;
        cellData = cellData_;
    }

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
        std::cout << "Image Length: " << image->GetDimensions()[0] << std::endl;
        clickX = world[0];
        clickY = world[1];
        std::cout << "CELL DATA SIZE: " << cellData->GetNumberOfTuples() << std::endl;
        std::cout << "MATRIX SIZE: " << matrix.size() << std::endl;
        //matrix_entry entry = (*matrix)[0];
        for (int i = 0; i < matrix.size(); i++)
        {
            matrix_entry entry = matrix[i];
            if (entry.indexA == clickX || entry.indexA == clickY)
            {
                filterNeighbors(entry.A->global_index, cellData);
            }

            if (entry.indexB == clickX || entry.indexB == clickY)
            {
                filterNeighbors(entry.B->global_index, cellData);
            }
        }

        updateImage(image, dimension, matrix);
        this->Interactor->GetRenderWindow()->Render();

        mapper->Modified();
        renderer->Modified();
        renderer->GetRenderWindow()->Render();
        // Forward events
        vtkInteractorStyleImage::OnLeftButtonDown();
    }
};

vtkStandardNewMacro(MouseInteractorStyle3);

void filterNeighbors(int index, vtkSmartPointer<vtkUnsignedCharArray> &cellData)
{
    for (int i = 0; i < cellData->GetNumberOfTuples(); ++i)
    {
        float rgb[4];
        rgb[0] = 227.0;
        rgb[1] = 207.0;
        rgb[2] = 87.0; // banana
        rgb[3] = 0;
        cellData->InsertTuple(i, rgb);
    }
    segment *s_current = all_segments[index];
    std::cout << "GLOBAL: " << s_current->streamline_index << ", #" << s_current->streamline_segment_index << std::endl
              << s_current->start->x << ", " << s_current->start->y << ", " << s_current->start->z << std::endl
              << s_current->end->x << ", " << s_current->end->y << ", " << s_current->end->z << std::endl
              << s_current->middle->x << ", " << s_current->middle->y << ", " << s_current->middle->z << std::endl;
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

    for (int n = 0; n < total_nodes; n++)
    {
        segment *s_next = all_segments[n];
        float rgb[4];
        rgb[0] = 227.0;
        rgb[1] = 207.0;
        rgb[2] = 87.0; // banana

        if (std::abs(s_next->middle->x - s_current->middle->x) + std::abs(s_next->middle->y - s_current->middle->y) + std::abs(s_next->middle->z - s_current->middle->z) > radar_range)
            rgb[3] = 0;
        else
        {
            rgb[3] = 255;
            //if (n % 100 == 0)
            //std::cerr << std::abs(s_next->middle->x - s_current->middle->x) + std::abs(s_next->middle->y - s_current->middle->y) + std::abs(s_next->middle->z - s_current->middle->z) << std::endl;
        }

        cellData->InsertTuple(s_next->global_index, rgb);
        for (int i = 0; i < s_next->neighbors.size(); i++)
        {
            segment *neighbor = s_next->neighbors[i]->seg;
            if (neighbor == s_current)
            {
                float rgb[4];
                rgb[0] = 255;
                rgb[1] = 255;
                rgb[2] = 255;
                rgb[3] = 255;
                cellData->InsertTuple(s_next->global_index, rgb);
            }
        }
    }

    for (int i = 0; i < s_current->neighbors.size(); i++)
    {
        segment *neighbor = s_current->neighbors[i]->seg;
        float rgb[4];
        rgb[0] = 255;
        rgb[1] = 99;
        rgb[2] = 71;
        rgb[3] = 255;
        cellData->InsertTuple(neighbor->global_index, rgb);
    }

    float rgb[4];
    rgb[0] = 0;
    rgb[1] = 255;
    rgb[2] = 0;
    rgb[3] = 255;
    cellData->InsertTuple(s_current->global_index, rgb);
}
std::vector<vtkSmartPointer<vtkRenderWindowInteractor>> interactors;

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
        counter += streamline.size(); //1
        //std::cerr << streamline.size() << std::endl;
        //std::cerr << "next sl" << endl;
    }

    vtkSmartPointer<vtkUnsignedCharArray> cellData =
        vtkSmartPointer<vtkUnsignedCharArray>::New();
    cellData->SetNumberOfComponents(4);
    cellData->SetNumberOfTuples(lines->GetNumberOfCells());

    segment *ss = segment_starters[12];
    ss->next->next;
    filterNeighbors(ss->global_index, cellData);

    // Create a polydata to store everything in
    vtkSmartPointer<vtkPolyData> linesPolyData =
        vtkSmartPointer<vtkPolyData>::New();
    std::cerr << points->GetNumberOfPoints() << ", " << lines->GetNumberOfCells() << std::endl;
    // Add the points to the dataset
    linesPolyData->SetPoints(points);

    // Add the lines to the dataset
    linesPolyData->SetLines(lines);

    linesPolyData->GetCellData()->SetScalars(cellData);

    points->ComputeBounds();
    double *bounds = points->GetBounds();
    matrix_region_x1 = bounds[0];
    matrix_region_x2 = bounds[1];
    matrix_region_y1 = bounds[2];
    matrix_region_y2 = bounds[3] / 3;
    matrix_region_z1 = bounds[4] / 3;
    matrix_region_z2 = bounds[5] / 3;

    radar_range = (abs(matrix_region_x2 - matrix_region_x1) + abs(matrix_region_y2 - matrix_region_y1) + abs(matrix_region_z2 - matrix_region_z1)) / 6;
    std::cerr << "RADAR: " << radar_range << std::endl;

    // Setup actor and mapper
    vtkSmartPointer<vtkNamedColors> colors =
        vtkSmartPointer<vtkNamedColors>::New();
    mapper =
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
    renderer =
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
    renderWindow->SetSize(700, 700);
    renderWindow->Render();

    //    renderWindowInteractor->Start();

    vtkSmartPointer<vtkInteractorStyleTrackballCamera> style =
        vtkSmartPointer<vtkInteractorStyleTrackballCamera>::New(); //like paraview

    renderWindowInteractor->SetInteractorStyle(style);

    interactors.push_back(renderWindowInteractor);
    int dim;
    std::vector<matrix_entry> matrix = compile_matrix(dim);
    renderMatrix(cellData, dim, matrix);
    filterNeighbors(150, cellData);

    interactors[1]
        ->Start();
}
void CreateRandomImage(vtkImageData *image, const unsigned int dimension, std::vector<matrix_entry> &matrix);

void renderMatrix(vtkSmartPointer<vtkUnsignedCharArray> &cellData, int dim, std::vector<matrix_entry> &matrix)
{
    // Big image
    vtkSmartPointer<vtkImageData> image = vtkSmartPointer<vtkImageData>::New();
    CreateRandomImage(image, dim, matrix);

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
    style->set(image, dim, matrix, cellData);
    renderWindowInteractor->SetInteractorStyle(style);

    renderWindowInteractor->SetRenderWindow(renderWindow);
    renderWindow->SetSize(800, 800);
    renderWindow->Render();
    renderWindowInteractor->Initialize();

    interactors.push_back(renderWindowInteractor);
}

void updateImage(vtkImageData *image, const unsigned int dimension, std::vector<matrix_entry> matrix)
{
    if (clickX < 0 || clickX > dimension ||
        clickY < 0 || clickY > dimension)
        return;

    for (unsigned int x = 0; x < dimension; x++)
    {
        for (unsigned int y = 0; y < dimension; y++)
        {
            unsigned char *pixel = static_cast<unsigned char *>(image->GetScalarPointer(x, y, 0));
            pixel[0] = 0;
            pixel[1] = 0;
            pixel[2] = 0;
        }
    }

    if (clickX > -1)
    {
        for (unsigned int i = 0; i < dimension; i++)
        {
            unsigned char *pixel = static_cast<unsigned char *>(image->GetScalarPointer(clickX, i, 0));
            pixel[0] = 100;
            pixel[1] = 100;
            pixel[2] = 100;

            pixel = static_cast<unsigned char *>(image->GetScalarPointer(i, clickY, 0));
            pixel[0] = 100;
            pixel[1] = 100;
            pixel[2] = 100;
        }
    }

    for (unsigned int i = 0; i < matrix.size(); i++)
    {
        matrix_entry entry = matrix[i];
        unsigned char *pixel = static_cast<unsigned char *>(image->GetScalarPointer(entry.indexA, entry.indexB, 0));
        pixel[0] = 255;
        pixel[1] = 255;
        pixel[2] = 255;
    }

    image->Modified();
}

void CreateRandomImage(vtkImageData *image, const unsigned int dimension, std::vector<matrix_entry> &matrix)
{
    image->SetDimensions(dimension, dimension, 1);
    image->SetOrigin(.5, .5, 0);
    image->AllocateScalars(VTK_UNSIGNED_CHAR, 3);

    updateImage(image, dimension, matrix);
}
