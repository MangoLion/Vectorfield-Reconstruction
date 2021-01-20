/// test

#include <stdio.h>
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <ctype.h>
#include <sstream>
#include <cmath>
#include <limits>
#define _USE_MATH_DEFINES
#include <math.h>


#include "Unused/Skeleton.h"
#include <time.h>
#include <string>
#include <fstream>
#include "structures.h"
#include "DataReaders/Reader.h"
#include "NeighborGraph/Graph.h"
#include "DataReaders/VectorField.h"
#include "Unused/kdtree.h"
#include "globals.h"
#include "Duong/Streamlines.h"
#include "ReconMethods/ReconstructionCurrent.h"

#include <vtkSmartPointer.h>
#include <vtkPolyData.h>
#include <vtkStructuredPointsReader.h>
#include <vtkStructuredPoints.h>
#include <vtkDataArray.h>
#include <vtkImageData.h>
#include <vtkDoubleArray.h>
#include <vtkShortArray.h>
#include <vtkIntArray.h>
#include <vtkPointData.h>
#include <vtkStreamTracer.h>
#include <vtkPolyDataReader.h>
#include <vtkIdTypeArray.h>
#include <vtkIdList.h>
#include <vtkFloatArray.h>
#include <omp.h>
//#include <ncurses.h>
#include "PointLocator/kdTreeHelper.h"

#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkDoubleArray.h>
#include <vtkLine.h>

#include "Duong/Steady3D.h"
#include "Duong/StreamlineTracer.h"
#include "Duong/StreamlineTracer.h"
#include "vtkPointSet.h"
#include <chrono>
#include "./DataReaders/TestReader.h"

#define _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING
#include <experimental/filesystem>
using namespace std::chrono;
namespace fs = std::experimental::filesystem;

#include <signal.h>
#include <stdio.h>

//IMPORTANT: PATH TO DATASET
std::string DATASET_DIR = "D:/Academic/VReconstruction/datasets/";
//IMPORTANT: PATH TO OUTPUT
std::string DIRECTORY = "F:/academic/VectorRecon/";

void clearDataset();
void readVTK(std::string vtkFile);
void buildSegmentsPolyData();
vtkSmartPointer<vtkPolyData> buildStreamlinesPolyData(std::vector<std::vector<node>> streamlines);
void compareStreamlines(std::string slFile1, std::string slFile2);
using namespace Eigen;
void compareStreamlines(std::vector<std::vector<Vector3f>> orgSL, std::vector<std::vector<Vector3f>> compSL, std::string name, bool writeOrg);
std::vector<Vector3f> buildRandomSeedingCurve(Steady3D& originalVF, int numSL);
std::vector<Vector3f> segmentStarterstoSeedingPoints();
std::vector<std::vector<Vector3f>> traceSL(Steady3D& vectorField, std::vector<Vector3f> seedingCurve);
Steady3D buildSteady3D(const vtkSmartPointer<vtkStructuredPoints>& originalVF);



/*struct TestProfile
{
	std::string vtkFile, slFile;
	bool doNormalizeCheck, doNormalizeRecon, useSegment;
	int skipNodes, skipLines;
	float radar, thresholdAngle, thresholdVelocity;
};*/

void testErrorStdev(float thresholdSize, float maxThreshold, vtkSmartPointer<vtkStructuredPoints> originalVF, vtkSmartPointer<vtkStructuredPoints> reconVF,int skipLines, int skipNodes,
	std::string testid, std::string testCategories);

/// <summary>
/// Original Vectorfield
/// </summary>
vtkSmartPointer<vtkStructuredPoints> originalVF;

/// <summary>
/// Reconstructed Vectorfield
/// </summary>
vtkSmartPointer<vtkStructuredPoints> reconVF;

void createTestFolder(string dir) {
	fs::create_directory(dir + "bernard");
	fs::create_directory(dir + "tornado");
	fs::create_directory(dir + "cylinder");
	fs::create_directory(dir + "streamlines");
}

/// <summary>
/// Have the user input the test ID then create the directory and subdirectories if needed
/// </summary>
void getBatchID() {
	cout << "Enter Batch ID:";
	string testID;
	cin >> testID;
	DIRECTORY += testID + "/";
	fs::create_directory(DIRECTORY);
	fs::create_directory(DIRECTORY+"tests");
	createTestFolder(DIRECTORY);
}

void askResetTest() {
	cout << "Continue? (y/n)" << endl;
	char choice;
	cin >> choice;
	switch (choice) {
	case 'n':
		ofstream counterFile(DIRECTORY + "tests/counter.txt");
		counterFile << -1;
		counterFile.close();
		break;
	}
}

/// <summary>
/// Create a list of tests using the latest reconstruction method, save this list to <testid>/tests/tests_latest.tsv
/// </summary>
void createTestsLatest() {
	int last_sl,
		last_sn;
	
	std::string datasetNames[3] = { "bernard", "tornado", "cylinder" };
	std::string vtkFileNames[3] = { "bernard3D.vtk", "tornado3D.vtk", "cylinder3D.vtk" };
	std::string slFileNames[3] = { "bernard.txt", "tornado.txt", "cylinder.txt" };
	int skips[4] = { 1, 2, 3, 5 };
	int radars[10] = { 1,2,4,8,16,32,64,128 };

	ofstream testFile(DIRECTORY + "tests/tests_latest.tsv");
	testFile << "TESTID DATASET SKIPLINES SKIPNODES EXCLUDE_THRESHOLD K_NEIGHBORS USE_NEIGHBORS RADAR" << endl;
	int test_num = 0;
	for (int f = 0; f <2; f+=1) //0.524512
	{
		int skipLines = 2, skipNodes = 2;
		last_sl = skipLines;
		last_sn = skipNodes;

		for (int USE_N = 0; USE_N <= 1; USE_N++)
			for (RECON_EXCLUDE_THRESHOLD = PI / 50; RECON_EXCLUDE_THRESHOLD <= PI / 20; RECON_EXCLUDE_THRESHOLD += PI / 50)
				for (RECON_K_NEIGHBORS = 5; RECON_K_NEIGHBORS < 81; RECON_K_NEIGHBORS *= 4)
					for (float radar = 10; radar <= 20; radar *= 2)
						//lock SL Nodes at 2-2
						for (int skipl = 1; skipl < 2; skipl++)
							for (int skipn = 1; skipn < 2; skipn++)
							{
								if (USE_N == 1)
									RECON_USE_NEIGHBORS = true;
								else {
									RECON_USE_NEIGHBORS = false;
									if (RECON_K_NEIGHBORS > 5)
										continue;
								}

								stringstream issCat;
								issCat << test_num<<" "<< datasetNames[f] << " " << skipLines << " " << skipNodes <<" "<< RECON_EXCLUDE_THRESHOLD<< " " << RECON_K_NEIGHBORS << " " << RECON_USE_NEIGHBORS << " " << radar;
								cout << issCat.str() << endl;
								testFile << issCat.str() << endl;
								test_num++;
							}

	}
	

	testFile.close();
	//system("pause");
}

