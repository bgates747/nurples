// Host regression for the linked JIT opener and first real 1 KiB refill.
// Arguments: nurples.bin, symbols, control earth.agnb, aligned earth.agnb.
use ez80::{Cpu, Machine, Reg16};
use std::{collections::HashMap, env, fs};

struct Ram(Vec<u8>);
impl Machine for Ram {
    fn peek(&self, a: u32) -> u8 { self.0[a as usize] }
    fn poke(&mut self, a: u32, v: u8) { self.0[a as usize] = v; }
    fn use_cycles(&self, _: i32) {}
    fn port_in(&mut self, _: u16) -> u8 { panic!("unexpected port read"); }
    fn port_out(&mut self, _: u16, _: u8) { panic!("unexpected port write"); }
}

fn main() {
    let args: Vec<_> = env::args().collect();
    let bin = fs::read(&args[1]).unwrap();
    let symbols = fs::read_to_string(&args[2]).unwrap();
    let s: HashMap<_, _> = symbols.lines().filter_map(|line| {
        let (name, addr) = line.split_once(" $")?;
        Some((name, u32::from_str_radix(addr.trim(), 16).ok()?))
    }).collect();
    for (path, offset) in [(&args[3], 72), (&args[4], 512)] {
        let input = fs::read(path).unwrap();
        let mut m = Ram(vec![0; 1 << 20]);
        m.0[0x40000..0x40000 + bin.len()].copy_from_slice(&bin);
        for name in ["jit_time_begin", "jit_time_end"] { m.poke(s[name], 0xc9); }
        let mut c = Cpu::new_ez80();
        c.state.reg.adl = true;
        c.state.reg.set24(Reg16::SP, 0x80000);
        m._poke24(0x80000, 0x12345);
        c.state.set_pc(s["jit_open"]);
        let mut pos = 0;
        for _ in 0..100_000 {
            let pc = c.state.pc();
            if pc == 0x12345 { break; }
            if pc == s["batch_open"] { assert_eq!(pos, offset); }
            if m.peek(pc) == 0x5b && m.peek(pc + 1) == 0xcf {
                // FFSCALL loads A with its immediate function ID before RST.LIL 08.
                assert_eq!(m.peek(pc - 2), 0x3e);
                assert_eq!(c.state.reg.get24(Reg16::HL), s["agnb_fil"]);
                match m.peek(pc - 1) {
                    0x80 => {
                        let at = (s["agnb_fil"] + s["fil_obj"] + s["ffobjid_objsize"]) as usize;
                        m.0[at..at+4].copy_from_slice(&(input.len() as u32).to_le_bytes());
                    }
                    0x82 => {
                        let n = c.state.reg.get24(Reg16::BC) as usize;
                        let dest = c.state.reg.get24(Reg16::DE) as usize;
                        m.0[dest..dest+n].copy_from_slice(&input[pos..pos+n]);
                        pos += n;
                    }
                    other => panic!("unexpected FFS call {other:x}"),
                }
                // Successful full read returns BC unchanged and A=0.
                m.poke(0x100, 0xaf);
                c.state.set_pc(0x100);
                c.fast_execute_instruction(&mut m);
                c.state.set_pc(pc + 2);
            } else {
                c.fast_execute_instruction(&mut m);
            }
        }
        assert_eq!(c.state.pc(), 0x12345);
        assert_eq!(pos, offset + 1024);
        assert_eq!(m._peek24(s["jit_rows_left"]), 3584);
        let dest = s["batch_a"] as usize + 11;
        assert_eq!(&m.0[dest..dest+1024], &input[offset..offset+1024]);
        println!("linked Earth reader: DATA at {offset}, first 1 KiB correct");
    }
}
