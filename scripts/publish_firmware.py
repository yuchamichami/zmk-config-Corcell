#!/usr/bin/env python3
"""Publish a successful Corcell build and atomically update its README links."""

import argparse
import base64
import hashlib
import json
import re
import struct
import subprocess
import tempfile
import zipfile
from pathlib import Path

REPO = "yuchamichami/zmk-config-Corcell"
URL = f"https://github.com/{REPO}"
START = "<!-- firmware-downloads:start -->"
END = "<!-- firmware-downloads:end -->"
BRANCHES = {"main": "通常版", "dya-studio": "DYA Studio版"}


def gh(*args):
    return subprocess.check_output(["gh", *args], text=True).strip()


def api(path, payload=None, method="GET"):
    args = ["gh", "api", f"repos/{REPO}/{path}", "--method", method]
    if payload is not None:
        args += ["--input", "-"]
    result = subprocess.run(args, input=json.dumps(payload) if payload is not None else None,
                            text=True, capture_output=True, check=True)
    return json.loads(result.stdout)


def validate_run(run, branch):
    if branch not in BRANCHES or run["head_repository"]["full_name"] != REPO:
        raise ValueError("Unexpected repository or destination branch")
    if (run["status"], run["conclusion"]) != ("completed", "success"):
        raise ValueError("The firmware build has not succeeded")
    if run["event"] not in ("push", "workflow_dispatch") or run["path"] != ".github/workflows/build.yml":
        raise ValueError("Only the firmware workflow's push/manual builds can be published")


def firmware_path(path):
    # These files do not affect the UF2 build. Everything else must match.
    return not (path.endswith(".md") or path.startswith(("docs/", "keymap-drawer/", ".github/", "scripts/", "tests/"))
                or path in ("LICENSE", ".gitignore"))


def firmware_tree(tree):
    if tree.get("truncated"):
        raise ValueError("Cannot verify a truncated source tree")
    return {entry["path"]: (entry["sha"], entry["mode"]) for entry in tree["tree"]
            if entry["type"] != "tree" and firmware_path(entry["path"])}


def branch_state(run, branch):
    head = api(f"git/ref/heads/{branch}")["object"]["sha"]
    comparison = api(f"compare/{run['head_sha']}...{head}")
    if comparison["merge_base_commit"]["sha"] != run["head_sha"]:
        raise ValueError("Build commit is not an ancestor of the destination branch")
    source = api(f"git/trees/{run['head_sha']}?recursive=1")
    current = api(f"git/trees/{head}?recursive=1")
    if firmware_tree(source) != firmware_tree(current):
        print("Skipped: a newer firmware change is already on the branch")
        return None
    content = api(f"contents/README.md?ref={head}")
    readme = base64.b64decode(content["content"]).decode()
    if readme.count(START) != 1 or readme.count(END) != 1:
        raise ValueError("README download markers are missing or duplicated")
    old = re.search(r"<!-- firmware-build: (\d+) (\d+) -->", readme)
    if old and tuple(map(int, old.groups())) > (run["id"], run["run_attempt"]):
        print("Skipped: README already links a newer build")
        return None
    return head, current["sha"], readme


def filenames(branch):
    suffix = "-dya-studio" if branch == "dya-studio" else ""
    return [f"{prefix}{suffix}-xiao_ble_zmk.uf2" for prefix in ("Corcell_L", "Corcell_R", "settings_reset")]


