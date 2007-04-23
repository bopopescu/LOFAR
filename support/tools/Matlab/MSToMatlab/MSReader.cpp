#include "MSReader.h"

#include "MSInfo.h"

#include <tables/Tables.h>
#include <tables/Tables/TableIter.h>

#include <casa/Arrays.h>
#include <casa/aipstype.h>
#include <casa/complex.h>
#include <casa/BasicMath/Math.h>

#include <iomanip>

using namespace std;
using namespace casa;

MSReader::MSReader(const string& msName):
	msInfo()
{
  ms = new MeasurementSet(msName, Table::Update);

	// Get the antenne column
	MSAntenna antennae = (*ms).antenna();
	msInfo.nAntennae = antennae.nrow();

	// Get the dataDescription column
	MSDataDescription dataDescription = (*ms).dataDescription();

	// The amount of data descriptions tells us
	// how many lines we have to skip for example from one timeslot to the next
	// when indexing the MeasurementSet
	bandsPerTimeSlot = dataDescription.nrow();

	// Filling the MeasurementSet Info with the rest of the metadata
	MSPolarization polarization = (*ms).polarization();
	ROScalarColumn<Int> NUM_CORR_col(polarization, "NUM_CORR");
	msInfo.nPolarizations = NUM_CORR_col(0);

	

	msInfo.validInfo = true;
}

MSInfo MSReader::getMSInfo()
{
	return msInfo;
}

Cube<complex<float> > MSReader::getTimeCube(int timeSlot, int selectedBand, int polarizationID, int startFreq, int stopFreq)
{
	// RO => Read Only
	int nAntennae = msInfo.nAntennae;

	Cube<complex<float> > cube(nAntennae, nAntennae, stopFreq - startFreq);
	
	ROArrayColumn<complex<float > > dataColumn((*ms), "DATA");
	ROScalarColumn<Int> antenna1Column((*ms), "ANTENNA1");
	ROScalarColumn<Int> antenna2Column((*ms), "ANTENNA2");
	ROScalarColumn<Double> timeCentroidColumn((*ms), "TIME_CENTROID");


	int baseLines = (nAntennae * nAntennae + nAntennae) / 2;
	int indexFirstBaseLine = (timeSlot*bandsPerTimeSlot+selectedBand) * baseLines;

	cout << endl << "Rownumber of first baseline of slot is " << indexFirstBaseLine << flush << endl;

	for(int i=0; i< baseLines ; i++)
	{
		int rowIndex = i + indexFirstBaseLine;
		Int antenna1 = antenna1Column(rowIndex);
		Int antenna2 = antenna2Column(rowIndex);
		Double timeCentroid = timeCentroidColumn(rowIndex);

		// Get the 2d array (matrix). Size is expected to be nAntennae * nPolarizations
		Matrix<complex<float > > matrix = dataColumn(rowIndex);

		// Get the measurements from matrix 'data' for given polarization
		Vector<complex<float> > measurements = matrix.row(polarizationID);

		// We have to map the frequency range (ex. 10-20) to the cube (0-10)
		int thirdIndexCube = 0;
		for(int f = startFreq ; f < stopFreq; f++)
		{
			// Copy the measurements for 1 polarization and one baseline. 
			cube(antenna1, antenna2, thirdIndexCube++) = measurements(f);
		}
	}
	return cube;
}

Cube<complex<float> > MSReader::getFrequencyCube(int frequency, int selectedBand, int polarizationID, int startTimeSlot, int stopTimeSlot)
{
	int nAntennae = msInfo.nAntennae;
	
	Cube<complex<float> > cube(nAntennae, nAntennae, stopTimeSlot - startTimeSlot);
	
	ROArrayColumn<complex<float > > dataColumn((*ms), "DATA");
	ROScalarColumn<Int> antenna1Column((*ms), "ANTENNA1");
	ROScalarColumn<Int> antenna2Column((*ms), "ANTENNA2");
	ROScalarColumn<Double> timeCentroidColumn((*ms), "TIME_CENTROID");


	int baseLines = (nAntennae * nAntennae + nAntennae) / 2;
	
	for(int t=startTimeSlot; t<stopTimeSlot; t++)
	{
		for(int i=0; i< baseLines; i++)
		{
			int indexFirstBaseLine = (t*bandsPerTimeSlot+selectedBand) * baseLines;
			int rowIndex = i + indexFirstBaseLine;
			Int antenna1 = antenna1Column(rowIndex);
			Int antenna2 = antenna2Column(rowIndex);
			Double timeCentroid = timeCentroidColumn(rowIndex);
	
			// Get the 2d array (matrix). Size is expected to be nAntennae * nPolarizations
			Matrix<complex<float > > matrix = dataColumn(rowIndex);
	
			// Get the measurements from matrix 'data' for given polarization
			Vector<complex<float> > measurements = matrix.row(polarizationID);

			// This place also needs mapping (does it?)
			cube(antenna1, antenna2, t) = measurements(frequency);
		}
	}
	return cube;
}

MSReader::~MSReader()
{
 	delete(ms);
}

int MSReader::getNumberAntennae()
{
	cout << "N antennae " << msInfo.nAntennae << flush << endl;
	return msInfo.nAntennae;
}


