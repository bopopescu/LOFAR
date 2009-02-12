//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

#include <Stokes.h>
#include <Interface/MultiDimArray.h>

namespace LOFAR {
namespace RTCP {

Stokes::Stokes( const bool coherent, const int nrStokes, const unsigned nrChannels, const unsigned nrSamplesPerIntegration, const unsigned nrSamplesPerStokesIntegration ):
  itsNrChannels(nrChannels),
  itsNrSamplesPerIntegration(nrSamplesPerIntegration),
  itsNrSamplesPerStokesIntegration(nrSamplesPerStokesIntegration),
  itsNrStokes(nrStokes),
  itsIsCoherent(coherent)
{
} 

void Stokes::calculateCoherent( const PencilBeamData *pencilBeamData, StokesData *stokesData, const unsigned nrBeams )
{
  computeCoherentStokes( pencilBeamData->samples, pencilBeamData->flags, stokesData, nrBeams );
}

void Stokes::calculateIncoherent( const FilteredData *filteredData, StokesData *stokesData, const unsigned nrStations )
{
  computeIncoherentStokes( filteredData->samples, filteredData->flags, stokesData, nrStations );
}

static inline float sqr( const float x ) { return x * x; }

struct stokes {
  float I, Q, U, V;
  float nrValidSamples;
};

static inline void addStokes( struct stokes stokes, const fcomplex &polX, const fcomplex &polY, bool allStokes )
{
  // assert: two polarizations
  const float powerX = sqr( real(polX) ) + sqr( imag(polX) );
  const float powerY = sqr( real(polY) ) + sqr( imag(polY) );

  stokes.I += powerX + powerY;
  if( allStokes ) {
    stokes.Q += powerX - powerY;
    stokes.U += real(polX * conj(polY));
    stokes.V += imag(polX * conj(polY));
  }
  stokes.nrValidSamples++;
}

void Stokes::computeCoherentStokes( const MultiDimArray<fcomplex,4> &in, const SparseSet<unsigned> *inflags, StokesData *out, const unsigned nrBeams )
{
  const unsigned &integrationSteps = itsNrSamplesPerStokesIntegration;
  const bool allStokes = itsNrStokes == 4;

  // conservative flagging; flag output if any of the inputs are flagged
  for(unsigned beam = 0; beam < nrBeams; beam++) {
    out->flags[beam] = inflags[beam];
  }

  for(unsigned beam = 0; beam < nrBeams; beam++) {
    out->flags[beam] /= integrationSteps;
  }

  for (unsigned ch = 0; ch < itsNrChannels; ch ++) {
    for (unsigned time = 0; time < itsNrSamplesPerIntegration; time += integrationSteps ) {
      for( unsigned beam = 0; beam < nrBeams; beam++ ) {
        struct stokes stokes = { 0, 0, 0, 0, 0 };

        for( unsigned fractime = 0; fractime < integrationSteps; fractime++ ) {
            if( inflags[beam].test( time+fractime ) ) {
              continue;
            }

	    addStokes( stokes, in[ch][beam][time+fractime][0], in[ch][beam][time+fractime][1], allStokes );
        }

        /* hack: if no valid samples, insert zeroes */
        if( stokes.nrValidSamples == 0 ) { stokes.nrValidSamples = 1; }

        #define dest out->samples[ch][beam][time / integrationSteps]
        dest[0] = stokes.I / stokes.nrValidSamples;
        if( allStokes ) {
          dest[1] = stokes.Q / stokes.nrValidSamples;
          dest[2] = stokes.U / stokes.nrValidSamples;
          dest[3] = stokes.V / stokes.nrValidSamples;
        }
        #undef dest
      }
    }
  }
}

void Stokes::computeIncoherentStokes( const MultiDimArray<fcomplex,4> &in, const SparseSet<unsigned> *inflags, StokesData *out, const unsigned nrStations )
{
  const unsigned &integrationSteps = itsNrSamplesPerStokesIntegration;
  const bool allStokes = itsNrStokes == 4;
  const unsigned upperBound = static_cast<unsigned>(itsNrSamplesPerIntegration * Stokes::MAX_FLAGGED_PERCENTAGE);
  bool validStation[nrStations];

  out->flags[0].reset();

  for(unsigned stat = 0; stat < nrStations; stat++) {
    if( inflags[stat].count() > upperBound ) {
      // drop station due to too much flagging
      validStation[stat] = false;
    } else {
      validStation[stat] = true;

      out->flags[0] |= inflags[stat];
    }
  }

  out->flags[0] /= integrationSteps;

  for (unsigned ch = 0; ch < itsNrChannels; ch ++) {
    for (unsigned time = 0; time < itsNrSamplesPerIntegration; time += integrationSteps ) {
      struct stokes stokes = { 0, 0, 0, 0, 0 };

      for( unsigned stat = 0; stat < nrStations; stat++ ) {
        if( !validStation[stat] ) {
	  continue;
	}

        for( unsigned fractime = 0; fractime < integrationSteps; fractime++ ) {
            if( inflags[stat].test( time+fractime ) ) {
              continue;
            }

	    addStokes( stokes, in[ch][stat][time+fractime][0], in[ch][stat][time+fractime][1], allStokes );
        }
      }

      /* hack: if no valid samples, insert zeroes */
      if( stokes.nrValidSamples == 0 ) { stokes.nrValidSamples = 1; }

      #define dest out->samples[ch][0][time / integrationSteps]
      dest[0] = stokes.I / stokes.nrValidSamples;
      if( allStokes ) {
        dest[1] = stokes.Q / stokes.nrValidSamples;
        dest[2] = stokes.U / stokes.nrValidSamples;
        dest[3] = stokes.V / stokes.nrValidSamples;
      }
      #undef dest
    }
  }
}

} // namespace RTCP
} // namespace LOFAR