/// <summary>
/// Create a list of tests using the legacy reconstruction method, save this list to <testid>/tests/tests_legacy.tsv
/// </summary>
void createTestsLegacy() {
	askResetTest();
	//temp variable to check if the parameter for the streamline densities are changed, if so then we re-process the streamlines
	int last_sl,
		last_sn;

	std::string datasetNames[3] = { "bernard", "cylinder" , "tornado" };
	std::string vtkFileNames[3] = { "bernard3D.vtk", "cylinder3D.vtk", "tornado3D.vtk" };
	std::string slFileNames[3] = { "bernard.txt", "cylinder.txt", "tornado.txt" };

	ofstream testFile(DIRECTORY + "tests/tests_legacy.tsv");
	testFile << "TESTID DATASET SKIPLINES SKIPNODES EXCLUDE_THRESHOLD K_NEIGHBORS USE_NEIGHBORS RADAR" << endl;
	int test_num = 0;
	for (int f = 0; f < 3; f += 1)
	{
		int skipLines = 2, skipNodes = 2;
		last_sl = skipLines;
		last_sn = skipNodes;


		for (float radar = 1; radar <= 8; radar *= 2)
			//lock SL Nodes at 2-2
			for (int skipl = 1; skipl < 2; skipl++)
				for (int skipn = 1; skipn < 2; skipn++)
				{

					stringstream issCat;
					issCat << test_num << " " << datasetNames[f] << " " << skipLines << " " << skipNodes<<" " << radar;
					cout << issCat.str() << endl;
					testFile << issCat.str() << endl;
					test_num++;
				}

	}


	testFile.close();
	//system("pause");
}

/// <summary>
/// Run tests using latest reconstruction method, first generated by the createTestLatest() method
/// </summary>
void runTestsLatest() {
	askResetTest();
	//reset logs
	std::ofstream outfileID, outfile;
	outfileID.open(DIRECTORY + "logsID.csv", std::ios_base::app);
	outfileID << "";
	outfileID.close();
	outfile.open(DIRECTORY + "logs.csv", std::ios_base::app);
	outfile << "";
	outfile.close();

	//v_diff_average angle_diff_count angle_diff_average v_diff_counter
	std::string DATASET_DIR = "D:/Academic/VReconstruction/datasets/";
	std::string datasetNames[3] = { "bernard", "tornado", "cylinder" };
	std::string vtkFileNames[3] = { "bernard3D.vtk", "tornado3D.vtk", "cylinder3D.vtk" };
	std::string slFileNames[3] = { "bernard.txt", "tornado.txt", "cylinder.txt" };
	int skips[4] = { 1, 2, 3, 5 };
	int radars[10] = { 1,2,4,8,16,32,64,128 };

	string current_dataset = "";
	int skipLines = -1, skipNodes = -1;
	float radar;

	fstream testFile(DIRECTORY + "tests/tests_latest.tsv"),
		counterFile(DIRECTORY + "tests/counter.txt"),
		resultsFile(DIRECTORY + "tests/results.txt");
	int counter = 0, target_counter, test_num;
	counterFile >> target_counter;
	counterFile.close();
	//while (testFile >> test_num >> dataset_name >> newSL >> newSN >> RECON_EXCLUDE_THRESHOLD >> RECON_K_NEIGHBORS >> RECON_USE_NEIGHBORS >> radar) {
	string line;
	getline(testFile, line);
	setTestHeader(line);

	while (getline(testFile, line)){
		readTestNextLine(line);
		if (counter < target_counter + 1) {
			counter++;
			continue;
		}
		cout << "#" << counter << " ";
		target_counter = -1;
		bool doReadStreamlines = false;
		if (P_DATASET != current_dataset) {
			current_dataset = P_DATASET;
			readVTK(DATASET_DIR + current_dataset + "3D.vtk");
			Steady3D orgSteady = buildSteady3D(originalVF);
			doReadStreamlines = true;
		}

		if (P_SKIPLINES != skipLines || P_SKIPNODES != skipNodes || doReadStreamlines) {
			auto start = high_resolution_clock::now();
			readStreamlines(streamlines, DATASET_DIR + current_dataset + ".txt", skipLines, skipNodes);
			writeSLFile(DIRECTORY + current_dataset + "/" + current_dataset + std::to_string(skipLines) + "-" + std::to_string(skipNodes) + "SL.vtk", buildStreamlinesPolyData(streamlines));
			//continue;
			read_segments_from_sl();
			buildSegmentsPolyData();
			update_graph(streamlines);
			auto stop = high_resolution_clock::now();
			skipLines = P_SKIPLINES;
			skipNodes = P_SKIPNODES;
		}

		std::stringstream issID, issCat;
		issID << current_dataset << std::to_string(skipLines) << "sl-" << std::to_string(skipNodes) << "sn-" << std::to_string(RECON_K_NEIGHBORS) << "knodes-" << std::to_string(RECON_EXCLUDE_THRESHOLD) << "threshold-" << std::to_string(RECON_USE_NEIGHBORS) << "use_n-" << std::to_string((int)P_RADAR_LEVEL) << "radar";
		//issID << datasetNames[f] << std::to_string(skipLines) << "sl-" << std::to_string(skipNodes) << "sn-" << std::to_string((int)radar) << "radar";
		std::cerr << "---Test Id: " << issID.str() << std::endl;
		issCat << current_dataset << "," << skipLines << ", " << skipNodes << ", " << RECON_K_NEIGHBORS << ", " << RECON_USE_NEIGHBORS << ", " << P_RADAR_LEVEL;
		//testError(originalVF, reconVF, false, false, -1, -1, true, SEARCH_LENGTH, err_angle, err_dot, err_mag, err_vx, err_vy, err_vz, skipLines, skipNodes, issID.str(), issCat.str());
		P_RADAR = SEARCH_LENGTH * P_RADAR_LEVEL;
		if (P_RADAR == 0) {
			std::cout << P_RADAR_LEVEL<<endl;
			std::cout << line<<endl;
		}
		testErrorStdev(0.03, 1.5, originalVF, reconVF, skipLines, skipNodes, issID.str(), issCat.str());
		writeVTKFile(DIRECTORY + current_dataset + "/" + issID.str() + " .vtk", reconVF);

		resultsFile << "#" << counter << issCat.str() << ": succeeded" << endl;
		counterFile.open(DIRECTORY + "tests/counter.txt");
		counterFile << " " << counter;
		counterFile.close();
		counter++;
	}
	testFile.close();
}