def validate_uf2(path):
    data = path.read_bytes()
    if not data or len(data) % 512:
        raise ValueError(f"Invalid UF2 size: {path.name}")
    seen = set()
    addresses = set()
    count = len(data) // 512
    for offset in range(0, len(data), 512):
        block = data[offset:offset + 512]
        m0, m1, flags, address, size, number, total, family = struct.unpack_from("<8I", block)
        if (m0, m1, struct.unpack_from("<I", block, 508)[0]) != (0x0A324655, 0x9E5D5157, 0x0AB16F30):
            raise ValueError(f"Invalid UF2 magic: {path.name}")
        # XIAO BLE application partition: 0x27000..0xec000, excluding settings/bootloader.
        if flags != 0x2000 or family != 0xADA52840 or size != 256 or address % 256 or not 0x27000 <= address < 0xEC000:
            raise ValueError(f"Unexpected UF2 target: {path.name}")
        if total != count or number >= count or number in seen or address in addresses:
            raise ValueError(f"Incomplete/duplicate UF2 blocks: {path.name}")
        seen.add(number)
        addresses.add(address)
    if min(addresses) != 0x27000 or max(addresses) - min(addresses) + 256 != count * 256:
        raise ValueError(f"Unexpected application range: {path.name}")


def package(source, output, branch, run):
    names = filenames(branch)
    files = sorted(p.name for p in source.rglob("*") if p.is_file())
    if files != sorted(names) or any(not (source / name).is_file() for name in names):
        raise ValueError("Artifact must contain exactly this branch's left, right and reset UF2s")
    output.mkdir(parents=True, exist_ok=True)
    for name in names:
        validate_uf2(source / name)
        (output / name).write_bytes((source / name).read_bytes())
    archive = output / f"Corcell-{branch}-LR.zip"
    with zipfile.ZipFile(archive, "w", zipfile.ZIP_DEFLATED) as z:
        for name in names[:2]:  # Reset firmware must not be mistaken for an ordinary update.
            info = zipfile.ZipInfo(name, (1980, 1, 1, 0, 0, 0))
            info.compress_type = zipfile.ZIP_DEFLATED
            z.writestr(info, (source / name).read_bytes())
    metadata = {"branch": branch, "source_branch": run["head_branch"], "commit": run["head_sha"],
                "build_url": run["html_url"], "run_id": run["id"], "run_attempt": run["run_attempt"],
                "built_at": run["created_at"], "hardware_test": "not established by this automated release"}
    (output / "firmware.json").write_text(json.dumps(metadata, indent=2) + "\n")
    sums = {p.name: hashlib.sha256(p.read_bytes()).hexdigest() for p in sorted(output.iterdir()) if p.is_file()}
    (output / "SHA256SUMS.txt").write_text("".join(f"{value}  {name}\n" for name, value in sums.items()))
    return names, archive.name


def download_block(branch, run, tag):
    left, right, reset = filenames(branch)
    base = f"{URL}/releases/download/{tag}"
    return f"""{START}
<!-- firmware-build: {run['id']} {run['run_attempt']} -->
[左右まとめてダウンロード（ZIP）]({base}/Corcell-{branch}-LR.zip)

| 個別ダウンロード | 用途 |
| --- | --- |
| [左手用（L）]({base}/{left}) | 左に書き込むUF2 |
| [右手用（R）]({base}/{right}) | 右に書き込むUF2 |
| [設定リセット用]({base}/{reset}) | ペアリング・保存した設定をすべて消す場合のみ |

配布元：{run['created_at'][:10]} / `{run['head_sha'][:7]}` · [配布ファイルの詳細]({URL}/releases/tag/{tag})
{END}"""


