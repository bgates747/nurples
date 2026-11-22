#!/usr/bin/python3
"""Oryx desktop launcher: record the profile-local FAE window and only its audio."""
import argparse
from contextlib import contextmanager
import datetime
import fcntl
import json
import os
from pathlib import Path
import signal
import subprocess as sp
import sys
import tempfile
import time


def output(*args):
    return sp.check_output(args, text=True).strip()


def group_stop(p):
    if p.poll() is None:
        os.killpg(p.pid, signal.SIGTERM)
        try:
            p.wait(timeout=4)
        except sp.TimeoutExpired:
            pass
    # On setup failure Bash may exit before SDL handles TERM. Only reap
    # remaining live members of the private session/group we created.
    def remaining():
        rows = sp.run(["ps", "--no-headers", "--sid", str(p.pid), "-o", "pid=,stat="],
                      capture_output=True, text=True).stdout.splitlines()
        return [int(pid) for pid, state in (r.split() for r in rows) if not state.startswith("Z")]

    for _ in range(20):
        if not remaining():
            break
        time.sleep(.1)
    for pid in remaining():
        try:
            fd = os.pidfd_open(pid)
            try:
                if os.getsid(pid) == os.getpgid(pid) == p.pid:
                    signal.pidfd_send_signal(fd, signal.SIGKILL)
            finally:
                os.close(fd)
        except ProcessLookupError:
            pass
    p.wait(timeout=5)
    for _ in range(30):
        if not remaining():
            return
        time.sleep(.1)
    raise RuntimeError("An owned capture-session process has not stopped")


@contextmanager
def game_audio(quiet):
    """Give SDL a private sink. In normal play it forwards to the actual speakers."""
    original = output("pactl", "get-default-sink")
    name = f"nurples_fae_{os.getpid()}"
    args = (["module-null-sink"] if quiet else ["module-remap-sink", f"master={original}"])
    module = output("pactl", "load-module", *args, f"sink_name={name}",
                    "sink_properties=device.class=filter")
    if not module.isdecimal():
        raise RuntimeError(f"Unexpected PulseAudio module ID: {module!r}")
    try:
        if output("pactl", "get-default-sink") == name:
            sp.run(["pactl", "set-default-sink", original], check=True)
        if not any(line.split()[1] == name + ".monitor" for line in
                   output("pactl", "list", "short", "sources").splitlines()):
            raise RuntimeError("Game audio monitor missing")
        yield name
    finally:
        sp.run(["pactl", "unload-module", module], check=True)


def find_window(game, smoke):
    deadline = time.monotonic() + 20
    last = None
    since = time.monotonic()
    while game.poll() is None and time.monotonic() < deadline:
        # A freshly started WM may not publish _NET_CLIENT_LIST until its
        # first client maps. Treat that as "not yet", not as capture failure.
        clients = sp.run(["wmctrl", "-lpG"], capture_output=True, text=True)
        candidate = None
        for line in clients.stdout.splitlines():
            wid, _, pid, _, _, width, height, _, *title = line.split()
            try:
                if (int(pid) > 0 and os.getpgid(int(pid)) == game.pid
                        and int(width) >= 256 and int(height) >= 192
                        and (smoke or "fab agon emulator" in " ".join(title).lower())):
                    candidate = wid
                    break
            except ProcessLookupError:
                pass
        # SDL probes a temporary window during startup before replacing it.
        if candidate != last:
            last, since = candidate, time.monotonic()
        elif last and time.monotonic() - since >= 1:
            return str(int(last, 16))
        time.sleep(.1)
    raise RuntimeError("No X11 window belonging to this launch; refusing desktop capture")


def route_audio(game, sink):
    # On Oryx, FAE can explicitly open the speakers despite PULSE_SINK.
    # Move only streams whose PID belongs to our private process group.
    sink_id = next(int(line.split()[0]) for line in output(
        "pactl", "list", "short", "sinks").splitlines() if line.split()[1] == sink)
    found = False
    for stream in json.loads(output("pactl", "--format=json", "list", "sink-inputs")):
        pid = stream.get("properties", {}).get("application.process.id", "")
        try:
            if not str(pid).isdecimal() or os.getpgid(int(pid)) != game.pid:
                continue
        except ProcessLookupError:
            continue
        if int(stream["sink"]) != sink_id:
            sp.run(["pactl", "move-sink-input", str(stream["index"]), sink], check=True)
        found = True
    return found


