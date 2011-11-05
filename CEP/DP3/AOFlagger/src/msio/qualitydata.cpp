/***************************************************************************
 *   Copyright (C) 2008 by A.R. Offringa   *
 *   offringa@astro.rug.nl   *
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

#include <AOFlagger/msio/qualitydata.h>

#include <stdexcept>
#include <set>

#include <ms/MeasurementSets/MSColumns.h>

#include <tables/Tables/ArrColDesc.h>
#include <tables/Tables/ScaColDesc.h>
#include <tables/Tables/SetupNewTab.h>

#include <measures/TableMeasures/TableMeasDesc.h>

#include <measures/Measures/MEpoch.h>

#include <AOFlagger/msio/statisticalvalue.h>

const std::string QualityData::_kindToNameTable[] =
{
	"RFIRatio",
	"FlaggedRatio",
	"Count",
	"Mean",
	"RFIMean",
	"RFICount",
	"SumP2",
	"SumP3",
	"SumP4",
	"Variance",
	"VarianceOfVariance",
	"Skewness",
	"Kurtosis",
	"SignalToNoise",
	"DMean",
	"DSumP2",
	"DSumP4",
	"DVariance",
	"DVarianceOfVariance",
	"DCount"
};

const std::string QualityData::_tableToNameTable[] =
{
	"QUALITY_KIND_NAME",
	"QUALITY_TIME_STATISTIC",
	"QUALITY_FREQUENCY_STATISTIC",
	"QUALITY_BASELINE_STATISTIC",
	"QUALITY_BASELINE_TIME_STATISTIC"
};

const enum QualityData::QualityTable QualityData::_dimensionToTableTable[] =
{
	TimeStatisticTable,
	FrequencyStatisticTable,
	BaselineStatisticTable,
	BaselineTimeStatisticTable
};

const std::string QualityData::ColumnNameAntenna1  = "ANTENNA1";
const std::string QualityData::ColumnNameAntenna2  = "ANTENNA2";
const std::string QualityData::ColumnNameFrequency = "FREQUENCY";
const std::string QualityData::ColumnNameKind      = "KIND";
const std::string QualityData::ColumnNameName      = "NAME";
const std::string QualityData::ColumnNameTime      = "TIME";
const std::string QualityData::ColumnNameValue     = "VALUE";

unsigned QualityData::QueryKindIndex(enum StatisticKind kind) const
{
	unsigned kindIndex;
	if(!QueryKindIndex(kind, kindIndex))
		throw std::runtime_error("getKindIndex(): Requested statistic kind not available.");
	return kindIndex;
}

bool QualityData::QueryKindIndex(enum StatisticKind kind, unsigned &destKindIndex) const
{
	casa::Table table(TableToName(KindNameTable));
	casa::ROScalarColumn<int> kindColumn(table, ColumnNameKind);
	casa::ROScalarColumn<casa::String> nameColumn(table, ColumnNameName);
	const casa::String nameToFind(KindToName(kind));
	
	const unsigned nrRow = table.nrow();
	
	for(unsigned i=0;i<nrRow;++i)
	{
		if(nameColumn(i) == nameToFind)
		{
			destKindIndex = kindColumn(i);
			return true;
		}
	}
	return false;
}

bool QualityData::hasOneEntry(enum QualityTable table, unsigned kindIndex) const
{
	casa::Table casaTable(TableToName(table));
	casa::ROScalarColumn<int> kindColumn(casaTable, ColumnNameKind);
	
	const unsigned nrRow = casaTable.nrow();
	
	for(unsigned i=0;i<nrRow;++i)
	{
		if(kindColumn(i) == (int) kindIndex)
			return true;
	}
	return false;
}

void QualityData::createKindNameTable()
{
	casa::TableDesc tableDesc("QUALITY_KIND_NAME_TYPE", QUALITY_DATA_VERSION_STR, casa::TableDesc::Scratch);
	tableDesc.comment() = "Couples the KIND column in the other quality tables to the name of a statistic (e.g. Mean)";
	tableDesc.addColumn(casa::ScalarColumnDesc<int>(ColumnNameKind, "Index of the statistic kind"));
	tableDesc.addColumn(casa::ScalarColumnDesc<casa::String>(ColumnNameName, "Name of the statistic"));

	casa::SetupNewTable newTableSetup(TableToName(KindNameTable), tableDesc, casa::Table::New);
	(class casa::Table)(newTableSetup);
}

void QualityData::addTimeColumn(casa::TableDesc &tableDesc)
{
	casa::ScalarColumnDesc<double> timeDesc(ColumnNameTime, "Central time of statistic");
	tableDesc.addColumn(timeDesc);
	
	casa::TableMeasRefDesc measRef(casa::MEpoch::UTC);
	casa::TableMeasValueDesc measVal(tableDesc, ColumnNameTime);
	casa::TableMeasDesc<casa::MEpoch> mepochCol(measVal, measRef);
	mepochCol.write(tableDesc);
}

void QualityData::addFrequencyColumn(casa::TableDesc &tableDesc)
{
	casa::ScalarColumnDesc<double> freqDesc(ColumnNameFrequency, "Central frequency of statistic bin");
	tableDesc.addColumn(freqDesc);
	
	casa::Unit hertzUnit("Hz");
	
	casa::TableQuantumDesc quantDesc(tableDesc, ColumnNameFrequency, hertzUnit);
	quantDesc.write(tableDesc);
}

void QualityData::addValueColumn(casa::TableDesc &tableDesc)
{
	casa::IPosition shape(1);
	shape[0] = 4;
	casa::ArrayColumnDesc<casa::Complex> valDesc(ColumnNameValue, "Value of statistic", shape, casa::ColumnDesc::Direct);
	tableDesc.addColumn(valDesc);
}

void QualityData::createTimeStatisticTable()
{
	casa::TableDesc tableDesc("QUALITY_TIME_STATISTIC_TYPE", QUALITY_DATA_VERSION_STR, casa::TableDesc::Scratch);
	tableDesc.comment() = "Statistics over time";
	
	addTimeColumn(tableDesc);
	addFrequencyColumn(tableDesc);
	tableDesc.addColumn(casa::ScalarColumnDesc<int>(ColumnNameKind, "Index of the statistic kind"));
	addValueColumn(tableDesc);

	casa::SetupNewTable newTableSetup(TableToName(TimeStatisticTable), tableDesc, casa::Table::New);
	(class casa::Table)(newTableSetup);
}

void QualityData::createFrequencyStatisticTable()
{
	casa::TableDesc tableDesc("QUALITY_FREQUENCY_STATISTIC_TYPE", QUALITY_DATA_VERSION_STR, casa::TableDesc::Scratch);
	tableDesc.comment() = "Statistics over frequency";
	
	addFrequencyColumn(tableDesc);
	tableDesc.addColumn(casa::ScalarColumnDesc<int>(ColumnNameKind, "Index of the statistic kind"));
	addValueColumn(tableDesc);

	casa::SetupNewTable newTableSetup(TableToName(FrequencyStatisticTable), tableDesc, casa::Table::New);
	(class casa::Table)(newTableSetup);
}

void QualityData::createBaselineStatisticTable()
{
	casa::TableDesc tableDesc("QUALITY_BASELINE_STATISTIC_TYPE", QUALITY_DATA_VERSION_STR, casa::TableDesc::Scratch);
	tableDesc.comment() = "Statistics per baseline";
	
	tableDesc.addColumn(casa::ScalarColumnDesc<int>(ColumnNameAntenna1, "Index of first antenna"));
	tableDesc.addColumn(casa::ScalarColumnDesc<int>(ColumnNameAntenna2, "Index of second antenna"));
	addFrequencyColumn(tableDesc);
	tableDesc.addColumn(casa::ScalarColumnDesc<int>(ColumnNameKind, "Index of the statistic kind"));
	addValueColumn(tableDesc);

	casa::SetupNewTable newTableSetup(TableToName(BaselineStatisticTable), tableDesc, casa::Table::New);
	(class casa::Table)(newTableSetup);
}

void QualityData::createBaselineTimeStatisticTable()
{
	casa::TableDesc tableDesc("QUALITY_BASELINE_TIME_STATISTIC_TYPE", QUALITY_DATA_VERSION_STR, casa::TableDesc::Scratch);
	tableDesc.comment() = "Statistics per baseline";
	
	addTimeColumn(tableDesc);
	tableDesc.addColumn(casa::ScalarColumnDesc<int>(ColumnNameAntenna1, "Index of first antenna"));
	tableDesc.addColumn(casa::ScalarColumnDesc<int>(ColumnNameAntenna2, "Index of second antenna"));
	addFrequencyColumn(tableDesc);
	tableDesc.addColumn(casa::ScalarColumnDesc<int>(ColumnNameKind, "Index of the statistic kind"));
	addValueColumn(tableDesc);

	casa::SetupNewTable newTableSetup(TableToName(BaselineTimeStatisticTable), tableDesc, casa::Table::New);
	(class casa::Table)(newTableSetup);
}

unsigned QualityData::StoreKindName(const std::string &name)
{
	// This should be done atomically, but two quality writers in the same table would be
	// a weird thing to do anyway, plus I don't know how the casa tables can be made atomic
	// (and still have good performance).

	casa::Table table(TableToName(KindNameTable), casa::Table::Update);
	
	unsigned kindIndex = findFreeKindIndex(table);
	
	unsigned newRow = table.nrow();
	table.addRow();
	casa::ScalarColumn<int> kindColumn(table, ColumnNameKind);
	casa::ScalarColumn<casa::String> nameColumn(table, ColumnNameName);
	kindColumn.put(newRow, kindIndex);
	nameColumn.put(newRow, name);
	return kindIndex;
}

unsigned QualityData::findFreeKindIndex(casa::Table &kindTable)
{
	int maxIndex = 0;
	
	casa::ROScalarColumn<int> kindColumn(kindTable, ColumnNameKind);
	
	const unsigned nrRow = kindTable.nrow();
	
	for(unsigned i=0;i<nrRow;++i)
	{
		if(kindColumn(i) > maxIndex)
			maxIndex = kindColumn(i);
	}
	return maxIndex + 1;
}

void QualityData::openTable(QualityTable table, bool needWrite, casa::Table **tablePtr)
{
	if(*tablePtr == 0)
	{
		if(needWrite)
			*tablePtr = new casa::Table(TableToName(table), casa::Table::Update);
		else
			*tablePtr = new casa::Table(TableToName(table));
	} else {
		if(needWrite && !(*tablePtr)->isWritable())
			(*tablePtr)->reopenRW();
	}
}

void QualityData::StoreTimeValue(double time, double frequency, const StatisticalValue &value)
{
	openTimeTable(true);
	
	unsigned newRow = _timeTable->nrow();
	_timeTable->addRow();
	
	// TODO maybe the columns need to be cached to avoid having to look them up for each store...
	casa::ScalarColumn<double> timeColumn(*_timeTable, ColumnNameTime);
	casa::ScalarColumn<double> frequencyColumn(*_timeTable, ColumnNameFrequency);
	casa::ScalarColumn<int> kindColumn(*_timeTable, ColumnNameKind);
	casa::ArrayColumn<casa::Complex> valueColumn(*_timeTable, ColumnNameValue);
	
	timeColumn.put(newRow, time);
	frequencyColumn.put(newRow, frequency);
	kindColumn.put(newRow, value.KindIndex());
	casa::Vector<casa::Complex> data(value.PolarizationCount());
	for(unsigned i=0;i<value.PolarizationCount();++i)
		data[i] = value.Value(i);
	valueColumn.put(newRow, data);
}

void QualityData::StoreFrequencyValue(double frequency, const StatisticalValue &value)
{
	openFrequencyTable(true);
	
	unsigned newRow = _frequencyTable->nrow();
	_frequencyTable->addRow();
	
	casa::ScalarColumn<double> frequencyColumn(*_frequencyTable, ColumnNameFrequency);
	casa::ScalarColumn<int> kindColumn(*_frequencyTable, ColumnNameKind);
	casa::ArrayColumn<casa::Complex> valueColumn(*_frequencyTable, ColumnNameValue);
	
	frequencyColumn.put(newRow, frequency);
	kindColumn.put(newRow, value.KindIndex());
	casa::Vector<casa::Complex> data(value.PolarizationCount());
	for(unsigned i=0;i<value.PolarizationCount();++i)
		data[i] = value.Value(i);
	valueColumn.put(newRow, data);
}

void QualityData::StoreBaselineValue(unsigned antenna1, unsigned antenna2, double frequency, const StatisticalValue &value)
{
	openBaselineTable(true);
	
	unsigned newRow = _baselineTable->nrow();
	_baselineTable->addRow();
	
	casa::ScalarColumn<int> antenna1Column(*_baselineTable, ColumnNameAntenna1);
	casa::ScalarColumn<int> antenna2Column(*_baselineTable, ColumnNameAntenna2);
	casa::ScalarColumn<double> frequencyColumn(*_baselineTable, ColumnNameFrequency);
	casa::ScalarColumn<int> kindColumn(*_baselineTable, ColumnNameKind);
	casa::ArrayColumn<casa::Complex> valueColumn(*_baselineTable, ColumnNameValue);
	
	antenna1Column.put(newRow, antenna1);
	antenna2Column.put(newRow, antenna2);
	frequencyColumn.put(newRow, frequency);
	kindColumn.put(newRow, value.KindIndex());
	casa::Vector<casa::Complex> data(value.PolarizationCount());
	for(unsigned i=0;i<value.PolarizationCount();++i)
		data[i] = value.Value(i);
	valueColumn.put(newRow, data);
}

void QualityData::StoreBaselineTimeValue(unsigned antenna1, unsigned antenna2, double time, double frequency, const StatisticalValue &value)
{
	openBaselineTimeTable(true);
	
	unsigned newRow = _baselineTimeTable->nrow();
	_baselineTimeTable->addRow();
	
	casa::ScalarColumn<double> timeColumn(*_baselineTimeTable, ColumnNameTime);
	casa::ScalarColumn<int> antenna1Column(*_baselineTimeTable, ColumnNameAntenna1);
	casa::ScalarColumn<int> antenna2Column(*_baselineTimeTable, ColumnNameAntenna2);
	casa::ScalarColumn<double> frequencyColumn(*_baselineTimeTable, ColumnNameFrequency);
	casa::ScalarColumn<int> kindColumn(*_baselineTimeTable, ColumnNameKind);
	casa::ArrayColumn<casa::Complex> valueColumn(*_baselineTimeTable, ColumnNameValue);
	
	timeColumn.put(newRow, time);
	antenna1Column.put(newRow, antenna1);
	antenna2Column.put(newRow, antenna2);
	frequencyColumn.put(newRow, frequency);
	kindColumn.put(newRow, value.KindIndex());
	casa::Vector<casa::Complex> data(value.PolarizationCount());
	for(unsigned i=0;i<value.PolarizationCount();++i)
		data[i] = value.Value(i);
	valueColumn.put(newRow, data);
}

void QualityData::removeStatisticFromStatTable(enum QualityTable qualityTable, enum StatisticKind kind)
{
	unsigned kindIndex;
	if(QueryKindIndex(kind, kindIndex))
	{	
		casa::Table table(TableToName(qualityTable), casa::Table::Update);
		casa::ScalarColumn<int> kindColumn(table, ColumnNameKind);

		unsigned nrRow = table.nrow();

		for(unsigned i=0;i<nrRow;++i)
		{
			while(i<nrRow && kindColumn(i) == (int) kindIndex)
			{
				table.removeRow(i);
				--nrRow;
			}
		}
	}
}

void QualityData::removeKindNameEntry(enum StatisticKind kind)
{
	casa::Table table(TableToName(KindNameTable), casa::Table::Update);
	casa::ScalarColumn<casa::String> nameColumn(table, ColumnNameName);
	
	const unsigned nrRow = table.nrow();
	const casa::String kindName(KindToName(kind));
	
	for(unsigned i=0;i<nrRow;++i)
	{
		if(nameColumn(i) == kindName)
		{
			table.removeRow(i);
			break;
		}
	}
}

void QualityData::removeEntries(enum QualityTable table)
{
	casa::Table casaTable(TableToName(KindNameTable), casa::Table::Update);
	const unsigned nrRow = casaTable.nrow();
	for(int i=nrRow-1;i>=0;--i)
	{
		casaTable.removeRow(i);
	}
}

unsigned QualityData::QueryStatisticEntryCount(enum StatisticDimension dimension, unsigned kindIndex) const
{
	casa::Table casaTable(TableToName(DimensionToTable(dimension)));
	casa::ROScalarColumn<int> kindColumn(casaTable, ColumnNameKind);
	
	const unsigned nrRow = casaTable.nrow();
	size_t count = 0;
	
	for(unsigned i=0;i<nrRow;++i)
	{
		if(kindColumn(i) == (int) kindIndex)
			++count;
	}
	return count;
}

void QualityData::QueryTimeStatistic(unsigned kindIndex, std::vector<std::pair<TimePosition, StatisticalValue> > &entries) const
{
	casa::Table table(TableToName(TimeStatisticTable));
	const unsigned nrRow = table.nrow();
	
	casa::ROScalarColumn<double> timeColumn(table, ColumnNameTime);
	casa::ROScalarColumn<double> frequencyColumn(table, ColumnNameFrequency);
	casa::ROScalarColumn<int> kindColumn(table, ColumnNameKind);
	casa::ROArrayColumn<casa::Complex> valueColumn(table, ColumnNameValue);
	
	int polarizationCount = valueColumn.columnDesc().shape()[0];
	
	for(unsigned i=0;i<nrRow;++i)
	{
		if(kindColumn(i) == (int) kindIndex)
		{
			StatisticalValue value(polarizationCount);
			value.SetKindIndex(kindIndex);
			casa::Array<casa::Complex> valueArray = valueColumn(i);
			casa::Array<casa::Complex>::iterator iter = valueArray.begin();
			for(int p=0;p<polarizationCount;++p)
			{
				value.SetValue(p, *iter);
				++iter;
			}
			TimePosition position;
			position.time = timeColumn(i);
			position.frequency = frequencyColumn(i);
			entries.push_back(std::pair<TimePosition, StatisticalValue>(position, value));
		}
	}
}

void QualityData::QueryFrequencyStatistic(unsigned kindIndex, std::vector<std::pair<FrequencyPosition, StatisticalValue> > &entries) const
{
	casa::Table table(TableToName(FrequencyStatisticTable));
	const unsigned nrRow = table.nrow();
	
	casa::ROScalarColumn<double> frequencyColumn(table, ColumnNameFrequency);
	casa::ROScalarColumn<int> kindColumn(table, ColumnNameKind);
	casa::ROArrayColumn<casa::Complex> valueColumn(table, ColumnNameValue);
	
	int polarizationCount = valueColumn.columnDesc().shape()[0];
	
	for(unsigned i=0;i<nrRow;++i)
	{
		if(kindColumn(i) == (int) kindIndex)
		{
			StatisticalValue value(polarizationCount);
			value.SetKindIndex(kindIndex);
			casa::Array<casa::Complex> valueArray = valueColumn(i);
			casa::Array<casa::Complex>::iterator iter = valueArray.begin();
			for(int p=0;p<polarizationCount;++p)
			{
				value.SetValue(p, *iter);
				++iter;
			}
			FrequencyPosition position;
			position.frequency = frequencyColumn(i);
			entries.push_back(std::pair<FrequencyPosition, StatisticalValue>(position, value));
		}
	}
}

void QualityData::QueryBaselineStatistic(unsigned kindIndex, std::vector<std::pair<BaselinePosition, StatisticalValue> > &entries) const
{
	casa::Table table(TableToName(BaselineStatisticTable));
	const unsigned nrRow = table.nrow();
	
	casa::ROScalarColumn<int> antenna1Column(table, ColumnNameAntenna1);
	casa::ROScalarColumn<int> antenna2Column(table, ColumnNameAntenna2);
	casa::ROScalarColumn<double> frequencyColumn(table, ColumnNameFrequency);
	casa::ROScalarColumn<int> kindColumn(table, ColumnNameKind);
	casa::ROArrayColumn<casa::Complex> valueColumn(table, ColumnNameValue);
	
	int polarizationCount = valueColumn.columnDesc().shape()[0];
	
	for(unsigned i=0;i<nrRow;++i)
	{
		if(kindColumn(i) == (int) kindIndex)
		{
			StatisticalValue value(polarizationCount);
			value.SetKindIndex(kindIndex);
			casa::Array<casa::Complex> valueArray = valueColumn(i);
			casa::Array<casa::Complex>::iterator iter = valueArray.begin();
			for(int p=0;p<polarizationCount;++p)
			{
				value.SetValue(p, *iter);
				++iter;
			}
			BaselinePosition position;
			position.antenna1 = antenna1Column(i);
			position.antenna2 = antenna2Column(i);
			position.frequency = frequencyColumn(i);
			entries.push_back(std::pair<BaselinePosition, StatisticalValue>(position, value));
		}
	}
}

