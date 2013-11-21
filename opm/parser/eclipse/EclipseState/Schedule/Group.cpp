/*
  Copyright 2013 Statoil ASA.

  This file is part of the Open Porous Media project (OPM).

  OPM is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  OPM is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with OPM.  If not, see <http://www.gnu.org/licenses/>.
 */



#include <boost/date_time.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/Group.hpp>

namespace Opm {
    namespace GroupProduction {
        struct ProductionData {
            ProductionData(TimeMapConstPtr timeMap);

            boost::shared_ptr<DynamicState<GroupProduction::ControlEnum> > controlMode;
            boost::shared_ptr<DynamicState<GroupProductionExceedLimit::ActionEnum> > exceedAction;
            boost::shared_ptr<DynamicState<double> > oilTarget;
            boost::shared_ptr<DynamicState<double> > waterTarget;
            boost::shared_ptr<DynamicState<double> > gasTarget;
            boost::shared_ptr<DynamicState<double> > liquidTarget;
            
        };
        
        ProductionData::ProductionData(TimeMapConstPtr timeMap) : 
            controlMode( new DynamicState<GroupProduction::ControlEnum>(timeMap , GroupProduction::NONE)),
            exceedAction( new DynamicState<GroupProductionExceedLimit::ActionEnum>(timeMap , GroupProductionExceedLimit::NONE)),
            oilTarget( new DynamicState<double>(timeMap , 0)),
            waterTarget( new DynamicState<double>(timeMap , 0)),
            gasTarget( new DynamicState<double>(timeMap , 0)),
            liquidTarget( new DynamicState<double>(timeMap , 0))
        {

        }
    }

    
    

    namespace GroupInjection {
        struct InjectionData {
            InjectionData(TimeMapConstPtr timeMap);
            
            boost::shared_ptr<DynamicState<PhaseEnum> > phase;
            boost::shared_ptr<DynamicState<GroupInjection::ControlEnum> > controlMode;
            boost::shared_ptr<DynamicState<double> > rate;
            boost::shared_ptr<DynamicState<double> > surfaceFlowMaxRate;
            boost::shared_ptr<DynamicState<double> > reservoirFlowMaxRate;
            boost::shared_ptr<DynamicState<double> > targetReinjectFraction;
            boost::shared_ptr<DynamicState<double> > targetVoidReplacementFraction;                
        };
        
        InjectionData::InjectionData(TimeMapConstPtr timeMap) : 
            phase( new DynamicState<PhaseEnum>( timeMap , WATER )),
            controlMode( new DynamicState<GroupInjection::ControlEnum>( timeMap , NONE )),
            rate( new DynamicState<double>( timeMap , 0 )),
            surfaceFlowMaxRate( new DynamicState<double>( timeMap , 0)),
            reservoirFlowMaxRate( new DynamicState<double>( timeMap , 0)),
            targetReinjectFraction( new DynamicState<double>( timeMap , 0)),
            targetVoidReplacementFraction( new DynamicState<double>( timeMap , 0))
        {
        
        }
    }

    /*****************************************************************/
    
    Group::Group(const std::string& name , TimeMapConstPtr timeMap) : 
        m_injection( new GroupInjection::InjectionData(timeMap) ),
        m_production( new GroupProduction::ProductionData( timeMap ))
    {
        m_name = name;
    }


    const std::string& Group::name() const {
        return m_name;
    }

    
    void Group::setInjectionPhase(size_t time_step , PhaseEnum phase){
        if (m_injection->phase->size() == time_step + 1) {
            PhaseEnum currentPhase = m_injection->phase->get(time_step);
            /*
              The ECLIPSE documentation of the GCONINJE keyword seems
              to indicate that a group can inject more than one phase
              simultaneously. This should be implemented in the input
              file as:

              GCONINJE
                 'GROUP'   'PHASE1'    'RATE'   ... /
                 'GROUP'   'PHASE2'    'RATE'   ... /
                 ...
              /

              I.e. the same group occurs more than once at the same
              time step, with different phases. This seems quite
              weird, and we do currently not support it. Changing the
              injected phase from one time step to the next is
              supported.
            */
            if (phase != currentPhase)
                throw std::invalid_argument("Sorry - we currently do not support injecting multiple phases at the same time.");
        }
        m_injection->phase->add( time_step , phase );
    }

