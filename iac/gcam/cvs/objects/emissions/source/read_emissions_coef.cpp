/*
 * LEGAL NOTICE
 * This computer software was prepared by Battelle Memorial Institute,
 * hereinafter the Contractor, under Contract No. DE-AC05-76RL0 1830
 * with the Department of Energy (DOE). NEITHER THE GOVERNMENT NOR THE
 * CONTRACTOR MAKES ANY WARRANTY, EXPRESS OR IMPLIED, OR ASSUMES ANY
 * LIABILITY FOR THE USE OF THIS SOFTWARE. This notice including this
 * sentence must appear on any copies of this computer software.
 * 
 * EXPORT CONTROL
 * User agrees that the Software will not be shipped, transferred or
 * exported into any country or used in any manner prohibited by the
 * United States Export Administration Act or any other applicable
 * export laws, restrictions or regulations (collectively the "Export Laws").
 * Export of the Software may require some form of license or other
 * authority from the U.S. Government, and failure to obtain such
 * export control license may result in criminal liability under
 * U.S. laws. In addition, if the Software is identified as export controlled
 * items under the Export Laws, User represents and warrants that User
 * is not a citizen, or otherwise located within, an embargoed nation
 * (including without limitation Iran, Syria, Sudan, Cuba, and North Korea)
 *     and that User is not otherwise prohibited
 * under the Export Laws from receiving the Software.
 * 
 * Copyright 2011 Battelle Memorial Institute.  All Rights Reserved.
 * Distributed as open-source under the terms of the Educational Community 
 * License version 2.0 (ECL 2.0). http://www.opensource.org/licenses/ecl2.php
 * 
 * For further details, see: http://www.globalchange.umd.edu/models/gcam/
 * 
 */

/*!
 * \file read_emissions_coef.cpp
 * \ingroup Objects
 * \brief ReadEmissionsCoef source file.
 * \author Jim Naslund
 */

#include "util/base/include/definitions.h"

#include "emissions/include/read_emissions_coef.h"

using namespace std;

//! Constructor that initializes the emissions coefficient.
ReadEmissionsCoef::ReadEmissionsCoef( const double aEmissionsCoef ):
AEmissionsCoef( aEmissionsCoef )
{
}

//! Clone operator.
ReadEmissionsCoef* ReadEmissionsCoef::clone() const {
    return new ReadEmissionsCoef( *this );
}

void ReadEmissionsCoef::initCalc( const IInfo* aSubsectorInfo, const string& aName, const int aPeriod ){
}

void ReadEmissionsCoef::overrideCoef( const double aEmissionsCoef ){
    mOverridesFuturePeriods = true;
    mEmissionsCoef = aEmissionsCoef;
}

const string& ReadEmissionsCoef::getXMLName() const{
    static const string XML_NAME = "uncontrolled-emiss-coef";
    return XML_NAME;
}
