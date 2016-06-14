import unittest
import mantid
from mantid.api import AlgorithmManager
from Common.SANSLogTagger import SANSLogTagger


class SANSLogTaggerTest(unittest.TestCase):
    @staticmethod
    def _provide_sample_workspace():
        alg = AlgorithmManager.createUnmanaged("CreateSampleWorkspace")
        alg.setChild(True)
        alg.initialize()
        alg.setProperty("OutputWorkspace", "dummy")
        alg.execute()
        return alg.getProperty("OutputWorkspace").value

    def test_that_can_read_and_write_tag_in_sample_logs(self):
        # Arrange
        tagger = SANSLogTagger()
        ws1 = SANSLogTaggerTest._provide_sample_workspace()
        tag1 = "test"
        value1 = 123

        # Act + Assert
        self.assertFalse(tagger.has_tag(tag1, ws1))
        tagger.set_tag(tag1, value1, ws1)
        self.assertTrue(tagger.has_tag(tag1, ws1))
        self.assertTrue(tagger.get_tag(tag1, ws1) == value1)

    def test_that_can_read_and_write_hash_in_sample_log(self):
        # Arrange
        tagger = SANSLogTagger()
        ws1 = self._provide_sample_workspace()
        tag1 = "test"
        value1 = "tested"
        value2 = "tested2"

        # Act + Assert
        self.assertFalse(tagger.has_hash(tag1, value1, ws1))
        tagger.set_hash(tag1, value1, ws1)
        self.assertTrue(tagger.has_hash(tag1, value1, ws1))
        self.assertFalse(tagger.has_hash(tag1, value2, ws1))


if __name__ == '__main__':
    unittest.main()
