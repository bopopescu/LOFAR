/***************************************************************************
 *   Copyright (C) 2011 by A.R. Offringa                                   *
 *   offringa@astro.rug.nl                                                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include <iostream>

#include <AOFlagger/quality/statisticscollector.h>
#include <AOFlagger/quality/defaultstatistics.h>

#include <AOFlagger/msio/measurementset.h>

void reportProgress(unsigned step, unsigned totalSteps)
{
	const unsigned twoPercent = (totalSteps+49)/50;
	if((step%twoPercent)==0)
	{
		if(((step/twoPercent)%5)==0)
			std::cout << (100*step/totalSteps) << std::flush;
		else
			std::cout << '.' << std::flush;
	}
}

void actionCollect(const std::string filename)
{
	MeasurementSet *ms = new MeasurementSet(filename);
	const unsigned polarizationCount = ms->GetPolarizationCount();
	const unsigned bandCount = ms->BandCount();
	BandInfo *bands = new BandInfo[bandCount];
	double **frequencies = new double*[bandCount];
	unsigned totalChannels = 0;
	for(unsigned b=0;b<bandCount;++b)
	{
		bands[b] = ms->GetBandInfo(b);
		frequencies[b] = new double[bands[b].channelCount];
		totalChannels += bands[b].channelCount;
		for(unsigned c=0;c<bands[b].channelCount;++c)
		{
			frequencies[b][c] = bands[b].channels[c].frequencyHz;
		}
	}
	delete ms;
	
	std::cout
		<< "Polarizations: " << polarizationCount << '\n'
		<< "Bands: " << bandCount << '\n'
		<< "Channels/band: " << (totalChannels / bandCount) << '\n';
	
	casa::Table table(filename, casa::Table::Update);
	StatisticsCollector collector(polarizationCount);
	for(unsigned b=0;b<bandCount;++b)
	{
		collector.InitializeBand(b, frequencies[b], bands[b].channelCount);
	}

	casa::ROArrayColumn<casa::Complex> dataColumn(table, "DATA");
	casa::ROArrayColumn<bool> flagColumn(table, "FLAG");
	casa::ROScalarColumn<double> timeColumn(table, "TIME");
	casa::ROScalarColumn<int> antenna1Column(table, "ANTENNA1"); 
	casa::ROScalarColumn<int> antenna2Column(table, "ANTENNA2");
	casa::ROScalarColumn<int> windowColumn(table, "DATA_DESC_ID");
	
	std::cout << "Collecting statistics..." << std::endl;
	
	const unsigned nrow = table.nrow();
	for(unsigned row = 0; row!=nrow; ++row)
	{
		const double time = timeColumn(row);
		const unsigned antenna1Index = antenna1Column(row);
		const unsigned antenna2Index = antenna2Column(row);
		const unsigned bandIndex = windowColumn(row);
		
		const BandInfo &band = bands[bandIndex];
		
		const casa::Array<casa::Complex> dataArray = dataColumn(row);
		const casa::Array<bool> flagArray = flagColumn(row);
		
		std::vector<std::complex<float> > samples[polarizationCount];
		bool *isRFI[polarizationCount];
		for(unsigned p = 0; p < polarizationCount; ++p)
			isRFI[p] = new bool[band.channelCount];
		
		casa::Array<casa::Complex>::const_iterator dataIter = dataArray.begin();
		casa::Array<bool>::const_iterator flagIter = flagArray.begin();
		for(unsigned channel = 0 ; channel<band.channelCount; ++channel)
		{
			for(unsigned p = 0; p < polarizationCount; ++p)
			{
				samples[p].push_back(*dataIter);
				isRFI[p][channel] = *flagIter;
				
				++dataIter;
				++flagIter;
			}
		}
		
		for(unsigned p = 0; p < polarizationCount; ++p)
		{
			collector.Add(antenna1Index, antenna2Index, time, bandIndex, p, samples[p], isRFI[p]);
		}

		for(unsigned p = 0; p < polarizationCount; ++p)
			delete[] isRFI[p];
		
		reportProgress(row, nrow);
	}
	
	for(unsigned b=0;b<bandCount;++b)
		delete[] frequencies[b];
	delete[] frequencies;
	delete[] bands;
	
	std::cout << "100\nWriting quality tables..." << std::endl;
	
	QualityData qualityData(filename);
	collector.Save(qualityData);
	
	std::cout << "Done.\n";
}

void printStatistics(std::complex<float> *complexStat, unsigned count)
{
	if(count != 1)
		std::cout << '[';
	if(count > 0)
		std::cout << complexStat[0].real() << " + " << complexStat[0].imag() << 'i';
	for(unsigned p=1;p<count;++p)
	{
		std::cout << ", " << complexStat[p].real() << " + " << complexStat[p].imag() << 'i';
	}
	if(count != 1)
		std::cout << ']';
}

void printStatistics(unsigned long *stat, unsigned count)
{
	if(count != 1)
		std::cout << '[';
	if(count > 0)
		std::cout << stat[0];
	for(unsigned p=1;p<count;++p)
	{
		std::cout << ", " << stat[p];
	}
	if(count != 1)
		std::cout << ']';
}

void printStatistics(const DefaultStatistics &statistics)
{
	std::cout << "Count=";
	printStatistics(statistics.count, statistics.polarizationCount);
	std::cout << "\nMean=";
	printStatistics(statistics.mean, statistics.polarizationCount);
	std::cout << "\nSumP2=";
	printStatistics(statistics.sumP2, statistics.polarizationCount);
	std::cout << "\nDCount=";
	printStatistics(statistics.dCount, statistics.polarizationCount);
	std::cout << "\nDMean=";
	printStatistics(statistics.dMean, statistics.polarizationCount);
	std::cout << "\nDSumP2=";
	printStatistics(statistics.dSumP2, statistics.polarizationCount);
	std::cout << "\nRFICount=";
	printStatistics(statistics.rfiCount, statistics.polarizationCount);
	std::cout << '\n';
}

void actionSummarize(const std::string &filename)
{
	MeasurementSet *ms = new MeasurementSet(filename);
	const unsigned polarizationCount = ms->GetPolarizationCount();
	delete ms;
	
	QualityData qualityData(filename);
	StatisticsCollector collector(polarizationCount);
	collector.Load(qualityData);
	
	DefaultStatistics statistics(polarizationCount);
	
	collector.GetGlobalTimeStatistics(statistics);
	std::cout << "Time statistics: \n";
	printStatistics(statistics);
	
	collector.GetGlobalFrequencyStatistics(statistics);
	std::cout << "\nFrequency statistics: \n";
	printStatistics(statistics);

	collector.GetGlobalCrossBaselineStatistics(statistics);
	std::cout << "\nCross-correlated baseline statistics: \n";
	printStatistics(statistics);

	collector.GetGlobalAutoBaselineStatistics(statistics);
	std::cout << "\nAuto-correlated baseline: \n";
	printStatistics(statistics);
}

void printSyntax(std::ostream &stream, char *argv[])
{
	stream << "Syntax: " << argv[0] <<
		" <action> [options]\n\n"
		"Possible actions:\n"
		"\thelp      - Get more info about an action (usage: '" << argv[0] << " help <action>')\n"
		"\tcollect   - Processes the entire measurement set, collects the statistics\n"
		"\t            and writes them in the quality tables.\n"
		"\tsummarize - Give a summary of the statistics currently in the quality tables.\n";
}

int main(int argc, char *argv[])
{
	if(argc < 2)
	{
		printSyntax(std::cerr, argv);
		return -1;
	} else {
		
		const std::string action = argv[1];
		
		if(action == "help")
		{
			if(argc != 3)
			{
				printSyntax(std::cout, argv);
			} else {
				std::string helpAction = argv[2];
				if(helpAction == "help")
				{
					printSyntax(std::cout, argv);
				}
				else if(helpAction == "collect")
				{
					std::cout << "Syntax: " << argv[0] << " collect <ms>\n\n"
						"The collect action will go over a whole measurement set and \n"
						"collect the default statistics. It will write the results in the \n"
						"quality subtables of the main measurement set.\n\n"
						"Currently, the default statistics are:\n"
						"\tRFIRatio, Count, Mean, SumP2, DCount, DMean, DSumP2.\n"
						"The subtables that will be updated are:\n"
						"\tQUALITY_KIND_NAME, QUALITY_TIME_STATISTIC,\n"
						"\tQUALITY_FREQUENCY_STATISTIC and QUALITY_BASELINE_STATISTIC.\n";
				}
				else if(helpAction == "summarize")
				{
					std::cout << "Syntax: " << argv[0] << " summarize <ms>\n\n"
						"Will give a summary of the statistics in the measurement set.\n";
				}
				else
				{
					std::cerr << "Unknown action specified in help.\n";
					return -1;
				}
			}
			return 0;
		}
		else if(action == "collect")
		{
			if(argc != 3)
			{
				std::cerr << "collect actions needs one parameter (the measurement set)\n";
				return -1;
			}
			else {
				actionCollect(argv[2]);
				return 0;
			}
		}
		else if(action == "summarize")
		{
			if(argc != 3)
			{
				std::cerr << "summarize actions needs one parameter (the measurement set)\n";
				return -1;
			}
			else {
				actionSummarize(argv[2]);
				return 0;
			}
		}
		std::cerr << "Unknown action '" << action << "'.\n";
		return -1;
	}
}