def publish(run, branch, output, tag):
    result = subprocess.run(["gh", "release", "view", tag, "-R", REPO, "--json", "isDraft,targetCommitish"],
                            text=True, capture_output=True)
    if result.returncode:
        if "release not found" not in result.stderr.lower():
            raise RuntimeError(result.stderr)
        notes = output.parent / "release-notes.md"
        notes.write_text(f"Corcell {BRANCHES[branch]}のファームです。\n\n"
                         f"左右用ZIPにはL/Rだけが入っています。通常の更新に設定リセットは不要です。\n\n"
                         f"対象：右トラックボールのCorcell / XIAO BLE。\n"
                         f"ソース：`{branch}` / `{run['head_sha']}`\n\n"
                         f"[元のビルド]({run['html_url']}) · [更新手順]({URL}/blob/{branch}/README.md#ファームを更新する)\n\n"
                         "ビルド成功とUF2形式を確認した配布物です。自動公開は実機での動作確認を意味しません。\n")
        gh("release", "create", tag, "-R", REPO, "--draft", "--target", run["head_sha"],
           "--title", f"Corcell {BRANCHES[branch]} {run['created_at'][:10]} / {run['head_sha'][:7]}",
           "--notes-file", str(notes), "--latest=false")
        draft = True
    else:
        release = json.loads(result.stdout)
        if release["targetCommitish"] != run["head_sha"]:
            raise ValueError("Existing release points to a different source commit")
        draft = release["isDraft"]
    release = json.loads(gh("release", "view", tag, "-R", REPO, "--json", "assets"))
    existing = {asset["name"]: asset for asset in release["assets"]}
    expected = {p.name: p for p in output.iterdir() if p.is_file()}
    if set(existing) - set(expected):
        raise ValueError("Unexpected assets in existing release")
    for name, path in expected.items():
        if name not in existing:
            if not draft:
                raise ValueError("Published release has missing assets")
            gh("release", "upload", tag, str(path), "-R", REPO)
    verify = output.parent / "verify"
    verify.mkdir()
    gh("release", "download", tag, "-R", REPO, "-D", str(verify))
    if {p.name: p.read_bytes() for p in verify.iterdir()} != {name: p.read_bytes() for name, p in expected.items()}:
        raise ValueError("Uploaded asset verification failed")
    if draft:
        gh("release", "edit", tag, "-R", REPO, "--draft=false", "--latest=false")


def update_readme(run, branch, tag):
    state = branch_state(run, branch)
    if state is None:
        return
    head, tree, readme = state
    before, remainder = readme.split(START)
    _, after = remainder.split(END)
    updated = before + download_block(branch, run, tag) + after
    if updated == readme:
        print("README already points to this build")
        return
    blob = api("git/blobs", {"content": updated, "encoding": "utf-8"}, "POST")
    new_tree = api("git/trees", {"base_tree": tree, "tree": [{"path": "README.md", "mode": "100644", "type": "blob", "sha": blob["sha"]}]}, "POST")
    commit = api("git/commits", {"message": f"docs: link {branch} firmware build {run['id']} [skip ci]", "tree": new_tree["sha"], "parents": [head]}, "POST")
    # Non-forced update refuses any concurrent branch edit, including firmware changes.
    api(f"git/refs/heads/{branch}", {"sha": commit["sha"], "force": False}, "PATCH")
    print(f"Updated {branch} README: {commit['sha']}")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--run-id", type=int, required=True)
    parser.add_argument("--branch", choices=BRANCHES, required=True)
    parser.add_argument("--work-dir", type=Path, required=True)
    parser.add_argument("--prepare-only", action="store_true", help="Download and validate without publishing")
    args = parser.parse_args()
    run = api(f"actions/runs/{args.run_id}")
    validate_run(run, args.branch)
    if branch_state(run, args.branch) is None:
        return
    args.work_dir.mkdir(parents=True, exist_ok=True)
    with tempfile.TemporaryDirectory(dir=args.work_dir) as work:
        work = Path(work)
        source = work / "input"
        gh("run", "download", str(args.run_id), "-R", REPO, "-n", "firmware", "-D", str(source))
        package(source, work / "release", args.branch, run)
        current_run = api(f"actions/runs/{args.run_id}")
        validate_run(current_run, args.branch)
        if current_run["run_attempt"] != run["run_attempt"]:
            raise ValueError("Build was rerun while its artifacts were being downloaded")
        tag = f"firmware-{args.branch}-{run['id']}-{run['run_attempt']}"
        if args.prepare_only:
            print(f"Validated {tag}; no GitHub changes made")
            return
        if branch_state(run, args.branch) is None:
            return
        publish(run, args.branch, work / "release", tag)
        update_readme(run, args.branch, tag)
        print(f"{URL}/releases/tag/{tag}")


if __name__ == "__main__":
    main()
