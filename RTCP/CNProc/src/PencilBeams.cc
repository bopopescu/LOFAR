//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

#include <PencilBeams.h>

#include <Interface/MultiDimArray.h>
#include <Interface/PrintVector.h>
#include <Common/Timer.h>
#include <iostream>
#include <cmath>
#include <cassert>
#include <vector>

#ifndef PENCILBEAMS_C_IMPLEMENTATION
  #include <BeamFormerAsm.h>
#endif

#ifndef M_SQRT3
  #define M_SQRT3     1.73205080756887719000
#endif

namespace LOFAR {
namespace RTCP {

static NSTimer pencilBeamFormTimer("PencilBeamFormer::formBeams()", true);

PencilCoordinates& PencilCoordinates::operator+=( const PencilCoordinates &rhs )
{
  itsCoordinates.reserve( itsCoordinates.size() + rhs.size() );
  for( unsigned i = 0; i < rhs.size(); i++ ) {
     itsCoordinates.push_back( rhs.itsCoordinates[i] );
  }

  return *this;
}

PencilCoordinates::PencilCoordinates( const Matrix<double> &coordinates )
{
  itsCoordinates.reserve( coordinates.size() );
  for( unsigned i = 0; i < coordinates.size(); i++ ) {
    itsCoordinates.push_back( PencilCoord3D( coordinates[i][0], coordinates[i][1] ) );
  }
}

PencilRings::PencilRings(const unsigned nrRings, const double ringWidth):
  itsNrRings(nrRings),
  itsRingWidth(ringWidth)
{
  computeBeamCoordinates();
}

unsigned PencilRings::nrPencils() const
{
  // the centered hexagonal number
  return 3 * itsNrRings * (itsNrRings + 1) + 1;
}

double PencilRings::pencilEdge() const
{
  return itsRingWidth / M_SQRT3;
}

double PencilRings::pencilWidth() const
{
  //  _   //
  // / \  //
  // \_/  //
  //|...| //
  return 2.0 * pencilEdge();
}

double PencilRings::pencilHeight() const
{
  //  _  _ //
  // / \ : //
  // \_/ _ //
  //       //
  return itsRingWidth;
}

double PencilRings::pencilWidthDelta() const
{
  //  _    //
  // / \_  //
  // \_/ \ //
  //   \_/ //
  //  |.|  //
  return 1.5 * pencilEdge();
}

double PencilRings::pencilHeightDelta() const
{
  //  _      //
  // / \_  - //
  // \_/ \ - //
  //   \_/   //
  return 0.5 * itsRingWidth;
}

void PencilRings::computeBeamCoordinates()
{
  std::vector<PencilCoord3D> &coordinates = getCoordinates();
  double dl[6], dm[6];

  // stride for each side, starting from the top, clock-wise

  //  _    //
  // / \_  //
  // \_/ \ //
  //   \_/ //
  dl[0] = pencilWidthDelta();
  dm[0] = -pencilHeightDelta();

  //  _  //
  // / \ //
  // \_/ //
  // / \ //
  // \_/ //
  dl[1] = 0;
  dm[1] = -pencilHeight();

  //    _  //
  //  _/ \ //
  // / \_/ //
  // \_/   //
  dl[2] = -pencilWidthDelta();
  dm[2] = -pencilHeightDelta();

  //  _    //
  // / \_  //
  // \_/ \ //
  //   \_/ //
  dl[3] = -pencilWidthDelta();
  dm[3] = pencilHeightDelta();

  //  _  //
  // / \ //
  // \_/ //
  // / \ //
  // \_/ //
  dl[4] = 0;
  dm[4] = pencilHeight();

  //    _  //
  //  _/ \ //
  // / \_/ //
  // \_/   //
  dl[5] = pencilWidthDelta();
  dm[5] = pencilHeightDelta();

  // ring 0: the center pencil beam
  coordinates.reserve(nrPencils());
  coordinates.push_back( PencilCoord3D( 0, 0 ) );

  // ring 1-n: create the pencil beams from the inner ring outwards
  for( unsigned ring = 1; ring <= itsNrRings; ring++ ) {
    // start from the top
    double l = 0;
    double m = pencilHeight() * ring;

    for( unsigned side = 0; side < 6; side++ ) {
      for( unsigned pencil = 0; pencil < ring; pencil++ ) {
        coordinates.push_back( PencilCoord3D( l, m ) );
        l += dl[side]; m += dm[side];
      }
    }
  }
}

PencilBeams::PencilBeams(PencilCoordinates &coordinates, const unsigned nrStations, const unsigned nrChannels, const unsigned nrSamplesPerIntegration, const double centerFrequency, const double channelBandwidth, const std::vector<double> &refPhaseCentre, const Matrix<double> &phaseCentres, const bool correctBandPass )
:
  itsCoordinates(coordinates.getCoordinates()),
  itsNrStations(nrStations),
  itsNrChannels(nrChannels),
  itsNrSamplesPerIntegration(nrSamplesPerIntegration),
  itsCenterFrequency(centerFrequency),
  itsChannelBandwidth(channelBandwidth),
  itsBaseFrequency(centerFrequency - (nrChannels/2) * channelBandwidth),
  itsRefPhaseCentre(refPhaseCentre),
  itsBandPass(BandPass(correctBandPass,nrChannels))
{
  // copy all phase centres and their derived constants
  itsPhaseCentres.reserve( nrStations );
  itsBaselines.reserve( nrStations );
  itsBaselinesSeconds.reserve( nrStations );
  itsDelays.resize( nrStations, itsCoordinates.size() );
  itsDelayOffsets.resize( nrStations, itsCoordinates.size() );
  for( unsigned stat = 0; stat < nrStations; stat++ ) {
     const double x = phaseCentres[stat][0];
     const double y = phaseCentres[stat][1];
     const double z = phaseCentres[stat][2];

     PencilCoord3D phaseCentre( x, y, z );
     //std::clog << "phase center #" << stat << ": " << phaseCentre;
     PencilCoord3D baseLine = phaseCentre - refPhaseCentre;
     PencilCoord3D baseLineSeconds = baseLine * (1.0/speedOfLight);

     itsPhaseCentres.push_back( phaseCentre );
     itsBaselines.push_back( baseLine );
     itsBaselinesSeconds.push_back( baseLineSeconds );

     for( unsigned beam = 0; beam < itsCoordinates.size(); beam++ ) {
       itsDelayOffsets[stat][beam] = baseLine * itsCoordinates[beam] * (1.0/speedOfLight);
       //std::clog << "delay offset for beam " << beam << " station " << stat << itsDelayOffsets[stat][beam] << " = " << baseLine << " * " << itsCoordinates[beam] << " * " << (1.0/speedOfLight) << std::endl;
     }
  }
}

void PencilBeams::calculateDelays( unsigned stat, const PencilCoord3D &beamDir )
{
  const double compensatedDelay = itsDelayOffsets[stat][0] - itsBaselinesSeconds[stat] * beamDir;

  //std::clog << "station " << stat << " beam 0 has an absolute delay of  " << compensatedDelay << std::endl;

  // centre beam does not need compensation
  itsDelays[stat][0] = 0.0;

  for( unsigned i = 1; i < itsCoordinates.size(); i++ ) {
     // delay[i] = baseline * (coordinate - beamdir) / c
     //          = (baseline * coordinate / c) - (baseline * beamdir) / c
     //          = delayoffset - baselinesec * beamdir
     //
     // further reduced by the delay we already compensate for when doing regular beam forming (the centre of the beam). that compensation is done at the IONode (sample shift) and the PPF (phase shift)
     itsDelays[stat][i] = itsDelayOffsets[stat][i] - itsBaselinesSeconds[stat] * beamDir - compensatedDelay;

     //std::clog << "station " << stat << " beam " << i << "has an additional delay of " << itsDelays[stat][i] << std::endl;
     //std::clog << itsDelayOffsets[stat][i] << " " << itsBaselinesSeconds[stat] << " " << beamDir << " " << compensatedDelay << std::endl;
     //std::clog << "example shift: " << phaseShift( itsBaseFrequency + itsNrChannels/2*itsChannelBandwidth, itsDelays[stat][i] ) << std::endl;
  }
}

#ifdef PENCILBEAMS_C_IMPLEMENTATION
void PencilBeams::computeComplexVoltages( const FilteredData *in, PencilBeamData *out, const std::vector<unsigned> &stations )
{
  const unsigned upperBound = static_cast<unsigned>(itsNrSamplesPerIntegration * PencilBeams::MAX_FLAGGED_PERCENTAGE);
  std::vector<unsigned> validStations;

  validStations.reserve( stations.size() );

  // determine which stations have too much flagged data
  for(unsigned stat = 0; stat < stations.size(); stat++) {
    if( in->flags[stations[stat]].count() > upperBound ) {
      // too much flagged data -- drop station
    } else {
      // keep station
      validStations.push_back( stations[stat] );
    }
  }

  // conservative flagging: flag output if any input was flagged 
  for( unsigned beam = 0; beam < itsCoordinates.size(); beam++ ) {
    out->flags[beam].reset();

    for (unsigned stat = 0; stat < validStations.size(); stat++ ) {
      out->flags[beam] |= in->flags[validStations[stat]]; 
    }
  }

  const double averagingFactor = 1.0 / validStations.size();

  for( unsigned beam = 0; beam < itsCoordinates.size(); beam++ ) {
    for (unsigned ch = 0; ch < itsNrChannels; ch ++) {
      const double frequency = itsBaseFrequency + ch * itsChannelBandwidth;
      const double bandPassFactor = itsBandPass.correctionFactors()[ch];
      const float factor = averagingFactor * bandPassFactor;

      for (unsigned time = 0; time < itsNrSamplesPerIntegration; time ++) {
        if( !out->flags[beam].test(time) ) {
          for (unsigned pol = 0; pol < NR_POLARIZATIONS; pol ++) {
            fcomplex &dest  = out->samples[ch][beam][time][pol];

            // combine the stations for this beam
            fcomplex sample = makefcomplex( 0, 0 );

            for( unsigned stat = 0; stat < validStations.size(); stat++ ) {
              // note: for beam #0 (central beam), the phaseShift is 1
              const fcomplex shift = phaseShift( frequency, itsDelays[validStations[stat]][beam] );
              sample += in->samples[ch][validStations[stat]][time][pol] * shift;
            }
            dest = sample * factor;
          }
        } else {
          // data is flagged
          for (unsigned pol = 0; pol < NR_POLARIZATIONS; pol ++) {
            out->samples[ch][beam][time][pol] = makefcomplex( 0, 0 );
  	  }
	}
      }
    }
  }
}

#else // ASM implementation

void PencilBeams::computeComplexVoltages( const FilteredData *in, PencilBeamData *out, const std::vector<unsigned> &stations )
{
  // ASSERTs enforce supported modes

  // Maximum number of flagged samples. If a station has more, we discard it entirely
  const unsigned upperBound = static_cast<unsigned>(itsNrSamplesPerIntegration * PencilBeams::MAX_FLAGGED_PERCENTAGE);

  // Whether each station is valid or discarded
  std::vector<bool> validStation;

  // Number of valid stations
  unsigned nrValidStations = 0;

  validStations.resize( stations.size() );

  // determine which stations have too much flagged data
  for(unsigned stat = 0; stat < stations.size(); stat++) {
    ASSERT( stations[stat] == stat );

    if( in->flags[stations[stat]].count() > upperBound ) {
      // too much flagged data -- drop station
      validStation[stat] = false;
    } else {
      // keep station
      validStation[stat] = true;
      nrValidStations++;
    }
  }

  // conservative flagging: flag output if any input was flagged 
  for( unsigned beam = 0; beam < itsCoordinates.size(); beam++ ) {
    out->flags[beam].reset();

    for (unsigned stat = 0; stat < stations.size(); stat++ ) {
      if( validStation[stat] ) {
        out->flags[beam] |= in->flags[stations[stat]]; 
      }
    }
  }

  const double averagingFactor = 1.0 / nrValidStations;

  // we only support this mode for now
  #define NRSTATIONS 3
  #define NRBEAMS 6

  #define FUNC_(s,b) _beamform_ ## s ## stations_ ## b ## beams
  #define FUNC(s,b) FUNC_(s,b)

  ASSERT( stations.size() % NRSTATIONS == 0 );
  ASSERT( itsCoordinates.size() % NRBEAMS == 0 );

  for (unsigned ch = 0; ch < itsNrChannels; ch ++) {
    const double frequency = itsBaseFrequency + ch * itsChannelBandwidth;
    const double bandPassFactor = itsBandPass.correctionFactors()[ch];
    const float factor = averagingFactor * bandPassFactor;

    for( unsigned beam = 0; beam < itsCoordinates.size(); beam += NRBEAMS ) {
      for( unsigned stat = 0; stat < stations.size(); stat += NRSTATIONS ) {
        fcomplex weights[NRSTATIONS][NRBEAMS] __attribute__ ((aligned(32)));

        // construct the weights, with zeroes for unused data
        for( unsigned s = 0; s < NRSTATIONS; s++ ) {
	  if( validStation[stat+s] ) {
            for( unsigned b = 0; b < NRBEAMS; b++ ) {
	      weights[s][b] = phaseShift( frequency, itsDelays[stat+s][beam+b] ) * factor;
	    }
	  } else {
	    // a dropped station has a weight of 0
            for( unsigned b = 0; b < NRBEAMS; b++ ) {
	      weights[s][b] = makefcomplex( 0, 0 );
	    }
	  }
	}

	// beam form
        FUNC(NRSTATIONS,NRBEAMS)(
          out->samples[ch][beam].origin(),
	  out->samples[ch][beam].strides()[0] * sizeof out->samples[0][0][0][0],

	  in->samples[ch][stat].origin(),
	  in->samples[ch][stat].strides()[0] * sizeof in->samples[0][0][0][0],

          &weights[0][0],
	  (&weights[1][0] - &weights[0][0]) * sizeof weights[0][0],

	  itsNrSamplesPerIntegration,
	  stat == 0
        );
      }
    }
  }

  #undef FUNC
  #undef FUNC_
  #undef NRBEAMS
  #undef NRSTATIONS

/*
        if( !out->flags[beam].test(time) ) {
          for (unsigned pol = 0; pol < NR_POLARIZATIONS; pol ++) {
            fcomplex &dest  = out->samples[ch][beam][time][pol];

            // combine the stations for this beam
            fcomplex sample = makefcomplex( 0, 0 );

            for( unsigned stat = 0; stat < validStations.size(); stat++ ) {
              // note: for beam #0 (central beam), the phaseShift is 1
              const fcomplex shift = phaseShift( frequency, itsDelays[validStations[stat]][beam] );
              sample += in->samples[ch][validStations[stat]][time][pol] * shift;
            }
            dest = sample * factor;
          }
        } else {
          // data is flagged
          for (unsigned pol = 0; pol < NR_POLARIZATIONS; pol ++) {
            out->samples[ch][beam][time][pol] = makefcomplex( 0, 0 );
  	  }
	}
      }
    }
  }
*/
}

#endif

void PencilBeams::calculateAllDelays( const FilteredData *filteredData )
{
  // calculate the delays for each station for this integration period
  for( unsigned stat = 0; stat < itsNrStations; stat++ ) {
    // todo: interpolate per time step?
    PencilCoord3D beamDirBegin = filteredData->metaData[stat].beamDirectionAtBegin;
    PencilCoord3D beamDirEnd = filteredData->metaData[stat].beamDirectionAfterEnd;
    PencilCoord3D beamDirAverage = (beamDirBegin + beamDirEnd) * 0.5;

    calculateDelays( stat, beamDirAverage );
  }
}

void PencilBeams::formPencilBeams( const FilteredData *filteredData, PencilBeamData *pencilBeamData )
{
  // TODO: fetch a list of stations to beam form. for now, we assume
  // we use all stations

  pencilBeamFormTimer.start();

  calculateAllDelays( filteredData );

  // select stations
  std::vector<unsigned> stations;

  for( unsigned stat = 0; stat < itsNrStations; stat++ ) {
    stations.push_back( stat );
  }

  computeComplexVoltages( filteredData, pencilBeamData, stations );

  pencilBeamFormTimer.stop();
}

} // namespace RTCP
} // namespace LOFAR