/// <summary>
/// Run Legacy Tests generated by createLegacyTests()
/// </summary>
void runTestsLegacy() {
	std::ofstream outfileID, outfile;
	outfileID.open(DIRECTORY + "logsID.csv", std::ios_base::app);
	outfileID << "";
	outfileID.close();
	outfile.open(DIRECTORY + "logs.csv", std::ios_base::app);
	outfile << "";
	outfile.close();

	//v_diff_average angle_diff_count angle_diff_average v_diff_counter
	std::string DATASET_DIR = "D:/Academic/VReconstruction/datasets/";
	std::string datasetNames[3] = { "bernard", "tornado", "cylinder" };
	std::string vtkFileNames[3] = { "bernard3D.vtk", "tornado3D.vtk", "cylinder3D.vtk" };
	std::string slFileNames[3] = { "bernard.txt", "tornado.txt", "cylinder.txt" };
	int skips[4] = { 1, 2, 3, 5 };
	int radars[10] = { 1,2,4,8,16,32,64,128 };

	/*readStreamlines(streamlines, slFileNames[1], 1, 1);
	read_segments_from_sl();
	buildSegmentsPolyData();
	update_graph(streamlines);*/

	string current_dataset = "";
	int skipLines = -1, skipNodes = -1;
	float radar;

	fstream testFile(DIRECTORY + "tests/tests_legacy.tsv"),
		counterFile(DIRECTORY + "tests/counter.txt"),
		resultsFile(DIRECTORY + "tests/results.txt");
	int counter = 0, target_counter, test_num;
	counterFile >> target_counter;
	counterFile.close();
	//while (testFile >> test_num >> dataset_name >> newSL >> newSN >>  radar) {
	string line;
	getline(testFile, line);
	setTestHeader(line);

	while (getline(testFile, line)) {
		readTestNextLine(line);
		if (counter < target_counter + 1) {
			counter++;
			continue;
		}
		cout << "#" << counter << " ";
		target_counter = -1;
		bool doReadStreamlines = false;
		if (P_DATASET != current_dataset) {
			current_dataset = P_DATASET;
			readVTK(DATASET_DIR + current_dataset + "3D.vtk");
			Steady3D orgSteady = buildSteady3D(originalVF);
			doReadStreamlines = true;
		}

		if (P_SKIPLINES != skipLines || P_SKIPNODES != skipNodes || doReadStreamlines) {
			auto start = high_resolution_clock::now();
			readStreamlines(streamlines, DATASET_DIR + current_dataset + ".txt", skipLines, skipNodes);
			writeSLFile(DIRECTORY + current_dataset + "/" + current_dataset + std::to_string(skipLines) + "-" + std::to_string(skipNodes) + "SL.vtk", buildStreamlinesPolyData(streamlines));
			//continue;
			read_segments_from_sl();
			buildSegmentsPolyData();
			update_graph(streamlines);
			auto stop = high_resolution_clock::now();
			skipLines = P_SKIPLINES;
			skipNodes = P_SKIPNODES;
		}

		std::stringstream issID, issCat;
		issID << current_dataset << std::to_string(skipLines) << "sl-" << std::to_string(skipNodes) << "sn-" << std::to_string((int)P_RADAR_LEVEL) << "radar";
		std::cerr << "---Test Id: " << issID.str() << std::endl;
		issCat << current_dataset << "," << skipLines << ", " << skipNodes << ", " << P_RADAR_LEVEL;
		//testError(originalVF, reconVF, false, false, -1, -1, true, SEARCH_LENGTH, err_angle, err_dot, err_mag, err_vx, err_vy, err_vz, skipLines, skipNodes, issID.str(), issCat.str());
		P_RADAR = SEARCH_LENGTH * P_RADAR_LEVEL;
		testErrorStdev(0.03, 1.5, originalVF, reconVF,skipLines, skipNodes, issID.str(), issCat.str());
		writeVTKFile(DIRECTORY + current_dataset + "/" + issID.str() + " .vtk", reconVF);

		resultsFile << "#" << counter << issCat.str() << ": succeeded" << endl;
		counterFile.open(DIRECTORY + "tests/counter.txt");
		counterFile << " " << counter;
		counterFile.close();
		counter++;
	}
	testFile.close();
}


void showBasicMenu() {
	while (true) {
		cout << "-------------------" << endl
			<< "0-Create Test-Latest" << endl
			<< "1-Create Test-Legacy" << endl
			<< "2-Run Test-Latest" << endl
			<< "3-Run Test-Legacy" << endl
			<< "4-Exit"<<endl;
		int choice;
		cin >> choice;
		switch (choice) {
		case 0:
			createTestsLatest();
			break;
		case 1:
			createTestsLegacy();
			break;
		case 2:
			runTestsLatest();
			break;
		case 3:
			runTestsLegacy();
			break;
		case 4:
			return;
		}
		
	}
}

int main(int argc, char* argv[])
{
	getBatchID();
	showBasicMenu();
	//createTestsExclude();
	//return 0;


	/*if (argc == 1)
		trace_sample_streamline(streamlines);
	else
	{
		readStreamlines(streamlines, argv[1]);
		update_graph(streamlines);
	}
	std::cout << "FINISHED TRACING " << streamlines.size() << std::endl;
	start_visualizer();*/

	/*compareStreamlines("bernard2", "bernard3");
	return 0;
	SignalInit();*/

	
	//runTestsLegacy();
	return 0;

}

/// <summary>
/// Append all errors of a single test to the next ling of the logsID.csv
/// </summary>
/// <param name="testid">readable string (parameters are listed) of the test + parameters</param>
/// <param name="testCategories">only the test's parameters are listed</param>
/// <param name="err_angle">average angle error</param>
/// <param name="err_mag">average magnitude error</param>
/// <param name="err_vx">average vx error</param>
/// <param name="err_vy">average vy error</param>
/// <param name="err_vz">average vz error</param>
/// <param name="skipLines">How many streamlines to skip for each streamline</param>
/// <param name="skipNodes">How many streamline nodes to skip for each node</param>
void saveErrorsToCSV(std::string testid, std::string testCategories, float& err_angle, float& err_mag, float& err_vx, float& err_vy, float& err_vz, int skipLines, int skipNodes)
{
	std::ofstream outfileID, outfile;
	outfileID.open(DIRECTORY + "logsID.csv", std::ios_base::app);
	outfileID << testid << ", " << err_angle << ", " << err_mag << ", " << err_vx << ", " << err_vy << ", " << err_vz << skipLines << skipNodes << std::endl;
	outfile.open(DIRECTORY + "logs.csv", std::ios_base::app);
	outfile << testCategories << ", " << err_angle << ", " << err_mag << ", " << err_vx << ", " << err_vy << ", " << err_vz << skipLines << skipNodes << std::endl;
}


/// <summary>
/// free up vtk structured grid memory
/// </summary>
void clearDataset()
{
	originalVF->Delete();
	reconVF->Delete();
}

/// <summary>
/// Returns a Steady3D field from the vtk structured grid for streamline tracing
/// </summary>
/// <param name="originalVF">original vtk structured grid</param>
/// <returns></returns>
Steady3D buildSteady3D(const vtkSmartPointer<vtkStructuredPoints>& originalVF)
{
	Steady3D steady3D;
	vtk_steady_vectorfield* vtkData = new vtk_steady_vectorfield;
	vtkData->dims = new int[VELOCITY_COMPONENTS];
	vtkData->spacing = new double[VELOCITY_COMPONENTS];
	vtkData->origin = new double[VELOCITY_COMPONENTS];
	vtkData->data_range = new float[2 * VELOCITY_COMPONENTS];

	//TODO: OPTIMIZE
	// Get dimension
	originalVF->GetDimensions(vtkData->dims);

	// Get spacing
	originalVF->GetSpacing(vtkData->spacing);

	// Get the origin
	originalVF->GetOrigin(vtkData->origin);

	vtkData->data_range[0] = vtkData->origin[0];
	vtkData->data_range[2] = vtkData->origin[1];
	vtkData->data_range[4] = vtkData->origin[2];
	vtkData->data_range[1] = vtkData->origin[0] + vtkData->spacing[0] * (vtkData->dims[0] + 1);
	vtkData->data_range[3] = vtkData->origin[1] + vtkData->spacing[1] * (vtkData->dims[1] + 1);
	vtkData->data_range[5] = vtkData->origin[2] + vtkData->spacing[2] * (vtkData->dims[2] + 1);

	// allocate dynamic memory to store velocity values
	vtkData->velocity_data = new float[vtkData->dims[0] * vtkData->dims[1] * vtkData->dims[2] * 3];
	vtkSmartPointer<vtkDataArray> velocity = originalVF->GetPointData()->GetArray("velocity");
	if (velocity == nullptr)
		velocity = originalVF->GetPointData()->GetArray("velocity_normalized");

	for (int k = 0; k < vtkData->dims[2]; k++)
		for (int j = 0; j < vtkData->dims[1]; j++)
			for (int i = 0; i < vtkData->dims[0]; i++)
			{
				int idx = k * vtkData->dims[1] * vtkData->dims[0] + j * vtkData->dims[0] + i;
				double* velo = velocity->GetTuple(idx);
				vtkData->velocity_data[VELOCITY_COMPONENTS * idx + 0] = velo[0];
				vtkData->velocity_data[VELOCITY_COMPONENTS * idx + 1] = velo[1];
				vtkData->velocity_data[VELOCITY_COMPONENTS * idx + 2] = velo[2];
				//std::cout<<velo[0]<<", "<<velo[1]<<", "<<velo[2]<<std::endl;
			}

	steady3D.setDomainDimension(vtkData->dims);
	steady3D.setOrigin(vtkData->origin);
	steady3D.setDataSpace(vtkData->spacing);
	steady3D.setDataRange(vtkData->data_range);
	steady3D.setData(vtkData->velocity_data);

	delete vtkData;

	return steady3D;
}

