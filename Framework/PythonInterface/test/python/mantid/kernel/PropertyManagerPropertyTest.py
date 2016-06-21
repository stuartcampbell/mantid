﻿"""Test the exposed PropertyManagerProperty
"""
import unittest
from mantid.kernel import PropertyManagerProperty, Direction, PropertyManager
from mantid.api import Algorithm
import numpy as np

class PropertyManagerPropertyTest(unittest.TestCase):

    def test_default_constructor_raises_an_exception(self):
        self.assertRaises(Exception, PropertyManagerProperty)

    def test_name_only_constructor_gives_correct_object(self):
        name = "Args"
        pmap = PropertyManagerProperty(name)
        self.assertTrue(isinstance(pmap, PropertyManagerProperty))
        self._check_object_attributes(pmap, name, Direction.Input)

    def test_name_direction_constructor_gives_correct_object(self):
        name = "Args"
        direc = Direction.Output
        arr = PropertyManagerProperty(name, direc)
        self._check_object_attributes(arr, name, direc)

    def test_set_property_on_algorithm_from_dictionary(self):
        class FakeAlgorithm(Algorithm):
            def PyInit(self):
                self.declareProperty(PropertyManagerProperty("Args"))

            def PyExec(self):
                pass
        #
        fake = FakeAlgorithm()
        fake.initialize()
        fake.setProperty("Args", {'A': 1, 
                                  'B':10.5, 
                                  'C':'String arg', 
                                  'D': [0.0,11.3], 
                                  'E':{'F':10.4, 
                                       'G': [1.0,2.0, 3.0],
                                       'H':{'I': "test",
                                       'J': 120.6}}})

        pmgr = fake.getProperty("Args").value
        self.assertTrue(isinstance(pmgr, PropertyManager))
        self.assertEqual(4, len(pmgr))
        self.assertTrue('A' in pmgr)
        self.assertEqual(1, pmgr['A'].value)
        self.assertTrue('B' in pmgr)
        self.assertEqual(10.5, pmgr['B'].value)
        self.assertTrue('C' in pmgr)
        self.assertEqual('String arg', pmgr['C'].value)
        self.assertTrue('D' in pmgr)
        array_value = pmgr['D'].value
        self.assertEqual(0.0,array_value[0])
        self.assertEqual(11.3,array_value[1])

        # Check the level-1 nested property manager property
        # Get the level1-nested property manager 
        nested_l1_pmgr = pmgr['E'].value
        evaluate_pmgr(nested_l1_pmgr, 'F', 10.4)
        evaluate_pmgr(nested_l1_pmgr, 'G', [1.0,2.0, 3.0])

        # Get the level2-nested property manager 
        nested_l2_pmgr = nested_l1_pmgr['H'].value
        evaluate_pmgr(nested_l2_pmgr, 'I', "test")
        evaluate_pmgr(nested_l2_pmgr, 'J', 120.6)

    def _check_object_attributes(self, prop, name, direction):
        self.assertEquals(prop.name, name)
        self.assertEquals(prop.direction, direction)

if __name__ == "__main__":
    unittest.main()