LUA = """
local obs = obslua
local dir = os.getenv("NURPLES_OBS_CONTROL") .. "/"
local function mark(name)
    local f = assert(io.open(dir .. name, "w")); f:close()
end
local function event(e)
    if e == obs.OBS_FRONTEND_EVENT_RECORDING_STARTED then mark("recording") end
    if e == obs.OBS_FRONTEND_EVENT_RECORDING_STOPPED then mark("finalized") end
end
local function check()
    if obs.obs_frontend_recording_active() then
        local src = obs.obs_get_source_by_name("Game window")
        if src then
            if not ready and obs.obs_source_get_width(src) > 0 then
                mark("video-ready"); ready = true
            end
            obs.obs_source_release(src)
        end
    end
    local f = io.open(dir .. "stop", "r")
    if f then
        f:close()
        if obs.obs_frontend_recording_active() then
            obs.timer_remove(check)
            obs.obs_frontend_recording_stop()
        end
    end
end
function script_load(s) obs.obs_frontend_add_event_callback(event); obs.timer_add(check, 100) end
"""


def setup(config, folder, wid, sink):
    base = config / "obs-studio"
    profiles = base / "basic/profiles/Nurples"
    scenes = base / "basic/scenes"
    profiles.mkdir(parents=True)
    scenes.mkdir()
    (base / "user.ini").write_text("[General]\nFirstRun=true\n"
                                  "[Basic]\nProfile=Nurples\nProfileDir=Nurples\n"
                                  "SceneCollection=Nurples\nSceneCollectionFile=Nurples\n")
    (base / "global.ini").write_text("[General]\nPre19Defaults=false\n"
                                    "LastVersion=536936448\n[OBSWebSocket]\nServerEnabled=false\n")
    (profiles / "basic.ini").write_text(
        "[General]\nName=Nurples\n[Output]\nMode=Simple\nFilenameFormatting=Purple-Nurples\n"
        f"[SimpleOutput]\nFilePath={folder}\nRecFormat2=mkv\nRecQuality=Small\nRecEncoder=x264\n"
        "[Video]\nBaseCX=960\nBaseCY=720\nOutputCX=960\nOutputCY=720\nFPSType=0\nFPSCommon=60\n"
        "[Audio]\nSampleRate=48000\nChannelSetup=Stereo\n")
    lua = config / "finalize.lua"
    lua.write_text(LUA)
    # OBS's bundled FFmpeg input can read one X11 drawable directly; it
    # does not require EGL pixmap import. Never pass 0 (the root window).
    if int(wid) <= 0:
        raise RuntimeError("Invalid capture window")
    video = {"name": "Game window", "id": "ffmpeg_source",
             "settings": {"is_local_file": False, "input_format": "x11grab",
                          "input": os.environ["DISPLAY"], "buffering_mb": 0,
                          "ffmpeg_options": f"window_id={wid} draw_mouse=0 framerate=60"}}
    audio = {"name": "Game audio", "id": "pulse_output_capture",
             "settings": {"device_id": sink + ".monitor"}, "mixers": 1}
    scene = {"name": "Nurples", "id": "scene", "settings": {"items": [
        {"name": video["name"], "visible": True, "id": 1, "align": 5,
         "bounds_type": 2, "bounds": {"x": 960., "y": 720.}, "bounds_align": 0},
        {"name": audio["name"], "visible": True, "id": 2}]}}
    (scenes / "Nurples.json").write_text(json.dumps({
        "name": "Nurples", "current_scene": "Nurples", "current_program_scene": "Nurples",
        "scene_order": [{"name": "Nurples"}], "sources": [video, audio, scene],
        "modules": {"scripts-tool": [{"path": str(lua), "settings": {}}]}}))


def wait_marker(path, obs, seconds):
    deadline = time.monotonic() + seconds
    while time.monotonic() < deadline:
        if path.exists():
            return True
        if obs.poll() is not None:
            break
        time.sleep(.1)
    return False