/// <summary>
/// Reads a vtk structured grid file and save the resulting vtk object to originalVF (a global vtk structured grid object)
/// 
/// </summary>
/// <param name="vtkFile"></param>
void readVTK(std::string vtkFile)
{
	std::cerr << "-------------------" << std::endl;
	std::cerr << "Reading vtk file " << vtkFile << std::endl;
	originalVF = readVTKFile(vtkFile);
	reconVF = vtkSmartPointer<vtkStructuredPoints>::New();
	reconVF->DeepCopy(originalVF);

	int* dims = originalVF->GetDimensions();
	double* spacing = originalVF->GetSpacing();
	SEARCH_LENGTH = (dims[0] * spacing[0] + dims[1] * spacing[1] + dims[2] * spacing[2]) / 250;
	std::cerr << "Search length: " << SEARCH_LENGTH << std::endl;

	reconVF->GetPointData()->RemoveArray("velocity");

	vtkSmartPointer<vtkDoubleArray> velocityN =
		vtkSmartPointer<vtkDoubleArray>::New();
	velocityN->SetName("velocity_normalized");
	velocityN->SetNumberOfComponents(3);
	velocityN->SetNumberOfTuples(originalVF->GetNumberOfPoints());
	reconVF->GetPointData()->AddArray(velocityN);

	vtkSmartPointer<vtkDoubleArray> errors =
		vtkSmartPointer<vtkDoubleArray>::New();
	errors->SetName("errors");
	errors->SetNumberOfComponents(3);
	errors->SetNumberOfTuples(originalVF->GetNumberOfPoints());
	reconVF->GetPointData()->AddArray(errors);

	vtkSmartPointer<vtkDoubleArray> errorsN =
		vtkSmartPointer<vtkDoubleArray>::New();
	errorsN->SetName("errors_normalized");
	errorsN->SetNumberOfComponents(3);
	errorsN->SetNumberOfTuples(originalVF->GetNumberOfPoints());
	reconVF->GetPointData()->AddArray(errorsN);

	vtkSmartPointer<vtkDoubleArray> errorsND =
		vtkSmartPointer<vtkDoubleArray>::New();
	errorsND->SetName("errors_normalized_dotproduct");
	errorsND->SetNumberOfComponents(1);
	errorsND->SetNumberOfTuples(originalVF->GetNumberOfPoints());
	reconVF->GetPointData()->AddArray(errorsND);

	vtkSmartPointer<vtkDoubleArray> stdevs =
		vtkSmartPointer<vtkDoubleArray>::New();
	stdevs->SetName("stdev");
	stdevs->SetNumberOfComponents(1);
	stdevs->SetNumberOfTuples(originalVF->GetNumberOfPoints());
	reconVF->GetPointData()->AddArray(stdevs);

	vtkSmartPointer<vtkIntArray> heatmap =
		vtkSmartPointer<vtkIntArray>::New();
	heatmap->SetName("heatmap");
	heatmap->SetNumberOfComponents(1);
	heatmap->SetNumberOfTuples(originalVF->GetNumberOfPoints());
	reconVF->GetPointData()->AddArray(heatmap);

	/*for (int i = 0; i < 10; i++)
	{
		std::string name = "threshold" + std::to_string(i);
		vtkSmartPointer<vtkShortArray> field =
				vtkSmartPointer<vtkShortArray>::New();
		field->SetName(name.c_str());
		field->SetNumberOfComponents(1);
		field->SetNumberOfTuples(originalVF->GetNumberOfPoints());
		reconVF->GetPointData()->AddArray(field);
	}*/
}

/// <summary>
/// Convert the current set of all streamline nodes into a set of vtkPolydata points. Then build a vtkPointLocator with the set of points (currently using KDTree)
/// </summary>
void buildSegmentsPolyData()
{
	vtkSmartPointer<vtkPoints> points =
		vtkSmartPointer<vtkPoints>::New();
	for (int i = 0; i < total_nodes; i++)
	{
		segment* s_next = all_segments[i];
		node* n = s_next->middle;
		points->InsertNextPoint(n->x, n->y, n->z);
	}
	std::cerr << "Done reading segments" << std::endl;
	vtkSmartPointer<vtkPolyData> pointSet = vtkSmartPointer<vtkPolyData>::New();
	pointSet->SetPoints(points);
	std::cerr << "Setting kdtree" << std::endl;
	pointLocatorSL = PointLocator(pointSet);
}

