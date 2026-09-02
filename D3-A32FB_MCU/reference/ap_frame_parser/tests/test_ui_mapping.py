import pathlib
import sys
import unittest


TOOL_ROOT = pathlib.Path(__file__).resolve().parents[1]
sys.path.insert(0, str(TOOL_ROOT))

from ui.main_window import byte_range_to_text_indices


class UiMappingTest(unittest.TestCase):
    def test_second_line_range(self):
        self.assertEqual(("2.0", "2.5"), byte_range_to_text_indices(16, 2, 16))


if __name__ == "__main__":
    unittest.main()
