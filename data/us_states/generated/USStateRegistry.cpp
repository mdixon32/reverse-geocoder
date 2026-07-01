#include "USStateRegistry.h"

namespace rg {

CountryBoundary makeUSStateBoundaryAK();
CountryBoundary makeUSStateBoundaryAL();
CountryBoundary makeUSStateBoundaryAR();
CountryBoundary makeUSStateBoundaryAZ();
CountryBoundary makeUSStateBoundaryCA();
CountryBoundary makeUSStateBoundaryCO();
CountryBoundary makeUSStateBoundaryCT();
CountryBoundary makeUSStateBoundaryDE();
CountryBoundary makeUSStateBoundaryFL();
CountryBoundary makeUSStateBoundaryGA();
CountryBoundary makeUSStateBoundaryHI();
CountryBoundary makeUSStateBoundaryIA();
CountryBoundary makeUSStateBoundaryID();
CountryBoundary makeUSStateBoundaryIL();
CountryBoundary makeUSStateBoundaryIN();
CountryBoundary makeUSStateBoundaryKS();
CountryBoundary makeUSStateBoundaryKY();
CountryBoundary makeUSStateBoundaryLA();
CountryBoundary makeUSStateBoundaryMA();
CountryBoundary makeUSStateBoundaryMD();
CountryBoundary makeUSStateBoundaryME();
CountryBoundary makeUSStateBoundaryMI();
CountryBoundary makeUSStateBoundaryMN();
CountryBoundary makeUSStateBoundaryMO();
CountryBoundary makeUSStateBoundaryMS();
CountryBoundary makeUSStateBoundaryMT();
CountryBoundary makeUSStateBoundaryNC();
CountryBoundary makeUSStateBoundaryND();
CountryBoundary makeUSStateBoundaryNE();
CountryBoundary makeUSStateBoundaryNH();
CountryBoundary makeUSStateBoundaryNJ();
CountryBoundary makeUSStateBoundaryNM();
CountryBoundary makeUSStateBoundaryNV();
CountryBoundary makeUSStateBoundaryNY();
CountryBoundary makeUSStateBoundaryOH();
CountryBoundary makeUSStateBoundaryOK();
CountryBoundary makeUSStateBoundaryOR();
CountryBoundary makeUSStateBoundaryPA();
CountryBoundary makeUSStateBoundaryRI();
CountryBoundary makeUSStateBoundarySC();
CountryBoundary makeUSStateBoundarySD();
CountryBoundary makeUSStateBoundaryTN();
CountryBoundary makeUSStateBoundaryTX();
CountryBoundary makeUSStateBoundaryUT();
CountryBoundary makeUSStateBoundaryVA();
CountryBoundary makeUSStateBoundaryVT();
CountryBoundary makeUSStateBoundaryWA();
CountryBoundary makeUSStateBoundaryWI();
CountryBoundary makeUSStateBoundaryWV();
CountryBoundary makeUSStateBoundaryWY();

std::vector<CountryBoundary> loadGeneratedUSStates() {
  return {
      makeUSStateBoundaryAK(),
      makeUSStateBoundaryAL(),
      makeUSStateBoundaryAR(),
      makeUSStateBoundaryAZ(),
      makeUSStateBoundaryCA(),
      makeUSStateBoundaryCO(),
      makeUSStateBoundaryCT(),
      makeUSStateBoundaryDE(),
      makeUSStateBoundaryFL(),
      makeUSStateBoundaryGA(),
      makeUSStateBoundaryHI(),
      makeUSStateBoundaryIA(),
      makeUSStateBoundaryID(),
      makeUSStateBoundaryIL(),
      makeUSStateBoundaryIN(),
      makeUSStateBoundaryKS(),
      makeUSStateBoundaryKY(),
      makeUSStateBoundaryLA(),
      makeUSStateBoundaryMA(),
      makeUSStateBoundaryMD(),
      makeUSStateBoundaryME(),
      makeUSStateBoundaryMI(),
      makeUSStateBoundaryMN(),
      makeUSStateBoundaryMO(),
      makeUSStateBoundaryMS(),
      makeUSStateBoundaryMT(),
      makeUSStateBoundaryNC(),
      makeUSStateBoundaryND(),
      makeUSStateBoundaryNE(),
      makeUSStateBoundaryNH(),
      makeUSStateBoundaryNJ(),
      makeUSStateBoundaryNM(),
      makeUSStateBoundaryNV(),
      makeUSStateBoundaryNY(),
      makeUSStateBoundaryOH(),
      makeUSStateBoundaryOK(),
      makeUSStateBoundaryOR(),
      makeUSStateBoundaryPA(),
      makeUSStateBoundaryRI(),
      makeUSStateBoundarySC(),
      makeUSStateBoundarySD(),
      makeUSStateBoundaryTN(),
      makeUSStateBoundaryTX(),
      makeUSStateBoundaryUT(),
      makeUSStateBoundaryVA(),
      makeUSStateBoundaryVT(),
      makeUSStateBoundaryWA(),
      makeUSStateBoundaryWI(),
      makeUSStateBoundaryWV(),
      makeUSStateBoundaryWY(),
  };
}

}  // namespace rg
