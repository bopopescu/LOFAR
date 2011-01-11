/***************************************************************************
 *   Copyright (C) 2008-2010 by A.R. Offringa   *
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

#ifndef RFI_UV_PROJECTION_H
#define RFI_UV_PROJECTION_H

#include <AOFlagger/msio/types.h>
#include <AOFlagger/msio/image2d.h>
#include <AOFlagger/msio/timefrequencymetadata.h>

#include <AOFlagger/imaging/uvimager.h>

#include <AOFlagger/util/aologger.h>

class UVProjection
{
	public:
		/**
		* This function will project a uv-track onto a straight line that has an angle of directionRad.
		*/
		static void Project(Image2DCPtr image, const size_t y, numl_t *rowValues, const bool *isConjugated, const bool isImaginary)
		{
			const size_t width = image->Width();

			// Change sign if necessary
			if(isImaginary)
			{
				for(size_t t=0;t<width;++t)
				{
					if(isConjugated[t]) {
						rowValues[t] = image->Value(t, y);
					} else {
						rowValues[t] = -image->Value(t, y);
					}
				}
			} else {
				for(size_t t=0;t<width;++t)
				{
					rowValues[t] = image->Value(t, y);
				}
			}
		}
		
		/**
		* This function will project a uv-track onto a straight line that has an angle of directionRad.
		*/
		static void ProjectPositions(TimeFrequencyMetaDataCPtr metaData, unsigned width, unsigned y, numl_t *rowUPositions, numl_t *rowVPositions, bool *isConjugated, numl_t directionRad)
		{
			const numl_t cosRotate = cosnl(directionRad);
			const numl_t sinRotate = sinnl(directionRad);
			const numl_t frequency = metaData->Band().channels[y].frequencyHz;

			// Calculate the projected positions and change sign if necessary
			for(size_t t=0;t<width;++t)
			{
				const UVW &uvw = metaData->UVW()[t];
				const numl_t vProject = uvw.u * sinRotate + uvw.v * cosRotate;
				numl_t uProject;
				/*if(vProject >= 0.0) {
					uProject = uvw.u * cosRotate - uvw.v * sinRotate;
					rowVPositions[t] = vProject * frequency / UVImager::SpeedOfLight();
					isConjugated[t] = false;
				} else {
					uProject = -uvw.u * cosRotate + uvw.v * sinRotate;
					rowVPositions[t] = -vProject * frequency / UVImager::SpeedOfLight();
					isConjugated[t] = true;
				}*/
				uProject = uvw.u * cosRotate - uvw.v * sinRotate;
				rowVPositions[t] = vProject * frequency / UVImager::SpeedOfLight();
				rowUPositions[t] = uProject * frequency / UVImager::SpeedOfLight();
				isConjugated[t] = false;
			}
		}
		
		static void MaximalUPositions(size_t width, const numl_t *uPositions, numl_t &leftPosition, numl_t &rightPosition)
		{
			leftPosition = 0.0,
			rightPosition = 0.0;
			for(unsigned i=0;i<width;++i)
			{
				if(uPositions[i] < leftPosition)
					leftPosition = uPositions[i];
				if(uPositions[i] > rightPosition)
					rightPosition = uPositions[i];
			}
		}
	
		static void ProjectImage(Image2DCPtr source, Image2DPtr destination, Image2DPtr weights, TimeFrequencyMetaDataCPtr metaData, numl_t directionRad, numl_t etaParameter, bool sourceIsImaginary)
		{
			const size_t
				inputWidth = source->Width();
			numl_t
				*rowValues = new numl_t[inputWidth],
				*rowUPositions = new numl_t[inputWidth],
				*rowVPositions = new numl_t[inputWidth];
			bool
				*isConjugated = new bool[inputWidth];

			for(unsigned y=0;y<source->Height();++y)
			{
				UVProjection::ProjectPositions(metaData, inputWidth, y, rowUPositions, rowVPositions, isConjugated, directionRad);
				
				UVProjection::Project(source, y, rowValues, isConjugated, sourceIsImaginary);

				numl_t leftDist, rightDist;
				MaximalUPositions(inputWidth, rowUPositions, leftDist, rightDist);
				numl_t maxDist = rightDist > -leftDist ? rightDist : -leftDist;

				for(unsigned t=0;t<inputWidth-1;++t)
				{
					unsigned
						t1 = t,
						t2 = t+1;
					double
						u1 = rowUPositions[t],
						u2 = rowUPositions[t+1];
					if(u1 > u2)
					{
						u1 = rowUPositions[t+1];
						u2 = rowUPositions[t];
						t1 = t+1;
						t2 = t;
					}
					if(u2 - u1 >= maxDist)
					{
						numl_t midValue = (rowValues[t1]+rowValues[t2])/2.0;
						Interpolate(destination, weights, leftDist, rightDist, leftDist, u1, midValue, rowValues[t2], y);
						Interpolate(destination, weights, leftDist, rightDist, u2, rightDist, rowValues[t1], midValue, y);
					} else {
						Interpolate(destination, weights, leftDist, rightDist, u1, u2, rowValues[t1], rowValues[t2], y);
					}
				}
				unsigned
					rangeStart = (unsigned) roundnl(etaParameter * (num_t) inputWidth / 2.0),
					rangeEnd = inputWidth - rangeStart;
				for(unsigned x=0;x<inputWidth;++x)
				{
					if(x > rangeStart && x < rangeEnd)
					{
						if(weights->Value(x, y) != 0.0)
							destination->SetValue(x, y, destination->Value(x, y) / weights->Value(x, y));
						else
							AOLogger::Warn << "UV projection did not fill entire range\n";
					} else {
						destination->SetValue(x, y, 0.0);
					}
				}
			}
		}
		
		/**
		 * This function converts a distance from phase centre in radians of the sky towards an
		 * index inside an image created by ProjectImage() that has been FFT'ed (hence it represents
		 * an index into a frequency).
		 */
		static void GetIndicesInProjectedImage(numl_t distance, numl_t minU, numl_t maxU, unsigned sourceWidth, unsigned destWidth, unsigned &lowestIndex, unsigned &highestIndex)
		{
			numl_t sincScale = 1.0/distance;
			numl_t clippingFrequency = 1.0/(sincScale * sourceWidth / (0.5*(maxU-minU)));
			numl_t fourierClippingIndex = (numl_t) destWidth * clippingFrequency;
			if(fourierClippingIndex*2.0 > destWidth)
				fourierClippingIndex = (numl_t) destWidth/2.0;
			if(fourierClippingIndex < 0.0)
				fourierClippingIndex = 0.0;
			lowestIndex = (unsigned) ceil(fourierClippingIndex),
			highestIndex = (unsigned) floor(destWidth - fourierClippingIndex);
		}
		
		/**
		 * This function converts an
		 * index inside an image created by ProjectImage() that has been FFT'ed (hence it represents
		 * an index into a frequency) towards the frequency in U direction of the uv plane.
		 */
		static numl_t GetUFrequency(unsigned fIndex, numl_t minU, numl_t maxU, unsigned sourceWidth, unsigned destWidth)
		{
			int normFIndex = fIndex;
			if(normFIndex > (int) (destWidth/2)) normFIndex = destWidth - normFIndex;
			numl_t frequencyInDest = (numl_t) normFIndex * sourceWidth / (0.5*(maxU-minU) * destWidth);
			return frequencyInDest;
		}
	private:
		
		static void Interpolate(Image2DPtr destination, Image2DPtr weights, numl_t leftDist, numl_t rightDist, numl_t u1, numl_t u2, numl_t v1, numl_t v2, unsigned y)
		{
			int
				width = destination->Width(),
				left = (int) ((u1 - leftDist) * (numl_t) width / (rightDist-leftDist)),
				right = (int) ((u2 - leftDist) * (numl_t) width / (rightDist-leftDist));
			if(left < 0) left = 0;
			if(right >= width) right = width;
			if(right - left < 1 && left+1 < width) right = left+1;
			int count = right-left;
			for(int x=left;x<right;++x)
			{
				numl_t value = v1 + ((v2-v1)*(numl_t) (x-left)/(numl_t) count);
				destination->SetValue(x, y, destination->Value(x, y) + value);
				weights->SetValue(x, y, weights->Value(x, y) + 1.0);
			}
		}

};

#endif // RFI_UV_PROJECTION_H
