import importlib.util
import struct
import tempfile
import unittest
import zipfile
from pathlib import Path
from unittest.mock import patch

SPEC = importlib.util.spec_from_file_location("publisher", Path(__file__).parents[1] / "scripts/publish_firmware.py")
p = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(p)


def run():
    return {"head_repository": {"full_name": p.REPO}, "status": "completed", "conclusion": "success",
            "event": "push", "path": ".github/workflows/build.yml", "head_branch": "dya-studio",
            "id": 123, "run_attempt": 1, "head_sha": "a" * 40,
            "created_at": "2026-11-01T12:00:00Z", "html_url": p.URL + "/actions/runs/123"}


def uf2(address=0x27000, family=0xADA52840, count=2):
    blocks = []
    for number in range(count):
        header = struct.pack("<8I", 0x0A324655, 0x9E5D5157, 0x2000, address + number * 256,
                             256, number, count, family)
        blocks.append(header + bytes(476) + struct.pack("<I", 0x0AB16F30))
    return b"".join(blocks)


class FirmwareReleaseTests(unittest.TestCase):
    def test_rejects_failed_pr_foreign_repository_and_draw_run(self):
        p.validate_run(run(), "dya-studio")
        for change in ({"conclusion": "failure"}, {"status": "in_progress"}, {"event": "pull_request"},
                       {"path": ".github/workflows/draw.yml"}, {"head_repository": {"full_name": "someone/fork"}}):
            with self.subTest(change=change), self.assertRaises(ValueError):
                p.validate_run({**run(), **change}, "dya-studio")

    def test_uf2_rejects_truncation_wrong_mcu_and_non_application_addresses(self):
        with tempfile.TemporaryDirectory() as tmp:
            path = Path(tmp) / "firmware.uf2"
            path.write_bytes(uf2())
            p.validate_uf2(path)
            for invalid in (b"", uf2()[:-1], uf2(family=1), uf2(address=0), uf2(address=0xEC000), uf2(address=0xF4000)):
                with self.subTest(size=len(invalid)), self.assertRaises(ValueError):
                    path.write_bytes(invalid)
                    p.validate_uf2(path)

    def test_uf2_rejects_missing_or_duplicate_blocks(self):
        with tempfile.TemporaryDirectory() as tmp:
            path = Path(tmp) / "firmware.uf2"
            for invalid in (uf2()[:512], uf2()[:512] * 2):
                path.write_bytes(invalid)
                with self.assertRaises(ValueError):
                    p.validate_uf2(path)

    def test_zip_is_reproducible_and_does_not_include_reset(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            source = root / "input"
            source.mkdir()
            for name in p.filenames("dya-studio"):
                (source / name).write_bytes(uf2())
            for dest in ("first", "second"):
                _, archive = p.package(source, root / dest, "dya-studio", run())
            self.assertEqual((root / "first" / archive).read_bytes(), (root / "second" / archive).read_bytes())
            with zipfile.ZipFile(root / "first" / archive) as z:
                self.assertEqual(z.namelist(), p.filenames("dya-studio")[:2])
            self.assertTrue((root / "first" / p.filenames("dya-studio")[2]).exists())

    def test_wrong_branch_or_incomplete_artifact_is_rejected(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            source = root / "input"
            source.mkdir()
            for name in p.filenames("main"):
                (source / name).write_bytes(uf2())
            with self.assertRaises(ValueError):
                p.package(source, root / "output", "dya-studio", run())
            (source / p.filenames("main")[1]).unlink()
            with self.assertRaises(ValueError):
                p.package(source, root / "output", "main", run())

    def test_source_comparison_ignores_docs_but_checks_all_firmware_files(self):
        tree = {"tree": [{"path": path, "type": "blob", "sha": "old", "mode": "100644"}
                         for path in ("README.md", "keymap-drawer/corcell.svg", "config/west.yml", "new-driver.c")]}
        self.assertEqual(set(p.firmware_tree(tree)), {"config/west.yml", "new-driver.c"})
        with self.assertRaises(ValueError):
            p.firmware_tree({**tree, "truncated": True})

    def test_newer_firmware_skips_readme_and_release_updates(self):
        def tree(sha):
            return {"sha": sha, "tree": [{"path": "config/corcell.keymap", "type": "blob", "sha": sha, "mode": "100644"}]}
        replies = [{"object": {"sha": "new-head"}}, {"merge_base_commit": {"sha": run()["head_sha"]}}, tree("old"), tree("new")]
        with patch.object(p, "api", side_effect=replies) as request:
            self.assertIsNone(p.branch_state(run(), "dya-studio"))
        self.assertEqual(request.call_count, 4)

    def test_readme_updates_only_marked_block_without_forcing_branch(self):
        readme = f"# Existing title\n\n{p.START}\nold links\n{p.END}\n\nUser's other edits\n"
        state = ("current-head", "current-tree", readme)
        with patch.object(p, "branch_state", return_value=state), patch.object(p, "api", side_effect=[{"sha": "blob"}, {"sha": "tree"}, {"sha": "commit"}, {}]) as request:
            p.update_readme(run(), "dya-studio", "firmware-dya-studio-123-1")
        updated = request.call_args_list[0].args[1]["content"]
        self.assertTrue(updated.startswith("# Existing title\n\n"))
        self.assertTrue(updated.endswith("User's other edits\n"))
        self.assertIn("firmware-dya-studio-123-1", updated)
        self.assertEqual(request.call_args_list[2].args[1]["parents"], ["current-head"])
        self.assertEqual(request.call_args_list[3].args[1], {"sha": "commit", "force": False})

    def test_existing_readme_does_not_create_another_commit(self):
        readme = p.download_block("main", run(), "tag")
        with patch.object(p, "branch_state", return_value=("head", "tree", readme)), patch.object(p, "api") as request:
            p.update_readme(run(), "main", "tag")
            request.assert_not_called()


if __name__ == "__main__":
    unittest.main()