    PhaseEnum Group::getInjectionPhase( size_t time_step ) const {
        return m_injection->phase->get( time_step );
    }

    void Group::setInjectionRate( size_t time_step , double rate) {
        return m_injection->rate->add( time_step , rate);
    }

    double Group::getInjectionRate( size_t time_step ) const {
        return m_injection->rate->get( time_step );
    }

    void Group::setInjectionControlMode(size_t time_step , GroupInjection::ControlEnum controlMode) {
        m_injection->controlMode->add( time_step , controlMode );
    }

    GroupInjection::ControlEnum Group::getInjectionControlMode( size_t time_step) const {
        return m_injection->controlMode->get( time_step );
    }

    void Group::setSurfaceMaxRate( size_t time_step , double rate) {
        return m_injection->surfaceFlowMaxRate->add( time_step , rate);
    }

    double Group::getSurfaceMaxRate( size_t time_step ) const {
        return m_injection->surfaceFlowMaxRate->get( time_step );
    }

    void Group::setReservoirMaxRate( size_t time_step , double rate) {
        return m_injection->reservoirFlowMaxRate->add( time_step , rate);
    }

    double Group::getReservoirMaxRate( size_t time_step ) const {
        return m_injection->reservoirFlowMaxRate->get( time_step );
    }

    void Group::setTargetReinjectFraction( size_t time_step , double rate) {
        return m_injection->targetReinjectFraction->add( time_step , rate);
    }

    double Group::getTargetReinjectFraction( size_t time_step ) const {
        return m_injection->targetReinjectFraction->get( time_step );
    }

    void Group::setTargetVoidReplacementFraction( size_t time_step , double rate) {
        return m_injection->targetVoidReplacementFraction->add( time_step , rate);
    }

    double Group::getTargetVoidReplacementFraction( size_t time_step ) const {
        return m_injection->targetVoidReplacementFraction->get( time_step );
    }

    /*****************************************************************/

    void Group::setProductionControlMode( size_t time_step , GroupProduction::ControlEnum controlMode) {
        m_production->controlMode->add(time_step , controlMode );
    }
    
    GroupProduction::ControlEnum Group::getProductionControlMode( size_t time_step ) const {
        return m_production->controlMode->get(time_step);
    }


    GroupProductionExceedLimit::ActionEnum Group::getProductionExceedLimitAction( size_t time_step ) const  {
        return m_production->exceedAction->get(time_step);
    }


    void Group::setProductionExceedLimitAction( size_t time_step , GroupProductionExceedLimit::ActionEnum action) {
        m_production->exceedAction->add(time_step , action);
    }


    void Group::setOilTargetRate(size_t time_step , double oilTargetRate) {
        m_production->oilTarget->add(time_step , oilTargetRate);
    }


    double Group::getOilTargetRate(size_t time_step) {
        return m_production->oilTarget->get(time_step);
    }


    void Group::setGasTargetRate(size_t time_step , double gasTargetRate) {
        m_production->gasTarget->add(time_step , gasTargetRate);
    }


    double Group::getGasTargetRate(size_t time_step) {
        return m_production->gasTarget->get(time_step);
    }


    void Group::setWaterTargetRate(size_t time_step , double waterTargetRate) {
        m_production->waterTarget->add(time_step , waterTargetRate);
    }


    double Group::getWaterTargetRate(size_t time_step) {
        return m_production->waterTarget->get(time_step);
    }


    void Group::setLiquidTargetRate(size_t time_step , double liquidTargetRate) {
        m_production->liquidTarget->add(time_step , liquidTargetRate);
    }


    double Group::getLiquidTargetRate(size_t time_step) {
        return m_production->liquidTarget->get(time_step);
    }


    

}


