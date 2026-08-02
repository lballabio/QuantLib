"""Phase-3 smoke tests (packaging helpers / docs references)."""

from __future__ import annotations

import runpy
import sys
from pathlib import Path

import pytest

import qlnb as ql

ROOT = Path(__file__).resolve().parents[1]
SCRIPTS = ROOT / "scripts"
BENCH = ROOT / "benchmarks"


def test_version_phase3():
    parts = tuple(int(x) for x in ql.__version__.split(".")[:2])
    assert parts >= (0, 3)


def test_free_threading_doc_exists():
    doc = ROOT / "docs" / "free-threading.md"
    assert doc.is_file()
    text = doc.read_text(encoding="utf-8")
    assert "QL_ENABLE_THREAD_SAFE_OBSERVER_PATTERN" in text
    assert "QLNB_THREAD_SAFE_OBSERVER" in text


def test_migration_doc_exists():
    migration = ROOT / "docs" / "migration.md"
    packaging = ROOT / "docs" / "packaging.md"
    assert migration.is_file()
    assert packaging.is_file()
    text = migration.read_text(encoding="utf-8")
    assert "import qlnb as ql" in text
    assert "FlatForward" in text
    assert "simulate_gbm_paths" in text


def test_check_npv_drift_script(tmp_path):
    script = SCRIPTS / "check_npv_drift.py"
    assert script.is_file()
    json_out = tmp_path / "npv_drift.json"
    # Run as a module-like script via runpy for a fast in-process check.
    sys_argv = list(sys.argv)
    try:
        sys.argv = [str(script), "--abs-tol", "1e-8", "--json-out", str(json_out)]
        runpy.run_path(str(script), run_name="__main__")
    except SystemExit as exc:
        assert exc.code in (0, None)
    finally:
        sys.argv = sys_argv
    assert json_out.is_file()
    payload = json_out.read_text(encoding="utf-8")
    assert "qlnb_npv" in payload
    assert '"ok": true' in payload


def test_bench_script_importable():
    bench = BENCH / "bench_phase0.py"
    assert bench.is_file()
    ns = runpy.run_path(str(bench), run_name="not_main")
    assert callable(ns.get("bench_qlnb"))
    assert callable(ns.get("_price_qlnb_european_put"))
