from unittest import TestCase
import os.path
from ert.test import TestAreaContext

from opm.parser import Parser
from opm.ecl_state.grid import EclipseGrid
from opm.ecl_state.grid import Eclipse3DProps
from opm.ecl_state.tables import TableManager
from opm_test import OpmTest

class Eclipse3DPropsTest(OpmTest):
    def setUp(self):
        pass

    def test_parse(self):
        p = Parser()
        test_file = self.createPath( "integration_tests/GRID/CORNERPOINT_ACTNUM.DATA" )
        deck = p.parseFile( test_file )
        grid = EclipseGrid( deck )
        tables = TableManager( deck )
        ecl_props = Eclipse3DProps( deck , tables , grid )
