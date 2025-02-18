#ifndef _PRICE_MARKET_H_
#define _PRICE_MARKET_H_
#if defined(_MSC_VER_)
#pragma once
#endif

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
* \file price_market.h
* \ingroup Objects
* \brief The PriceMarket class header file.
* \author Steve Smith
*/

#include "marketplace/include/market.h"
class Tabs;

/*!
* \ingroup Objects
* \brief A derived class which defines a single PriceMarket object for use in solving simultaneous markets.
* \author Steve Smith
*/

class PriceMarket: public Market {
public:
    PriceMarket( const std::string& goodNameIn, const std::string& regionNameIn, const int periodIn, Market* demandMarketIn );
    PriceMarket( const Market& marketIn, Market* demandMarketIn );

    virtual IMarketType::Type getType() const;

    virtual void initPrice();
    virtual void setPrice( const double priceIn );
    virtual void set_price_to_last_if_default( const double lastPrice );
    virtual void set_price_to_last( const double lastPrice );
    virtual double getPrice() const;

    virtual void addToDemand( const double demandIn );
    virtual double getDemand() const;

    virtual void nullSupply();
    virtual double getSupply() const;
    virtual void addToSupply( const double supplyIn );
    
    virtual bool meetsSpecialSolutionCriteria() const;
    virtual bool shouldSolve() const;
    virtual bool shouldSolveNR() const;
protected:
    virtual void toDebugXMLDerived( std::ostream& out, Tabs* tabs ) const;
private:
    Market* demandMarketPointer; //!< A pointer to the companion DemandMarket
    
    //! Flag to indicate if the price was negative which will have to be
    //! hidden since the solver will not work with negative prices
    bool mIsPriceNegative;
};

#endif // _PRICE_MARKET_H_