/// <summary>
/// Convert the current set of all streamline nodes into a set of vtkPolydata points. Then build a vtkPointLocator with the set of points (currently using KDTree)
/// </summary>
/// <param name="streamlines"></param>
/// <returns></returns>
vtkSmartPointer<vtkPolyData> buildStreamlinesPolyData(std::vector<std::vector<node>> streamlines)
{

	// Create a vtkPoints object and store the points in it
	vtkSmartPointer<vtkPoints> points =
		vtkSmartPointer<vtkPoints>::New();

	// Create a cell array to store the lines in and add the lines to it
	vtkSmartPointer<vtkCellArray>
		lines =
		vtkSmartPointer<vtkCellArray>::New();

	//std::cerr << "size: " << streamlines.size() << std::endl;
	int counter = 0;
	for (int i = 0; i < streamlines.size(); i++)
	{
		//std::cerr << segment_starters[i]->global_index << std::endl;
		std::vector<node> streamline = streamlines[i];
		//std::cerr << "size: " << streamline.size() << std::endl;
		for (int j = 0; j < streamline.size(); j++)
		{
			node n = streamline[j];
			points->InsertNextPoint(n.x, n.y, n.z);

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
	}

	// Create a polydata to store everything in
	vtkSmartPointer<vtkPolyData> linesPolyData =
		vtkSmartPointer<vtkPolyData>::New();
	std::cerr << points->GetNumberOfPoints() << ", " << lines->GetNumberOfCells() << std::endl;
	// Add the points to the dataset
	linesPolyData->SetPoints(points);

	// Add the lines to the dataset
	linesPolyData->SetLines(lines);

	return linesPolyData;
}

void compareStreamlines(std::string slFile1, std::string slFile2)
{
	std::vector<std::vector<node>> streamlines1, streamlines2;
	std::string DIR = DIRECTORY;
	readStreamlines(streamlines1, slFile1 + ".txt", 1, 1);
	//writeSLFile(DIR + "streamlines/" + slFile1 + "SL.vtk", buildStreamlinesPolyData(streamlines1));

	readStreamlines(streamlines2, slFile2 + ".txt", 1, 1);
	int numcells = 0;
	for (int i = 0; i < streamlines2.size(); i++)
	{
		//std::cerr << segment_starters[i]->global_index << std::endl;
		std::vector<node> streamline = streamlines2[i];
		numcells += streamline.size();
	}
	//std::cout << numcells << std::endl;

	// Create a vtkPoints object and store the points in it
	vtkSmartPointer<vtkPoints> points =
		vtkSmartPointer<vtkPoints>::New();

	// Create a cell array to store the lines in and add the lines to it
	vtkSmartPointer<vtkCellArray>
		lines =
		vtkSmartPointer<vtkCellArray>::New();

	vtkSmartPointer<vtkDoubleArray> errorsV =
		vtkSmartPointer<vtkDoubleArray>::New();
	errorsV->SetName("errors_velocity");
	errorsV->SetNumberOfComponents(3);
	errorsV->SetNumberOfTuples(numcells);

	vtkSmartPointer<vtkDoubleArray> errorsP =
		vtkSmartPointer<vtkDoubleArray>::New();
	errorsP->SetName("errors_pos");
	errorsP->SetNumberOfComponents(3);
	errorsP->SetNumberOfTuples(numcells);

	//	std::cerr << "size: " << streamlines.size
	float avg_err = 0;
	int counter = 0, counted = 0, missed = 0;
	for (int i = 0; i < streamlines2.size(); i++)
	{
		//std::cerr << segment_starters[i]->global_index << std::endl;
		std::vector<node> streamline = streamlines2[i];
		std::vector<node> streamline_org = streamlines1[i];
		//std::cerr << "size: " << streamline.size() << std::endl;
		for (int j = 0; j < streamline.size(); j++)
		{
			node n = streamline[j];
			//std::cerr << n.x << ", ";
			points->InsertNextPoint(n.x, n.y, n.z); //InsertNextPoint(i, 1, j); //
			if (j < streamline_org.size())
			{
				counted++;
				node n_org = streamline_org[j];
				float vx_diff = abs(n_org.vx - n.vx),
					vy_diff = abs(n_org.vy - n.vy),
					vz_diff = abs(n_org.vz - n.vz),
					x_diff = abs(n_org.x - n.x),
					y_diff = abs(n_org.y - n.y),
					z_diff = abs(n_org.z - n.z);
				avg_err += vx_diff + vy_diff + vz_diff;
				float errorV[3] = { vx_diff, vy_diff, vz_diff },
					errorP[3] = { x_diff, y_diff, z_diff };
				//std::cout << error[0] << error[1] << error[2] << std::endl;
				int a = j + counter;
				errorsV->SetComponent(a, 0, vx_diff);
				errorsV->SetComponent(a, 1, vy_diff);
				errorsV->SetComponent(a, 2, vz_diff);

				errorsP->SetComponent(a, 0, x_diff);
				errorsP->SetComponent(a, 1, y_diff);
				errorsP->SetComponent(a, 2, z_diff);
			}
			else
			{
				missed++;
				int a = j + counter;
				errorsV->SetComponent(a, 0, 0);
				errorsV->SetComponent(a, 1, 0);
				errorsV->SetComponent(a, 2, 0);

				errorsP->SetComponent(a, 0, 0);
				errorsP->SetComponent(a, 1, 0);
				errorsP->SetComponent(a, 2, 0);
				//float error[3] = {0, 0, 0};
			}

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
	//std::cout << "average err: " << avg_err / counted << std::endl
	//					<< "missed: " << missed << std::endl;

	// Create a polydata to store everything in
	vtkSmartPointer<vtkPolyData> linesPolyData =
		vtkSmartPointer<vtkPolyData>::New();
	std::cerr << points->GetNumberOfPoints() << ", " << lines->GetNumberOfCells() << std::endl;
	// Add the points to the dataset
	linesPolyData->SetPoints(points);

	// Add the lines to the dataset
	linesPolyData->SetLines(lines);

	linesPolyData->GetPointData()->AddArray(errorsV);
	linesPolyData->GetPointData()->AddArray(errorsP);
	linesPolyData->GetPointData()->SetActiveScalars("errors_velocity");

	writeSLFile(DIR + "streamlines/" + slFile2 + "SL.vtk", linesPolyData);
}

std::vector<Vector3f> segmentStarterstoSeedingPoints()
{
	std::vector<Vector3f> points;
	for (int i = 0; i < segment_starters.size(); i++)
	{
		node* n = segment_starters[i]->start;
		points.push_back(Vector3f(n->x, n->y, n->z));
	}
	return points;
}

void compareStreamlines(std::vector<std::vector<Vector3f>> orgSL, std::vector<std::vector<Vector3f>> compSL, std::string name, bool writeOrg = false)
{
	std::vector<std::vector<node>> streamlines1, streamlines2;
	std::string DIR =DIRECTORY;
	readStreamlines(streamlines1, orgSL);

	if (writeOrg)
		writeSLFile(DIR + "streamlines/" + name + "_orgSL.vtk", buildStreamlinesPolyData(streamlines1));

	readStreamlines(streamlines2, compSL);
	int numcells = 0;
	for (int i = 0; i < streamlines2.size(); i++)
	{
		//std::cerr << segment_starters[i]->global_index << std::endl;
		std::vector<node> streamline = streamlines2[i];
		numcells += streamline.size();
	}
	//std::cout << numcells << std::endl;

	// Create a vtkPoints object and store the points in it
	vtkSmartPointer<vtkPoints> points =
		vtkSmartPointer<vtkPoints>::New();

	// Create a cell array to store the lines in and add the lines to it
	vtkSmartPointer<vtkCellArray>
		lines =
		vtkSmartPointer<vtkCellArray>::New();

	vtkSmartPointer<vtkDoubleArray> errorsV =
		vtkSmartPointer<vtkDoubleArray>::New();
	errorsV->SetName("errors_velocity");
	errorsV->SetNumberOfComponents(3);
	errorsV->SetNumberOfTuples(numcells);

	vtkSmartPointer<vtkDoubleArray> errorsP =
		vtkSmartPointer<vtkDoubleArray>::New();
	errorsP->SetName("errors_pos");
	errorsP->SetNumberOfComponents(3);
	errorsP->SetNumberOfTuples(numcells);

	//	std::cerr << "size: " << streamlines.size
	float avg_err = 0;
	int counter = 0, counted = 0, missed = 0;
	for (int i = 0; i < streamlines2.size(); i++)
	{
		//std::cerr << segment_starters[i]->global_index << std::endl;
		std::vector<node> streamline = streamlines2[i];
		std::vector<node> streamline_org = streamlines1[i];
		//std::cerr << "size: " << streamline.size() << std::endl;
		for (int j = 0; j < streamline.size(); j++)
		{
			node n = streamline[j];
			//std::cerr << n.x << ", ";
			points->InsertNextPoint(n.x, n.y, n.z); //InsertNextPoint(i, 1, j); //
			if (j < streamline_org.size())
			{
				counted++;
				node n_org = streamline_org[j];
				float vx_diff = abs(n_org.vx - n.vx),
					vy_diff = abs(n_org.vy - n.vy),
					vz_diff = abs(n_org.vz - n.vz),
					x_diff = abs(n_org.x - n.x),
					y_diff = abs(n_org.y - n.y),
					z_diff = abs(n_org.z - n.z);
				avg_err += vx_diff + vy_diff + vz_diff;
				float errorV[3] = { vx_diff, vy_diff, vz_diff },
					errorP[3] = { x_diff, y_diff, z_diff };
				//std::cout << error[0] << error[1] << error[2] << std::endl;
				int a = j + counter;
				errorsV->SetComponent(a, 0, vx_diff);
				errorsV->SetComponent(a, 1, vy_diff);
				errorsV->SetComponent(a, 2, vz_diff);

				errorsP->SetComponent(a, 0, x_diff);
				errorsP->SetComponent(a, 1, y_diff);
				errorsP->SetComponent(a, 2, z_diff);
			}
			else
			{
				missed++;
				int a = j + counter;
				errorsV->SetComponent(a, 0, 0);
				errorsV->SetComponent(a, 1, 0);
				errorsV->SetComponent(a, 2, 0);

				errorsP->SetComponent(a, 0, 0);
				errorsP->SetComponent(a, 1, 0);
				errorsP->SetComponent(a, 2, 0);
				//float error[3] = {0, 0, 0};
			}

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
	//std::cout << "average err: " << avg_err / counted << std::endl
	//					<< "missed: " << missed << std::endl;

	// Create a polydata to store everything in
	vtkSmartPointer<vtkPolyData> linesPolyData =
		vtkSmartPointer<vtkPolyData>::New();
	//std::cerr << points->GetNumberOfPoints() << ", " << lines->GetNumberOfCells() << std::endl;
	// Add the points to the dataset
	linesPolyData->SetPoints(points);

	// Add the lines to the dataset
	linesPolyData->SetLines(lines);

	linesPolyData->GetPointData()->AddArray(errorsV);
	linesPolyData->GetPointData()->AddArray(errorsP);
	linesPolyData->GetPointData()->SetActiveScalars("errors_velocity");

	writeSLFile(DIR + "streamlines/" + name + "reconSL.vtk", linesPolyData);
}





const int NUM_THREADS = 11;

/// <summary>
/// Main reconstruction method using the latest method. Keep track of average error values and log them accordingly
/// </summary>
/// <param name="thresholdSize"></param>
/// <param name="maxThreshold"></param>
/// <param name="originalVF"></param>
/// <param name="reconVF"></param>
/// <param name="skipLines"></param>
/// <param name="skipNodes"></param>
/// <param name="testid"></param>
/// <param name="testCategories"></param>
void testErrorStdev(float thresholdSize, float maxThreshold, vtkSmartPointer<vtkStructuredPoints> originalVF, vtkSmartPointer<vtkStructuredPoints> reconVF, int skipLines, int skipNodes,
	std::string testid, std::string testCategories)
{
	float err_angle, err_mag, err_vx, err_vy, err_vz, err_dot;

	std::cout << testid << std::endl;
	auto start = high_resolution_clock::now();
	std::cerr << "-------------------" << std::endl
		<< "Parameters: "
		//<< "doNormalizeCheck: " << doNormalizeCheck << ", doNormalizeRecon: " << doNormalizeRecon
		<< ", useSegment: " << P_USE_SEGMENTS
		<< ", thresholdAngle: " << P_THRESHOLD_ANGLE << ", radar: " << P_RADAR
		<< ", thresholdVelocity: " << P_THRESHOLD_VELOCITY << std::endl;

	RECON_EXCLUDED = err_angle = err_mag = err_vx = err_vy = err_vz = err_dot = 0;
	int err_angle_count = 0;

	vtkSmartPointer<vtkDataArray> velocity = originalVF->GetPointData()->GetArray("velocity");
	//vtkSmartPointer<vtkDataArray> velocity2 = reconVF->GetPointData()->GetArray("velocity");
	vtkSmartPointer<vtkDataArray> errors = reconVF->GetPointData()->GetArray("errors");
	vtkSmartPointer<vtkDataArray> velocityN = reconVF->GetPointData()->GetArray("velocity_normalized");
	vtkSmartPointer<vtkDataArray> errorsN = reconVF->GetPointData()->GetArray("errors_normalized");
	vtkSmartPointer<vtkDataArray> errorsND = reconVF->GetPointData()->GetArray("errors_normalized_dotproduct");
	vtkSmartPointer<vtkDataArray> heatmap = reconVF->GetPointData()->GetArray("heatmap");
	vtkSmartPointer<vtkDataArray> stdevs = reconVF->GetPointData()->GetArray("stdev");
	pointLocatorStdev.velocity = velocityN;
	pointLocatorStdev.idMap.clear();
	pointLocatorStdev.ready = false;
	pointLocatorStdev.heatmap = heatmap;
	/*vtkSmartPointer<vtkDataArray> thresholds[10];
	for (int i = 0; i < 10; i++)
	{
		std::string name = "threshold" + std::to_string(i);
		thresholds[i] = reconVF->GetPointData()->GetArray(name.c_str());
	}*/
	int counter = 0;
	int zeroes = 0;
	vtkSmartPointer<vtkPoints> points =
		vtkSmartPointer<vtkPoints>::New();

	//vector<double*> pts_thread[NUM_THREADS];
	//vector<vtkIdType> idx_thread[NUM_THREADS];

	for (int i = 0; i < NUM_THREADS; i++) {
		//points_thread[i] = vtkSmartPointer<vtkPoints>::New();
	}


	//retrieve the entries from the grid and print them to the screen
#pragma omp parallel for schedule(dynamic, 11)
	for (vtkIdType i = 0; i < originalVF->GetNumberOfPoints(); i++)
	{
		double p[3];
		originalVF->GetPoint(i, p);
		float stdev = recon_get_stdev(p, P_RADAR);
		//std::cerr << stdev << ", ";
		stdevs->SetComponent(i, 0, stdev);
		heatmap->SetComponent(i, 0, 0);
		//points->InsertNextPoint(p);
	}

	//update stdev thresholds
	/*for (int t = 1; t < 11; t++)
	{
		float threshold = 0.2 * t;
		float preThreshold = threshold - 0.2;
		for (int i = 0; i < originalVF->GetNumberOfPoints(); i++)
		{
			double stdev = stdevs->GetTuple(i)[0];
			int value = 0;
			if (stdev > preThreshold && stdev < threshold)
				value = 1;
			thresholds[t - 1]->SetComponent(i, 0, value);
		}
	}*/
	auto pointSet = vtkSmartPointer<vtkPolyData>::New();
	int times = maxThreshold / thresholdSize;
	//cout << times << ", " << maxThreshold << ", " << thresholdSize << endl;
	for (int t = 1; t <= times; t++)
	{

		float threshold = thresholdSize * t;
		float preThreshold = threshold - thresholdSize;
		//cout << preThreshold << " - " << threshold << endl;
		if (t >= 2)
		{
			//std::cerr << "BUILDING";
			pointLocatorStdev.ready = true;
			//std::cout << "POINTS: " << points->GetNumberOfPoints() << endl;

			pointSet->SetPoints(points);
			//pointLocatorSL = PointLocator(pointSet);
			pointLocatorStdev.buildLocator(pointSet);
			//std::cerr << "DONE!!";
		}
#pragma omp parallel for schedule(dynamic, 11)// num_threads(NUM_THREADS)
		for (int i = 0; i < originalVF->GetNumberOfPoints(); i++)
		{
			//cout << omp_get_num_threads() << endl;
			double stdev = stdevs->GetTuple(i)[0];
			if (!(stdev >= preThreshold && stdev < threshold))
				continue;

			double* pt = originalVF->GetPoint(i);
			double* v = velocity->GetTuple(i);
			float ovx = v[0], ovy = v[1], ovz = v[2];
			//normalize
			/*if (doNormalizeCheck)
			{
				float mag = sqrt(ovx * ovx + ovy * ovy + ovz * ovz);
				ovx /= mag;
				ovy /= mag;
				ovz /= mag;
			}*/

			float vx, vy, vz;
			//float stdev;
			//int num = interpolate_streamline_selective(pt[0], pt[1], pt[2], vx, vy, vz, radar, doNormalizeRecon, thresholdAngle, thresholdVelocity, useSegment);
			float meh;
			interpolate_streamline_selective_weighted_sum_stdev(meh, pt[0], pt[1], pt[2], vx, vy, vz);

			if (vx == 0 && vy == 0 && vz == 0)
				zeroes++;

			float vx_diff = abs(vx - ovx),
				vy_diff = abs(vy - ovy),
				vz_diff = abs(vz - ovz);
			err_mag += sqrt(vx_diff * vx_diff + vy_diff * vy_diff + vz_diff * vz_diff);

			float mag = sqrt(ovx * ovx + ovy * ovy + ovz * ovz);
			float magT = sqrt(vx * vx + vy * vy + vz * vz);
			float vx2 = vx / magT, vy2 = vy / magT, vz2 = vz / magT;
			float vx_diff2 = abs(vx2 - ovx / mag);
			float vy_diff2 = abs(vy2 - ovy / mag);
			float vz_diff2 = abs(vz2 - ovz / mag);

			float angle_diff = getAbsoluteDiff3Angles(ovx / mag, ovy / mag, ovz / mag, vx2, vy2, vz2);
			err_vx += vx_diff2;
			err_vy += vy_diff2;
			err_vz += vz_diff2;
			//TODO: fix NAN
			float v1[3] = { ovx / mag, ovy / mag, ovz / mag }, v2[3] = { vx2, vy2, vz2 };
			float err_dot_new = dot_product(v1, v2);
			err_dot += err_dot_new;



#pragma omp critical
			{
				errors->SetComponent(i, 0, vx_diff);
				errors->SetComponent(i, 1, vy_diff);
				errors->SetComponent(i, 2, vz_diff);
				velocityN->SetComponent(i, 0, vx2);
				velocityN->SetComponent(i, 1, vy2);
				velocityN->SetComponent(i, 2, vz2);
				errorsN->SetComponent(i, 0, vx_diff2);
				errorsN->SetComponent(i, 1, vy_diff2);
				errorsN->SetComponent(i, 2, vz_diff2);

				errorsND->SetComponent(i, 0, err_dot_new);

				///int tid = omp_get_thread_num();
				//pts_thread[tid].push_back(pt);
				//idx_thread[tid].push_back(i);


				points->InsertNextPoint(pt);
				pointLocatorStdev.idMap.push_back(i);

				//std::cerr << i << ", ";
				/*velocity2->SetComponent(i, 0, vx);
			velocity2->SetComponent(i, 1, vy);
			velocity2->SetComponent(i, 2, vz);*/

			//errorsND->SetComponent(i, 0, 0);
			//stdevs->SetComponent(i, 0, stdev);
			/////////////////
			vtkIdType id;
			//pointLocatorStdev.pointTree->InsertPointWithoutChecking(pt, id, 1);


			if (!isnan(angle_diff))
			{
				err_angle_count++;
				err_angle += angle_diff;
				if (err_angle == NAN)
					std::cerr << angle_diff;
			}
			else
			{
				//std::cout << "error computing " << v[0] << ", " << v[1] << ", " << v[2] << ", " << vx << ", " << vy << ", " << vz << std::endl;
			}

			counter++;
			if (counter % (originalVF->GetNumberOfPoints() / 10) == 0)
			{
				int progress = (float(counter) / originalVF->GetNumberOfPoints()) * 100;

				std::cerr << progress << ",";
			}
			}
		}

		/*for (int i = 0; i < NUM_THREADS; i++) {
			auto pts = pts_thread[i];
			auto idx = idx_thread[i];
			for (int a = 0; a < pts.size(); a++) {
				points->InsertNextPoint(pts[a]);
				pointLocatorStdev.idMap.push_back(idx[a]);
			}
			pts.clear();
			idx.clear();
		}*/
	}
	//std::cout << "ZEROES: " << zeroes << endl;
	std::cerr << std::endl;
	err_vx /= counter;
	err_vy /= counter;
	err_vz /= counter;
	err_mag /= counter;
	err_angle /= err_angle_count;
	std::cerr << "avg angle: " << angle_diff_avg / angle_diff_c << std::endl;
	std::cerr << "err_vx: " << err_vx << std::endl;
	std::cerr << "err_vy: " << err_vy << std::endl;
	std::cerr << "err_vz: " << err_vz << std::endl;
	std::cerr << "err_mag: " << err_mag << std::endl;
	std::cerr << "err_angle: " << err_angle << std::endl;
	std::cerr << "excluded_pts: " << RECON_EXCLUDED << std::endl;
	auto stop = high_resolution_clock::now();
	auto duration = duration_cast<seconds>(stop - start);
	std::cerr << "NF Duration: " << duration.count() << std::endl;
	saveErrorsToCSV(testid, testCategories, err_angle, err_mag, err_vx, err_vy, err_vz, skipLines, skipNodes);
}


/// <summary>
/// Running legacy reconstruction method and keep track of average errors,then log
/// TODO: NEED FIX. Currently unused as legacy method is being called from the newer test function!
/// </summary>
/// <param name="originalVF"></param>
/// <param name="reconVF"></param>
/// <param name="doNormalizeCheck"></param>
/// <param name="doNormalizeRecon"></param>
/// <param name="thresholdAngle"></param>
/// <param name="thresholdVelocity"></param>
/// <param name="useSegment"></param>
/// <param name="radar"></param>
/// <param name="err_angle"></param>
/// <param name="err_dot"></param>
/// <param name="err_mag"></param>
/// <param name="err_vx"></param>
/// <param name="err_vy"></param>
/// <param name="err_vz"></param>
/// <param name="skipLines"></param>
/// <param name="skipNodes"></param>
/// <param name="testid"></param>
/// <param name="testCategories"></param>
void testErrorLegacy(vtkSmartPointer<vtkStructuredPoints> originalVF, vtkSmartPointer<vtkStructuredPoints> reconVF,
	bool doNormalizeCheck, bool doNormalizeRecon, float thresholdAngle, float thresholdVelocity, bool useSegment,
	float radar, float& err_angle, float& err_dot, float& err_mag, float& err_vx, float& err_vy, float& err_vz, int skipLines, int skipNodes,
	std::string testid, std::string testCategories)
{
	auto start = high_resolution_clock::now();
	std::cerr << "-------------------" << std::endl
		<< "Parameters: "
		<< "doNormalizeCheck: " << doNormalizeCheck << ", doNormalizeRecon: " << doNormalizeRecon
		<< ", useSegment: " << useSegment
		<< ", thresholdAngle: " << thresholdAngle << ", radar: " << radar
		<< ", thresholdVelocity: " << thresholdVelocity << std::endl;

	err_angle = err_mag = err_vx = err_vy = err_vz = err_dot = 0;
	int err_angle_count = 0;

	vtkSmartPointer<vtkDataArray> velocity = originalVF->GetPointData()->GetArray("velocity");
	//vtkSmartPointer<vtkDataArray> velocity2 = reconVF->GetPointData()->GetArray("velocity");
	vtkSmartPointer<vtkDataArray> errors = reconVF->GetPointData()->GetArray("errors");
	vtkSmartPointer<vtkDataArray> velocityN = reconVF->GetPointData()->GetArray("velocity_normalized");
	vtkSmartPointer<vtkDataArray> errorsN = reconVF->GetPointData()->GetArray("errors_normalized");
	vtkSmartPointer<vtkDataArray> errorsND = reconVF->GetPointData()->GetArray("errors_normalized_dotproduct");

	//Legacy test dont use stdev
	//vtkSmartPointer<vtkDataArray> stdevs = reconVF->GetPointData()->GetArray("stdev");

	/*vtkSmartPointer<vtkDataArray> thresholds[10];
	for (int i = 0; i < 10; i++)
	{
		std::string name = "threshold" + std::to_string(i);
		thresholds[i] = reconVF->GetPointData()->GetArray(name.c_str());
	}*/
	int counter = 0;

	//Legacy test dont use stdev
	/*std::cerr << "Progress: ";
	vtkSmartPointer<vtkPoints> points =
			vtkSmartPointer<vtkPoints>::New();

	//retrieve the entries from the grid and print them to the screen
	for (vtkIdType i = 0; i < originalVF->GetNumberOfPoints(); i++)
	{
		double p[3];
		originalVF->GetPoint(i, p);

		points->InsertNextPoint(p);
	}
	pointLocatorStdev.pointTree->SetDataSet(originalVF);
	pointLocatorStdev.pointTree->InitPointInsertion(points, points->GetBounds()); //, points->GetNumberOfPoints());*/
#pragma omp parallel for schedule(dynamic, 1)
	for (int i = 0; i < originalVF->GetNumberOfPoints(); i++)
	{

		double* pt = originalVF->GetPoint(i);
		double* v = velocity->GetTuple(i);
		float ovx = v[0], ovy = v[1], ovz = v[2];
		//normalize
		if (doNormalizeCheck)
		{
			float mag = sqrt(ovx * ovx + ovy * ovy + ovz * ovz);
			ovx /= mag;
			ovy /= mag;
			ovz /= mag;
		}

		float vx, vy, vz;

		//radar, doNormalizeRecon, thresholdAngle, thresholdVelocity, useSegment
		float stdev = interpolate_streamline_selective_weighted_sum_stdev(stdev, pt[0], pt[1], pt[2], vx, vy, vz);

		float vx_diff = abs(vx - ovx),
			vy_diff = abs(vy - ovy),
			vz_diff = abs(vz - ovz);
		err_mag += sqrt(vx_diff * vx_diff + vy_diff * vy_diff + vz_diff * vz_diff);

		float mag = sqrt(ovx * ovx + ovy * ovy + ovz * ovz);
		float magT = sqrt(vx * vx + vy * vy + vz * vz);
		float vx2 = vx / magT, vy2 = vy / magT, vz2 = vz / magT;
		float vx_diff2 = abs(vx2 - ovx / mag);
		float vy_diff2 = abs(vy2 - ovy / mag);
		float vz_diff2 = abs(vz2 - ovz / mag);

		float angle_diff = getAbsoluteDiff3Angles(ovx / mag, ovy / mag, ovz / mag, vx2, vy2, vz2);
		err_vx += vx_diff2;
		err_vy += vy_diff2;
		err_vz += vz_diff2;
		//TODO: fix NAN
		float v1[3] = { ovx / mag, ovy / mag, ovz / mag }, v2[3] = { vx2, vy2, vz2 };
		float err_dot_new = dot_product(v1, v2);
		err_dot += err_dot_new;

		errors->SetComponent(i, 0, vx_diff);
		errors->SetComponent(i, 1, vy_diff);
		errors->SetComponent(i, 2, vz_diff);
		velocityN->SetComponent(i, 0, vx2);
		velocityN->SetComponent(i, 1, vy2);
		velocityN->SetComponent(i, 2, vz2);
		errorsN->SetComponent(i, 0, vx_diff2);
		errorsN->SetComponent(i, 1, vy_diff2);
		errorsN->SetComponent(i, 2, vz_diff2);

		errorsND->SetComponent(i, 0, err_dot_new);

#pragma omp critical
		{
			if (angle_diff != NAN)
			{
				err_angle_count++;
				err_angle += angle_diff;
				if (err_angle == NAN)
					std::cerr << angle_diff;
			}
			else
			{
				//std::cerr << "error computing " << v[0] << ", " << v[1] << ", " << v[2] << ", " << vx << ", " << vy << ", " << vz << std::endl;
			}

			counter++;
			if (counter % (originalVF->GetNumberOfPoints() / 10) == 0)
			{
				int progress = (float(counter) / originalVF->GetNumberOfPoints()) * 100;

				std::cerr << progress << ",";
			}
		}
	}

	//Legacy test dont use stdev
	//update stdev thresholds
	/*for (int t = 1; t < 11; t++)
	{
		float threshold = 0.2 * t;
		float preThreshold = threshold - 0.2;
		for (int i = 0; i < originalVF->GetNumberOfPoints(); i++)
		{
			double stdev = stdevs->GetTuple(i)[0];
			int value = 0;
			if (stdev > preThreshold && stdev < threshold)
				value = 1;
			thresholds[t - 1]->SetComponent(i, 0, value);
		}
	}*/

	std::cerr << std::endl;
	err_vx /= counter;
	err_vy /= counter;
	err_vz /= counter;
	err_mag /= counter;
	err_angle /= err_angle_count;
	std::cerr << "avg angle: " << angle_diff_avg / angle_diff_c << std::endl;
	std::cerr << "err_vx: " << err_vx << std::endl;
	std::cerr << "err_vy: " << err_vy << std::endl;
	std::cerr << "err_vz: " << err_vz << std::endl;
	std::cerr << "err_mag: " << err_mag << std::endl;
	std::cerr << "err_angle: " << err_angle << std::endl;
	auto stop = high_resolution_clock::now();
	auto duration = duration_cast<seconds>(stop - start);
	std::cerr << "NF Duration: " << duration.count() << std::endl;
	saveErrorsToCSV(testid, testCategories, err_angle, err_mag, err_vx, err_vy, err_vz, skipLines, skipNodes);
}


/// <summary>
/// trace streamline from the vectorfield
/// </summary>
/// <param name="vectorField"></param>
/// <param name="seedingCurve"></param>
/// <returns></returns>
std::vector<std::vector<Vector3f>> traceSL(Steady3D& vectorField, std::vector<Vector3f> seedingCurve)
{
	StreamlineTracer streamlineTracer(vectorField);
	float stepsize = SEARCH_LENGTH / 5,
		sl_length = SEARCH_LENGTH * 20;
	streamlineTracer.setStepSize(stepsize);

	std::vector<std::vector<Vector3f>> allStreamlines;
	for (int i = 0; i < seedingCurve.size(); i++)
	{
		// forward
		streamlineTracer.setForward(true);
		vector<Vector3f> streamline = streamlineTracer.trace(seedingCurve[i], sl_length);
		allStreamlines.push_back(streamline);
		// backward
		streamlineTracer.setForward(false);
		streamline = streamlineTracer.trace(seedingCurve[i], sl_length);
		allStreamlines.push_back(streamline);
	}

	return allStreamlines;
}

float random_range(float LO, float HI)
{
	float r3 = LO + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (HI - LO)));
	return r3;
}

/// <summary>
/// return a random seeding curve from the vectorfield
/// </summary>
/// <param name="s_org">the original vectorfield</param>
/// <param name="numSL">number of streamlines</param>
/// <returns></returns>
std::vector<Vector3f> buildRandomSeedingCurve(Steady3D& s_org, int numSL)
{
	float xRange[2]; // the minimum and maximum values of x-coordinates, not velocity!
	float yRange[2]; // the minimum and maximum values of y-coordinates, not velocity!
	float zRange[2];
	s_org.getXRange(xRange);
	s_org.getYRange(yRange);
	s_org.getZRange(zRange);

	std::vector<Vector3f> seedingCurve;

	for (int i = 0; i < numSL; i++)
	{
		float randX = random_range(xRange[0], xRange[1]);
		float randY = random_range(yRange[0], yRange[1]);
		float randZ = random_range(zRange[0], zRange[1]);
		seedingCurve.push_back(Vector3f(randX, randY, randZ));
	}

	//vector<vector<Vector3f>> streamlines_org = traceSL(s_org, seedingCurve), streamlines_recon = traceSL(s_recon, seedingCurve);

	return seedingCurve;
	//compareStreamlines(streamlines_org, streamlines_recon, name);
}