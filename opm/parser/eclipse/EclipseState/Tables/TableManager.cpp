/*
  Copyright 2015 Statoil ASA.

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

#include <opm/parser/eclipse/Parser/ParserKeywords.hpp>
#include <opm/parser/eclipse/Deck/Deck.hpp>
#include <opm/parser/eclipse/EclipseState/Tables/TableManager.hpp>
#include <opm/parser/eclipse/OpmLog/OpmLog.hpp>

namespace Opm {

    TableManager::TableManager( const Deck& deck ) {
        initDims( deck );
        initSimpleTables( deck );
        initFullTables(deck, "PVTG", m_pvtgTables);
        initFullTables(deck, "PVTO", m_pvtoTables);

        initVFPProdTables(deck, m_vfpprodTables);
        initVFPInjTables(deck,  m_vfpinjTables);
    }


    void TableManager::initDims(const Deck& deck) {
        using namespace Opm::ParserKeywords;
        if (deck.hasKeyword<TABDIMS>()) {
            auto keyword = deck.getKeyword<TABDIMS>();
            auto record = keyword->getRecord(0);
            int ntsfun = record->getItem<TABDIMS::NTSFUN>()->getInt(0);
            int ntpvt  = record->getItem<TABDIMS::NTPVT>()->getInt(0);
            int nssfun = record->getItem<TABDIMS::NSSFUN>()->getInt(0);
            int nppvt  = record->getItem<TABDIMS::NPPVT>()->getInt(0);
            int ntfip  = record->getItem<TABDIMS::NTFIP>()->getInt(0);
            int nrpvt  = record->getItem<TABDIMS::NRPVT>()->getInt(0);

            m_tabdims = std::make_shared<Tabdims>(ntsfun , ntpvt , nssfun , nppvt , ntfip , nrpvt);
        } else
            m_tabdims = std::make_shared<Tabdims>();

        if (deck.hasKeyword<EQLDIMS>()) {
            auto keyword = deck.getKeyword<EQLDIMS>();
            auto record = keyword->getRecord(0);
            int ntsequl   = record->getItem<EQLDIMS::NTEQUL>()->getInt(0);
            int nodes_p   = record->getItem<EQLDIMS::DEPTH_NODES_P>()->getInt(0);
            int nodes_tab = record->getItem<EQLDIMS::DEPTH_NODES_TAB>()->getInt(0);
            int nttrvd    = record->getItem<EQLDIMS::NTTRVD>()->getInt(0);
            int ntsrvd    = record->getItem<EQLDIMS::NSTRVD>()->getInt(0);

            m_eqldims = std::make_shared<Eqldims>(ntsequl , nodes_p , nodes_tab , nttrvd , ntsrvd );
        } else
            m_eqldims = std::make_shared<Eqldims>();

        if (deck.hasKeyword<REGDIMS>()) {
            auto keyword = deck.getKeyword<REGDIMS>();
            auto record = keyword->getRecord(0);
            int ntfip  = record->getItem<REGDIMS::NTFIP>()->getInt(0);
            int nmfipr = record->getItem<REGDIMS::NMFIPR>()->getInt(0);
            int nrfreg = record->getItem<REGDIMS::NRFREG>()->getInt(0);
            int ntfreg = record->getItem<REGDIMS::NTFREG>()->getInt(0);
            int nplmix = record->getItem<REGDIMS::NPLMIX>()->getInt(0);
            m_regdims = std::make_shared<Regdims>( ntfip , nmfipr , nrfreg , ntfreg , nplmix );
        } else
            m_regdims = std::make_shared<Regdims>();
    }


    void TableManager::addTables( const std::string& tableName , size_t numTables) {
        m_simpleTables.emplace(std::make_pair(tableName , TableContainer( numTables )));
    }


    bool TableManager::hasTables( const std::string& tableName ) const {
        auto pair = m_simpleTables.find( tableName );
        if (pair == m_simpleTables.end())
            return false;
        else {
            const auto& tables = pair->second;
            return !tables.empty();
        }
    }


    const TableContainer& TableManager::getTables( const std::string& tableName ) const {
        auto pair = m_simpleTables.find( tableName );
        if (pair == m_simpleTables.end())
            throw std::invalid_argument("No such table collection: " + tableName);
        else
            return pair->second;
    }

    TableContainer& TableManager::forceGetTables( const std::string& tableName , size_t numTables )  {
        auto pair = m_simpleTables.find( tableName );
        if (pair == m_simpleTables.end()) {
            addTables( tableName , numTables );
            pair = m_simpleTables.find( tableName );
        }
        return pair->second;
    }


    const TableContainer& TableManager::operator[](const std::string& tableName) const {
        return getTables(tableName);
    }

    void TableManager::initSimpleTables(const Deck& deck) {
        addTables( "SWOF" , m_tabdims->getNumSatTables() );
        addTables( "SGOF",  m_tabdims->getNumSatTables() );
        addTables( "SLGOF", m_tabdims->getNumSatTables() );
        addTables( "SOF2",  m_tabdims->getNumSatTables() );
        addTables( "SOF3",  m_tabdims->getNumSatTables() );
        addTables( "SWFN",  m_tabdims->getNumSatTables() );
        addTables( "SGFN",  m_tabdims->getNumSatTables() );
        addTables( "SSFN",  m_tabdims->getNumSatTables() );

        addTables( "PLYADS", m_tabdims->getNumSatTables() );
        addTables( "PLYROCK", m_tabdims->getNumSatTables());
        addTables( "PLYVISC", m_tabdims->getNumPVTTables());
        addTables( "PLYDHFLF", m_tabdims->getNumPVTTables());

        addTables( "PVDG", m_tabdims->getNumPVTTables());
        addTables( "PVDO", m_tabdims->getNumPVTTables());
        addTables( "PVDS", m_tabdims->getNumPVTTables());

        addTables( "OILVISCT", m_tabdims->getNumPVTTables());
        addTables( "WATVISCT", m_tabdims->getNumPVTTables());


        addTables( "PLYMAX", m_regdims->getNPLMIX());
        addTables( "RSVD", m_eqldims->getNumEquilRegions());
        addTables( "RVVD", m_eqldims->getNumEquilRegions());

        {
            size_t numEndScaleTables = ParserKeywords::ENDSCALE::NUM_TABLES::defaultValue;

            if (deck.hasKeyword<ParserKeywords::ENDSCALE>()) {
                auto keyword = deck.getKeyword<ParserKeywords::ENDSCALE>();
                auto record = keyword->getRecord(0);
                numEndScaleTables = static_cast<size_t>(record->getItem<ParserKeywords::ENDSCALE::NUM_TABLES>()->getInt(0));
            }

            addTables( "ENKRVD", numEndScaleTables);
            addTables( "ENPTVD", numEndScaleTables);
            addTables( "IMKRVD", numEndScaleTables);
            addTables( "IMPTVD", numEndScaleTables);
        }
        // Unhandled:

        /*
          initPlyshlogTables(deck, "PLYSHLOG", m_plyshlogTables);
          initRocktabTables(deck);
          initRTempTables(deck);
          initGasvisctTables(deck, "GASVISCT", m_gasvisctTables);
        */
        initSimpleTableContainer<SwofTable>(deck, "SWOF" , m_tabdims->getNumSatTables());
        initSimpleTableContainer<SgofTable>(deck, "SGOF" , m_tabdims->getNumSatTables());
        initSimpleTableContainer<SlgofTable>(deck, "SLGOF" , m_tabdims->getNumSatTables());
        initSimpleTableContainer<Sof2Table>(deck, "SOF2" , m_tabdims->getNumSatTables());
        initSimpleTableContainer<Sof3Table>(deck, "SOF3" , m_tabdims->getNumSatTables());
        initSimpleTableContainer<SwfnTable>(deck, "SWFN" , m_tabdims->getNumSatTables());
        initSimpleTableContainer<SgfnTable>(deck, "SGFN" , m_tabdims->getNumSatTables());
        initSimpleTableContainer<SsfnTable>(deck, "SSFN" , m_tabdims->getNumSatTables());

        initSimpleTableContainer<RsvdTable>(deck, "RSVD" , m_eqldims->getNumEquilRegions());
        initSimpleTableContainer<RvvdTable>(deck, "RVVD" , m_eqldims->getNumEquilRegions());
        {
            size_t numEndScaleTables = ParserKeywords::ENDSCALE::NUM_TABLES::defaultValue;

            if (deck.hasKeyword<ParserKeywords::ENDSCALE>()) {
                auto keyword = deck.getKeyword<ParserKeywords::ENDSCALE>();
                auto record = keyword->getRecord(0);
                numEndScaleTables = static_cast<size_t>(record->getItem<ParserKeywords::ENDSCALE::NUM_TABLES>()->getInt(0));
            }

            initSimpleTableContainer<EnkrvdTable>( deck , "ENKRVD", numEndScaleTables);
            initSimpleTableContainer<EnptvdTable>( deck , "ENPTVD", numEndScaleTables);
            initSimpleTableContainer<ImkrvdTable>( deck , "IMKRVD", numEndScaleTables);
            initSimpleTableContainer<ImptvdTable>( deck , "IMPTVD", numEndScaleTables);
        }
        initSimpleTableContainer<PvdgTable>(deck, "PVDG", m_tabdims->getNumPVTTables());
        initSimpleTableContainer<PvdoTable>(deck, "PVDO", m_tabdims->getNumPVTTables());
        initSimpleTableContainer<PvdsTable>(deck, "PVDS", m_tabdims->getNumPVTTables());

        /*****************************************************************/


        initSimpleTable(deck, "PLYADS", m_plyadsTables);
        initSimpleTable(deck, "PLYVISC", m_plyviscTables);
        initSimpleTable(deck, "PLYDHFLF", m_plydhflfTables);
        initSimpleTable(deck, "OILVISCT", m_oilvisctTables);
        initSimpleTable(deck, "WATVISCT", m_watvisctTables);

        initPlyrockTables(deck , "PLYROCK" , m_plyrockTables);
        initPlymaxTables(deck , "PLYMAX" , m_plymaxTables);
        initPlyshlogTables(deck, "PLYSHLOG", m_plyshlogTables);
        initRocktabTables(deck);
        initRTempTables(deck);
        initGasvisctTables(deck, "GASVISCT", m_gasvisctTables);
    }


    void TableManager::initRTempTables(const Deck& deck) {
        // the temperature vs depth table. the problem here is that
        // the TEMPVD (E300) and RTEMPVD (E300 + E100) keywords are
        // synonymous, but we want to provide only a single cannonical
        // API here, so we jump through some small hoops...
        if (deck.hasKeyword("TEMPVD") && deck.hasKeyword("RTEMPVD"))
            throw std::invalid_argument("The TEMPVD and RTEMPVD tables are mutually exclusive!");
        else if (deck.hasKeyword("TEMPVD"))
            initSimpleTable(deck, "TEMPVD", m_rtempvdTables);
        else if (deck.hasKeyword("RTEMPVD"))
            initSimpleTable(deck, "RTEMPVD", m_rtempvdTables);

    }


    void TableManager::initGasvisctTables(const Deck& deck,
                                    const std::string& keywordName,
                                    std::vector<GasvisctTable>& tableVector) {
        if (!deck.hasKeyword(keywordName))
            return; // the table is not featured by the deck...

        if (deck.numKeywords(keywordName) > 1) {
            complainAboutAmbiguousKeyword(deck, keywordName);
            return;
        }

        const auto& tableKeyword = deck.getKeyword(keywordName);
        for (size_t tableIdx = 0; tableIdx < tableKeyword->size(); ++tableIdx) {
            if (tableKeyword->getRecord(tableIdx)->getItem(0)->size() == 0) {
                // for simple tables, an empty record indicates that the< previous table
                // should be copied...
                if (tableIdx == 0) {
                    std::string msg = "The first table for keyword " + keywordName + " must be explicitly defined! Ignoring keyword";
                    OpmLog::addMessage(Log::MessageType::Error , Log::fileMessage(tableKeyword->getFileName(),tableKeyword->getLineNumber(),msg));
                    return;
                }
                tableVector.push_back(tableVector.back());
                continue;
            }

            tableVector.push_back(GasvisctTable());
            tableVector[tableIdx].init(deck, tableKeyword->getRecord(tableIdx)->getItem(0));
        }
    }

    void TableManager::initPlyshlogTables(const Deck& deck,
                                          const std::string& keywordName,
                                          std::vector<PlyshlogTable>& tableVector){

        if (!deck.hasKeyword(keywordName)) {
            return;
        }

        if (!deck.numKeywords(keywordName)) {
            complainAboutAmbiguousKeyword(deck, keywordName);
            return;
        }

        const auto& keyword = deck.getKeyword(keywordName);

        tableVector.push_back(PlyshlogTable());

        tableVector[0].init(keyword);

    }


    void TableManager::initPlyrockTables(const Deck& deck,
                                         const std::string& keywordName,
                                         std::vector<PlyrockTable>& tableVector){

        if (!deck.hasKeyword(keywordName)) {
            return;
        }

        if (!deck.numKeywords(keywordName)) {
            complainAboutAmbiguousKeyword(deck, keywordName);
            return;
        }

        const auto& keyword = deck.getKeyword(keywordName);
        for( auto iter = keyword->begin(); iter != keyword->end(); ++iter) {
            auto record = *iter;
            tableVector.push_back( PlyrockTable() );
            tableVector.back().init( record );
        }
    }


    void TableManager::initPlymaxTables(const Deck& deck,
                                        const std::string& keywordName,
                                        std::vector<PlymaxTable>& tableVector){

        if (!deck.hasKeyword(keywordName)) {
            return;
        }

        if (!deck.numKeywords(keywordName)) {
            complainAboutAmbiguousKeyword(deck, keywordName);
            return;
        }

        const auto& keyword = deck.getKeyword(keywordName);
        for( auto iter = keyword->begin(); iter != keyword->end(); ++iter) {
            auto record = *iter;
            tableVector.push_back( PlymaxTable() );
            tableVector.back().init( record );
        }
    }



    void TableManager::initRocktabTables(const Deck& deck) {
        if (!deck.hasKeyword("ROCKTAB"))
            return; // ROCKTAB is not featured by the deck...

        if (deck.numKeywords("ROCKTAB") > 1) {
            complainAboutAmbiguousKeyword(deck, "ROCKTAB");
            return;
        }

        const auto rocktabKeyword = deck.getKeyword("ROCKTAB");

        bool isDirectional = deck.hasKeyword("RKTRMDIR");
        bool useStressOption = false;
        if (deck.hasKeyword("ROCKOPTS")) {
            const auto rockoptsKeyword = deck.getKeyword("ROCKOPTS");
            useStressOption = (rockoptsKeyword->getRecord(0)->getItem("METHOD")->getTrimmedString(0) == "STRESS");
        }

        for (size_t tableIdx = 0; tableIdx < rocktabKeyword->size(); ++tableIdx) {
            if (rocktabKeyword->getRecord(tableIdx)->getItem(0)->size() == 0) {
                // for ROCKTAB tables, an empty record indicates that the previous table
                // should be copied...
                if (tableIdx == 0)
                    throw std::invalid_argument("The first table for keyword ROCKTAB"
                                                " must be explicitly defined!");
                m_rocktabTables[tableIdx] = m_rocktabTables[tableIdx - 1];
                continue;
            }

            m_rocktabTables.push_back(RocktabTable());
            m_rocktabTables[tableIdx].init(rocktabKeyword->getRecord( tableIdx )->getItem(0),
                                           isDirectional,
                                           useStressOption);
        }
    }



    void TableManager::initVFPProdTables(const Deck& deck,
                                          std::map<int, VFPProdTable>& tableMap) {
        if (!deck.hasKeyword(ParserKeywords::VFPPROD::keywordName)) {
            return;
        }

        int num_tables = deck.numKeywords(ParserKeywords::VFPPROD::keywordName);
        const auto& keywords = deck.getKeywordList<ParserKeywords::VFPPROD>();
        const auto unit_system = deck.getActiveUnitSystem();
        for (int i=0; i<num_tables; ++i) {
            const auto& keyword = keywords[i];

            VFPProdTable table;
            table.init(keyword, unit_system);

            //Check that the table in question has a unique ID
            int table_id = table.getTableNum();
            if (tableMap.find(table_id) == tableMap.end()) {
                tableMap.insert(std::make_pair(table_id, std::move(table)));
            }
            else {
                throw std::invalid_argument("Duplicate table numbers for VFPPROD found");
            }
        }
    }

    void TableManager::initVFPInjTables(const Deck& deck,
                                        std::map<int, VFPInjTable>& tableMap) {
        if (!deck.hasKeyword(ParserKeywords::VFPINJ::keywordName)) {
            return;
        }

        int num_tables = deck.numKeywords(ParserKeywords::VFPINJ::keywordName);
        const auto& keywords = deck.getKeywordList<ParserKeywords::VFPINJ>();
        const auto unit_system = deck.getActiveUnitSystem();
        for (int i=0; i<num_tables; ++i) {
            const auto& keyword = keywords[i];

            VFPInjTable table;
            table.init(keyword, unit_system);

            //Check that the table in question has a unique ID
            int table_id = table.getTableNum();
            if (tableMap.find(table_id) == tableMap.end()) {
                tableMap.insert(std::make_pair(table_id, std::move(table)));
            }
            else {
                throw std::invalid_argument("Duplicate table numbers for VFPINJ found");
            }
        }
    }

    std::shared_ptr<const Tabdims> TableManager::getTabdims() const {
        return m_tabdims;
    }


    /*
      const std::vector<SwofTable>& TableManager::getSwofTables() const {
        return m_swofTables;
        }
    */

    const TableContainer& TableManager::getSwofTables() const {
        return getTables("SWOF");
    }

    const TableContainer& TableManager::getSlgofTables() const {
        return getTables("SLGOF");
    }


    const TableContainer& TableManager::getSgofTables() const {
        return getTables("SGOF");
    }

    const TableContainer& TableManager::getSof2Tables() const {
        return getTables("SOF2");
    }

    const TableContainer& TableManager::getSof3Tables() const {
        return getTables("SOF3");
    }

    const TableContainer& TableManager::getSwfnTables() const {
        return getTables("SWFN");
    }

    const TableContainer& TableManager::getSgfnTables() const {
        return getTables("SGFN");
    }

    const TableContainer& TableManager::getSsfnTables() const {
        return getTables("SSFN");
    }

    const TableContainer& TableManager::getRsvdTables() const {
        return getTables("RSVD");
    }

    const TableContainer& TableManager::getRvvdTables() const {
        return getTables("RVVD");
    }

    const TableContainer& TableManager::getEnkrvdTables() const {
        return getTables("ENKRVD");
    }

    const TableContainer& TableManager::getEnptvdTables() const {
        return getTables("ENPTVD");
    }


    const TableContainer& TableManager::getImkrvdTables() const {
        return getTables("IMKRVD");
    }

    const TableContainer& TableManager::getImptvdTables() const {
        return getTables("IMPTVD");
    }

    const TableContainer& TableManager::getPvdgTables() const {
        return getTables("PVDG");
    }

    const TableContainer& TableManager::getPvdoTables() const {
        return getTables("PVDO");
    }

    const TableContainer& TableManager::getPvdsTables() const {
        return getTables("PVDS");
    }

    const std::vector<OilvisctTable>& TableManager::getOilvisctTables() const {
        return m_oilvisctTables;
    }

    const std::vector<WatvisctTable>& TableManager::getWatvisctTables() const {
        return m_watvisctTables;
    }

    const std::vector<GasvisctTable>& TableManager::getGasvisctTables() const {
        return m_gasvisctTables;
    }

    const std::vector<PlyadsTable>& TableManager::getPlyadsTables() const {
        return m_plyadsTables;
    }

    const std::vector<PlymaxTable>& TableManager::getPlymaxTables() const {
        return m_plymaxTables;
    }

    const std::vector<PlyrockTable>& TableManager::getPlyrockTables() const {
        return m_plyrockTables;
    }

    const std::vector<PlyviscTable>& TableManager::getPlyviscTables() const {
        return m_plyviscTables;
    }

    const std::vector<PlydhflfTable>& TableManager::getPlydhflfTables() const {
        return m_plydhflfTables;
    }

    const std::vector<PlyshlogTable>& TableManager::getPlyshlogTables() const {
        return m_plyshlogTables;
    }

    const std::vector<RocktabTable>& TableManager::getRocktabTables() const {
        return m_rocktabTables;
    }

    const std::vector<RtempvdTable>& TableManager::getRtempvdTables() const {
        return m_rtempvdTables;
    }


    const std::vector<PvtgTable>& TableManager::getPvtgTables() const {
        return m_pvtgTables;
    }


    const std::vector<PvtoTable>& TableManager::getPvtoTables() const {
        return m_pvtoTables;
    }


    const std::map<int, VFPProdTable>& TableManager::getVFPProdTables() const {
        return m_vfpprodTables;
    }

    const std::map<int, VFPInjTable>& TableManager::getVFPInjTables() const {
        return m_vfpinjTables;
    }


    void TableManager::complainAboutAmbiguousKeyword(const Deck& deck, const std::string& keywordName) const {
        OpmLog::addMessage(Log::MessageType::Error, "The " + keywordName + " keyword must be unique in the deck. Ignoring all!");
        auto keywords = deck.getKeywordList(keywordName);
        for (size_t i = 0; i < keywords.size(); ++i) {
            std::string msg = "Ambiguous keyword "+keywordName+" defined here";
            OpmLog::addMessage(Log::MessageType::Error , Log::fileMessage( keywords[i]->getFileName(), keywords[i]->getLineNumber(),msg));
        }
    }
}


