// Run the *linked* Earth fixture's VBlank-claim code with a scripted MOS clock.
// No timed game, real MOS, UART, PRT or display. Arguments: binary, symbols.
use ez80::{Cpu, Machine, Reg16};
use std::{cell::Cell, collections::HashMap, env, fs};

const CLOCK: u32 = 0x90000;
const RETURN: u32 = 0x12345;
struct Ram {
    bytes: Vec<u8>,
    clock: Cell<u32>,
    reads: Cell<u32>,
    advance_on: u32,
}
impl Machine for Ram {
    fn peek(&self, a: u32) -> u8 {
        if a == CLOCK {
            self.reads.set(self.reads.get() + 1);
            if self.reads.get() == self.advance_on {
                self.clock.set((self.clock.get() + 2) & 0xffffff);
            }
        }
        if (CLOCK..CLOCK + 3).contains(&a) {
            (self.clock.get() >> (8 * (a - CLOCK))) as u8
        } else {
            self.bytes[a as usize]
        }
    }
    fn poke(&mut self, a: u32, v: u8) { self.bytes[a as usize] = v; }
    fn use_cycles(&self, _: i32) {}
    fn port_in(&mut self, a: u16) -> u8 { panic!("unexpected IN {a:x}"); }
    fn port_out(&mut self, a: u16, _: u8) { panic!("unexpected OUT {a:x}"); }
}
struct Harness {
    syms: HashMap<String, u32>,
    m: Ram,
    cpu: Cpu,
}
impl Harness {
    fn call(&mut self, routine: &str) -> (usize, usize) {
        let (mut legacy, mut music) = (0, 0);
        self.cpu.state.reg.set24(Reg16::SP, 0x80000);
        self.m._poke24(0x80000, RETURN);
        self.cpu.state.set_pc(self.syms[routine]);
        for _ in 0..2_000_000 {
            let pc = self.cpu.state.pc();
            if pc == RETURN { return (legacy, music); }
            legacy += usize::from(pc == self.syms["vdu_vblank"]);
            music += usize::from(pc == self.syms["lp_poll"]);
            self.cpu.fast_execute_instruction(&mut self.m);
        }
        panic!("{routine} did not terminate");
    }
    fn setup(&mut self, previous: u32, current: u32, advance_on: u32) {
        self.m._poke24(self.syms["game_state"], self.syms["game_playing"]);
        self.m._poke24(self.syms["pace_prev"], previous);
        self.m._poke24(self.syms["claim_immediate"], 0);
        for (n, v) in [("claim_pending",0),("claim_reset",0),("pace_count_this",1),
                       ("game_pause_active",0),("wait_clock_ready",0),("vdu_timing_fault_flags",0)] {
            self.m.poke(self.syms[n], v);
        }
        self.m.clock.set(current);
        self.m.reads.set(0);
        self.m.advance_on = advance_on;
    }
    fn value(&self, name: &str) -> u8 { self.m.peek(self.syms[name]) }
}
fn main() {
    let args: Vec<_> = env::args().collect();
    let bin = fs::read(&args[1]).unwrap();
    let syms = fs::read_to_string(&args[2]).unwrap().lines().filter_map(|l| {
        let (k, v) = l.split_once(" $")?;
        Some((k.to_string(), u32::from_str_radix(v.trim(), 16).ok()?))
    }).collect();
    let mut h = Harness { syms, m: Ram { bytes: vec![0; 1 << 20], clock: Cell::new(0),
        reads: Cell::new(0), advance_on: 0 }, cpu: Cpu::new_ez80() };
    h.cpu.state.reg.adl = true;
    h.m.bytes[0x40000..0x40000 + bin.len()].copy_from_slice(&bin);
    // Only the unmodified outside-scope wait and music I/O are stubbed.
    for n in ["vdu_vblank","lp_poll"] { h.m.poke(h.syms[n], 0xc9); }
    h.m._poke24(h.syms["pace_read_clock"] + 1, CLOCK);

    if h.syms.contains_key("trace_append") {
        h.call("trace_reset");
        let base = h.syms["trace_rows"];
        for (i, (mos, read, want)) in [
            (2, None, [0, 0, 1, 0]),
            (4, Some(1234), [0xd2, 4, 2, 3]),
            (0, Some(0), [0, 0, 0, 3]),
            (511, Some(65535), [255, 255, 255, 11]),
            (512, Some(65536), [255, 255, 255, 39]),
        ].iter().enumerate() {
            h.call("trace_begin_frame");
            if let Some(duration) = read {
                h.call("trace_mark_read");
                h.cpu.state.reg.set24(Reg16::HL, *duration);
                h.call("trace_valid_sd");
                assert_eq!(h.cpu.state.reg.get24(Reg16::HL), *duration);
            }
            h.cpu.state.reg.set24(Reg16::HL, *mos);
            h.call("trace_append");
            assert_eq!(h.cpu.state.reg.get24(Reg16::HL), *mos);
            let at = base as usize + i*4;
            assert_eq!(&h.m.bytes[at..at+4], want);
        }
        let end = h.syms["trace_rows_end"];
        h.m._poke24(h.syms["trace_cursor"], end-4);
        h.call("trace_append");
        h.call("trace_append");
        assert_eq!(h.m._peek24(h.syms["trace_cursor"]), end);
        assert_eq!(h.value("trace_truncated"), 1);
        println!("linked trace: SD counts, byte order, frame flags, last row and capacity passed");
    }

    for (old, now) in [(106,108),(106,112),(0xfffffe,0),(106,106+256)] {
        h.setup(old, now, 0);
        if now == 112 { h.m._poke24(h.syms["game_state"], h.syms["game_level_transition"]); }
        assert_eq!(h.call("wait_clock_frame").0, 0);
        assert_eq!(h.m._peek24(h.syms["claim_epoch"]), now);
        assert_ne!(h.value("claim_pending"), 0);
        h.call("claim_count_start");
        h.call("claim_count_start");
        assert_eq!(h.m._peek24(h.syms["claim_immediate"]), 1);
    }
    // Edge between entry and the subsequent wait loop; must not wait again.
    h.setup(116,116,2);
    assert_eq!(h.call("wait_clock_frame").0, 0);
    assert_eq!(h.m._peek24(h.syms["claim_epoch"]),118);
    assert_eq!(h.value("claim_pending"),0);
    assert_eq!(h.value("vdu_timing_fault_flags") & 1,0);

    h.setup(10,10,0);
    let (_, music) = h.call("wait_clock_frame");
    assert!(music > 0, "bounded wait lost its music service");
    assert_eq!(h.value("vdu_timing_fault_flags") & 1,1);
    assert_eq!(h.value("claim_pending"),0);
    for (n,v) in [("pace_count_this",0),("game_pause_active",1),("claim_reset",1)] {
        h.setup(106,108,0);
        h.m.poke(h.syms[n],v);
        assert_eq!(h.call("wait_clock_frame").0,1);
        assert_eq!(h.value("claim_pending"),0);
    }
    h.setup(106,108,0);
    h.m._poke24(h.syms["game_state"], h.syms["game_over"]);
    assert_eq!(h.call("wait_clock_frame").0,1);
    assert_eq!(h.value("claim_pending"),0);
    println!("linked VBlank claim: pending, multiple, rollover, waited, timeout and excluded paths passed");
}
