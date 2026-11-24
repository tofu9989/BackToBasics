import json
from pathlib import Path
import subprocess
import tempfile
import unittest


ROOT = Path(__file__).resolve().parents[1]
EXE = ROOT / "json_cal.exe"


class CalculatorTests(unittest.TestCase):
    def run_calculator(self, *args):
        return subprocess.run(
            [str(EXE), *args], cwd=ROOT, capture_output=True, text=True, timeout=5
        )

    def run_input(self, data):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "input.json"
            path.write_text(data, encoding="utf-8")
            return self.run_calculator(str(path))

    def test_default_file(self):
        run = self.run_calculator()
        self.assertEqual(run.returncode, 0, run.stderr)
        expected = json.loads((ROOT / "data.json").read_text(encoding="utf-8"))
        expected["result"] = 376.5
        self.assertEqual(json.loads(run.stdout), expected)

    def test_operators(self):
        for op, expected in [("+", 10), ("-", 6), ("*", 16), ("/", 4)]:
            with self.subTest(op=op):
                data = {"cal": {"op": op, "a": 8, "b": 2}, "result": -1}
                run = self.run_input(json.dumps(data))
                self.assertEqual(run.returncode, 0, run.stderr)
                data["result"] = expected
                self.assertEqual(json.loads(run.stdout), data)

    def test_invalid_input(self):
        for data, error in [
            ('{"cal":{"op":"/","a":8,"b":0}}', "cannot divide by zero"),
            ('{"cal":{"op":"*","a":"8","b":2}}', "finite numbers"),
            ('{"cal":{"op":"%","a":8,"b":2}}', "supported operators"),
            ('{}', "cal must be a JSON object"),
            ('{} trailing', "Expected a JSON object"),
        ]:
            with self.subTest(data=data):
                run = self.run_input(data)
                self.assertEqual(run.returncode, 1, run.stderr)
                self.assertEqual(run.stdout, "")
                self.assertIn(error, run.stderr)


if __name__ == "__main__":
    unittest.main(verbosity=2)