def run(smoke, output_dir=None):
    if not os.environ.get("DISPLAY"):
        raise RuntimeError("Run from an X11 terminal on Oryx")
    profile = Path.cwd()
    if not smoke and (not (profile / "fab-agon-emulator").is_file()
                      or (profile / "fab-agon-emulator").is_symlink()):
        raise RuntimeError("Run from the staged FAE profile with its real launcher")
    media = output_dir.resolve() if output_dir else (
        profile if smoke else Path.home() / "Videos/Purple-Nurples")
    media.mkdir(parents=True, exist_ok=True)
    stamp = datetime.datetime.now().strftime("%Y-%m-%d_%H-%M-%S")
    folder = Path(tempfile.mkdtemp(prefix=stamp + "-", dir=media))
    cfg = folder / ".capture"
    print(f"Recording folder: {folder}", flush=True)
    with game_audio(smoke) as sink, (folder / "obs.log").open("xb") as log:
        cmd = (["ffplay", "-v", "error", "-autoexit", "-f", "lavfi", "-i",
                "testsrc2=size=640x480:rate=30:duration=9[out0];"
                "sine=frequency=440:duration=9[out1]"] if smoke else ["./fab-agon-emulator"])
        game = sp.Popen(cmd, start_new_session=True, env={
            **os.environ, "PULSE_SINK": sink, "SDL_AUDIODRIVER": "pulseaudio",
            "SDL_AUDIO_DRIVER": "pulseaudio", "SDL_VIDEODRIVER": "x11", "SDL_VIDEO_DRIVER": "x11"})
        try:
            wid = find_window(game, smoke)
            for _ in range(50):
                if route_audio(game, sink):
                    break
                time.sleep(.1)
            else:
                raise RuntimeError("No audio stream belonging to FAE; refusing a silent recording")
            setup(cfg, folder, wid, sink)
            obs = sp.Popen(["obs", "--multi", "--only-bundled-plugins", "--profile", "Nurples",
                            "--collection", "Nurples", "--scene", "Nurples", "--minimize-to-tray",
                            "--startrecording", "--disable-missing-files-check"],
                           stdout=log, stderr=sp.STDOUT, start_new_session=True,
                           env={**os.environ, "XDG_CONFIG_HOME": str(cfg),
                                "NURPLES_OBS_CONTROL": str(cfg), "QT_QPA_PLATFORM": "xcb"})
            try:
                if not wait_marker(cfg / "recording", obs, 20):
                    raise RuntimeError(f"OBS did not start recording; see {log.name}")
                if not wait_marker(cfg / "video-ready", obs, 10):
                    raise RuntimeError(f"OBS has not received a frame from FAE; see {log.name}")
                sp.run(["wmctrl", "-ia", hex(int(wid))], check=False)
                print("OBS is recording. Type RUN at MOS; close FAE to save.", flush=True)
                next_check = time.monotonic() + 3
                while game.poll() is None:
                    if obs.poll() is not None or (cfg / "finalized").exists():
                        raise RuntimeError("OBS stopped before the game closed")
                    if time.monotonic() >= next_check:
                        route_audio(game, sink)
                        next_check = time.monotonic() + 3
                    time.sleep(.25)
            finally:
                (cfg / "stop").touch()
                if obs.poll() is None:
                    flushed = wait_marker(cfg / "finalized", obs, 20)
                    if not flushed and (cfg / "recording").exists():
                        print("OBS has not confirmed finalization. Leaving it open; "
                              "use Stop Recording there.", file=sys.stderr)
                        raise RuntimeError("Recording finalization not confirmed")
                    obs.send_signal(signal.SIGINT)
                    obs.wait(timeout=15)
        finally:
            group_stop(game)
    clips = list(folder.glob("*.mkv"))
    if len(clips) != 1:
        raise RuntimeError(f"Expected one MKV in {folder}, found {len(clips)}")
    streams = json.loads(output("ffprobe", "-v", "error", "-show_entries",
                                "stream=codec_type", "-of", "json", str(clips[0])))
    if {s["codec_type"] for s in streams["streams"]} != {"audio", "video"}:
        raise RuntimeError(f"Missing audio or video stream: {clips[0]}")
    print(f"Saved: {clips[0]}", flush=True)


if __name__ == "__main__":
    def interrupted(signum, frame):
        raise KeyboardInterrupt(f"received signal {signum}")

    for sig in (signal.SIGTERM, signal.SIGHUP):
        signal.signal(sig, interrupted)
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--self-test", action="store_true",
                    help="capture a synthetic test pattern and isolated tone instead of FAE")
    ap.add_argument("--output-dir", type=Path, help="parent for a new, private recording folder")
    try:
        args = ap.parse_args()
        with open(".record-fae.lock", "a") as lock:
            fcntl.flock(lock, fcntl.LOCK_EX | fcntl.LOCK_NB)
            run(args.self_test, args.output_dir)
    except (OSError, RuntimeError, sp.SubprocessError, KeyboardInterrupt) as error:
        sys.exit(f"Recording launcher: {error}")
