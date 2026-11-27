// Differential test of linked eZ80 instructions. Stub MOS sends and UART only.
// Args: directory holding direct/, sync/, async/ with their own bin + symbols.
use ez80::{Cpu, Machine, Reg16};
use std::{collections::HashMap, env, fs, path::Path};
#[path="earth_music_queue.rs"]
mod music_queue;

struct Ram(Vec<u8>, usize, Option<u16>);
impl Machine for Ram {
    fn peek(&self, a: u32) -> u8 { self.0[a as usize] }
    fn poke(&mut self, a: u32, v: u8) { self.0[a as usize] = v; }
    fn use_cycles(&self, _: i32) {}
    fn port_in(&mut self, a: u16) -> u8 {
        if let Some(ref mut counter) = self.2 {
            match a & 255 {
                0x84 => { assert!(*counter > 0); *counter -= 1; return *counter as u8 },
                0x85 => return (*counter >> 8) as u8,
                _ => (),
            }
        }
        assert_eq!(a & 255, 0xc5);
        self.1 += 1;
        if self.1 % 3 == 0 { 0x40 } else { 0 }
    }
    fn port_out(&mut self, _: u16, _: u8) { panic!("unexpected port write") }
}
struct Test {
    m: Ram, c: Cpu, s: HashMap<String,u32>, packets: Vec<Vec<u8>>,
    borrowed: Option<(usize,Vec<u8>)>,
    keyboard: u32, profile: u16, final_error: bool,
}
impl Test {
    fn load(dir: &Path) -> Self {
        let bin = fs::read(dir.join("nurples.bin")).unwrap();
        let s = fs::read_to_string(dir.join("nurples.symbols")).unwrap().lines().filter_map(|l| {
            let (k,v) = l.split_once(" $")?;
            Some((k.to_owned(), u32::from_str_radix(v.trim(), 16).ok()?))
        }).collect();
        let mut m = Ram(vec![0; 1<<20], 0, None);
        m.0[0x40000..0x40000+bin.len()].copy_from_slice(&bin);
        let mut c = Cpu::new_ez80();
        c.state.reg.adl = true;
        Self { m, c, s, packets: vec![], borrowed: None, keyboard:0, profile:8, final_error:false }
    }
    fn call(&mut self, name: &str) {
        self.c.state.reg.set24(Reg16::SP, 0x80000);
        self.m._poke24(0x80000, 0x12345);
        self.c.state.set_pc(self.s[name]);
        let mut busy_loops = 0;
        let limit=if matches!(name,"lp_prepare"|"lp_prime_track"|"ay_init") {100_000} else {10_000};
        for _ in 0..limit {
            let pc = self.c.state.pc();
            if pc == 0x12345 { return }
            if self.s.get("native_callback").is_some() && pc==self.s["vdu_query_mode"] {
                // Ordered normal VDP reply fence stub: SCF/RET.
                self.c.state.set_pc(0x10090);
                self.m.0[0x10090..0x10092].copy_from_slice(&[0x37,0xc9]);
                continue;
            }
            let op = self.m.peek(pc+1);
            if self.m.peek(pc) == 0x5b && (op == 0xdf || op == 0xcf) {
                let at = self.c.state.reg.get24(Reg16::HL) as usize;
                let n = self.c.state.reg.get24(Reg16::BC) as usize;
                if op==0xcf && self.s.get("native_callback").is_some() {
                    match self.c.state.reg.a() {
                        0x1d=>self.keyboard=at as u32,
                        8=>self.c.state.reg.set24(Reg16::IX,0x18000),
                        a=>panic!("unexpected MOS query {a}")
                    }
                    self.c.state.set_pc(pc+2);
                    continue;
                }
                assert!((1..=8192).contains(&n));
                assert!(self.borrowed.is_none(), "overlap with previous transfer");
                let bytes = self.m.0[at..at+n].to_vec();
                self.packets.push(bytes.clone());
                if op == 0xcf {
                    assert_eq!(self.c.state.reg.get24(Reg16::DE), self.s["pose_callback"]);
                    self.borrowed = Some((at, bytes.clone()));
                }
                self.c.state.set_pc(pc+2);
                if self.keyboard != 0 && bytes.len() >= 3 &&
                    (bytes[..3]==[23,0,0xd9] || bytes[..3]==[23,0,0xd8]) {
                    let (kind,token,value):(u8,u8,u16) = if bytes[2]==0xd9 {(1,0,5)}
                    else {
                        assert_eq!(bytes.len(),5+usize::from(bytes[3]));
                        assert_eq!(&bytes[8..11],&[0,1,0]);
                        let token=bytes[7];
                        let (k,v)=match bytes[6] {
                            1=>{assert_eq!(&bytes[11..13], &[0,0x2b]); (2,0x9471)},
                            2=>{assert_eq!(&bytes[11..], &[0x71,0x94,1,0,1,0x2b]);
                                if self.final_error {(5,1)} else {(3,0)}},
                            3=>{assert_eq!(&bytes[11..], &[0x71,0x94]); (3,0)},
                            4=>{(4,if bytes[11]==24 {self.profile} else {0})},
                            _=>panic!("invalid runtime query")
                        }; (k,token,v)
                    };
                    self.m.0[0x10100..0x1010a].copy_from_slice(&[86,68,80,76,1,kind,token,0,value as u8,(value>>8) as u8]);
                    // Inject the actual callback at transfer return, before
                    // the caller even asks MOS for the clock.
                    self.m.0[0x10040..0x1004b].copy_from_slice(&[0xd5,0x11,0,1,1,0xcd,0,0,0,0xd1,0xc9]);
                    self.m._poke24(0x10046,self.keyboard);
                    let sp=self.c.state.reg.get24(Reg16::SP)-3;
                    self.c.state.reg.set24(Reg16::SP,sp);
                    self.m._poke24(sp,pc+2);
                    self.c.state.set_pc(0x10040);
                }
            } else {
                // Deliver an interrupt only after the actual busy wait loads
                // the flag several times. Mimic the MOS wrapper preserving AF.
                if name == "pose_barrier" && self.borrowed.is_some() && self.m.peek(pc) == 0x3a &&
                    self.m._peek24(pc+1) == self.s["pose_released"] {
                    busy_loops += 1;
                    if busy_loops == 3 {
                        let (at, bytes) = self.borrowed.take().unwrap();
                        assert_eq!(self.m.0[at..at+bytes.len()], bytes);
                        self.m.0[0x10000..0x10007].copy_from_slice(&[0xf5,0xcd,0,0,0,0xf1,0xc9]);
                        self.m._poke24(0x10002,self.s["pose_callback"]);
                        let sp = self.c.state.reg.get24(Reg16::SP)-3;
                        self.c.state.reg.set24(Reg16::SP,sp);
                        self.m._poke24(sp,pc);
                        self.c.state.set_pc(0x10000);
                        continue;
                    }
                }
                self.c.fast_execute_instruction(&mut self.m);
            }
        }
        panic!("unbounded {name}");
    }
    fn check(&mut self, mode: &str) {
        if mode=="native" {
            self.profile=7;
            self.call("native_startup");
            assert_eq!(self.m.peek(self.s["native_owned"]),0);
            assert!(!self.packets.iter().any(|p|p.len()>200));
            self.call("native_shutdown");
            assert_eq!(self.keyboard,0);
            self.profile=8;
            self.call("native_startup");
            assert_eq!(self.m.peek(self.s["native_owned"]),1);
            assert_eq!(self.m._peek16(self.s["native_handle"]),0x9471);
            self.packets.clear();
        }
        let mut expected = vec![];
        let mut seed = 43u32;
        // Empty, singleton, sparse and full batches. Include negative and
        // fractional coordinates that borrow/carry across the screen origin.
        for n in [0usize,1,5,12,32,0,3] {
            let initial = self.packets.len();
            let mut state = vec![];
            self.m.poke(self.s["pose_frame_n"],0);
            self.m.poke(self.s["pose_frame_sends"],0);
            for i in 0..n {
                let rec = self.s["air_base"];
                self.c.state.reg.set24(Reg16::IY, rec);
                let id = ((3*i+12)%32+12) as u8;
                let frame = (i%6) as u8;
                self.m.poke(rec+self.s["sprite_id"], id);
                self.m.poke(rec+self.s["sprite_frame"], frame);
                let mut xy = [0u16;2];
                for (j, (coord, origin)) in [("sprite_x",128),("sprite_y",48)].iter().enumerate() {
                    seed = seed.wrapping_mul(1664525).wrapping_add(1013904223);
                    let fixed = seed & 0xffffff;
                    self.m._poke24(rec+self.s[*coord],fixed);
                    xy[j] = ((fixed >> 8) as u16).wrapping_add(*origin);
                }
                expected.extend([23,27,4,id,23,27,10,frame,23,27,13]);
                state.extend([id,frame]);
                for v in xy { expected.extend(v.to_le_bytes()); state.extend(v.to_le_bytes()); }
                self.call("pose_publish");
            }
            if n != 5 { self.call("pose_submit"); } // mid-traversal barrier flush
            if let Some((at, data)) = &self.borrowed {
                assert_eq!(self.m.0[*at..*at+data.len()], *data);
            }
            let reads_before = self.m.1;
            self.call("pose_barrier");
            if mode == "async" {
                assert_eq!(self.m.1 - reads_before, if n>0 {3} else {0});
                assert!(self.borrowed.is_none());
                assert_eq!(self.m.peek(self.s["pose_in_flight"]),0);
            }
            let calls = if mode.ends_with("direct") {3*n} else {usize::from(n>0)};
            assert_eq!(self.packets.len()-initial, calls);
            assert_eq!(self.m.peek(self.s["pose_frame_n"]), n as u8);
            assert_eq!(self.m.peek(self.s["pose_frame_sends"]), calls as u8);
            if mode == "golem" && n>0 {
                let p = &self.packets[initial];
                let le = |i:usize| u16::from_le_bytes([p[i],p[i+1]]) as u32;
                assert_eq!(&p[..3], &[23,0,160]);
                assert_eq!(le(3), self.s["pose_state_id"]);
                assert_eq!(&p[5..7], &[5,0xc2]);
                assert_eq!(le(7),self.s["pose_input_tail"]-6*n as u32);
                assert_eq!(le(9),state.len() as u32);
                let i = 11+6*n;
                assert_eq!(&p[11..i],state);
                assert_eq!(&p[i..i+3], &[23,0,160]);
                let descriptor = self.s["pose_descriptors"]+6*(n as u32-1);
                assert_eq!(le(i+3),self.m._peek24(descriptor));
                assert_eq!(&p[i+5..i+9], &[1,23,0,160]);
                assert_eq!(le(i+9),self.s["pose_state_id"]);
                assert_eq!(p[i+11],11);
                assert_eq!(le(i+12)+(u32::from(p[i+14])<<16), self.m._peek24(descriptor+3));
                assert_eq!(p.len(),26+6*n);
            } else if mode=="native" && n>0 {
                let p=&self.packets[initial]; let i=12+6*n;
                assert_eq!(&p[..9],&[23,0,160,1,0x2b,5,0xc2,0,0]);
                assert_eq!(p[9] as usize,1+6*n);
                assert_eq!(p[10],0);assert_eq!(p[11] as usize,n);
                assert_eq!(&p[12..i],state);
                assert_eq!(&p[i..],&[23,0,0xd8,12,0,1,2,0,0,0,0,0x71,0x94,0,0,1,0x2b]);
            } else if mode != "golem" && mode != "native" {
                assert_eq!(self.packets.concat(),expected);
            }
        }
        if mode=="native" {
            assert_eq!(self.m._peek24(self.s["native_sends"]),5);
            assert_eq!(self.m._peek24(self.s["native_poses"]),53);
            self.m.poke(self.s["bench_complete"],1);
            self.call("native_shutdown");
            assert_eq!(self.m.peek(self.s["bench_complete"]),1);
            assert_eq!(self.keyboard,0);
            self.call("native_startup");
            self.final_error=true;
            self.call("native_shutdown");
            assert_eq!(self.m.peek(self.s["bench_complete"]),0);
            assert_eq!(self.m.peek(self.s["native_bad_token"]),4);
            assert_eq!(self.m.peek(self.s["native_bad_kind"]),5);
            assert_eq!(self.keyboard,0);
        }
    }
}

// Exercise the actual sparse observer and transport vectors against an
// uninstrumented twin. The timer is artificial: this test proves scope,
// byte parity, event format, and capacity, not throughput.
fn cost_check(root: &Path, original: &Path, arm: &str, eps: &[u8], dest: &Path) {
    let mut t = Test::load(&root.join(arm));
    let mut plain = Test::load(&original.join(arm));
    assert_eq!(&eps[..4], b"EPS1");
    assert_eq!(eps.len(), 33 + 4544*6);
    t.m.poke(t.s["lp_owned"],1);
    let mut events = 0;
    for (frame, row) in eps[33..].chunks_exact(6).enumerate() {
        t.m.poke(t.s["pace_count_this"],1);
        t.m._poke24(t.s["pace_frames"],frame as u32);
        t.m.2 = Some(1200); // no single sampled frame takes 1,200 timestamps
        t.m._poke24(t.s["lp_irqs"],frame as u32);
        t.call("pc_frame");
        let n = row[4] as usize;
        let sends = row[5] as usize;
        for tt in [&mut t,&mut plain] {
            let cost = tt.s.contains_key("pc_frame");
            tt.m.poke(tt.s["pose_frame_n"],0);
            tt.m.poke(tt.s["pose_frame_sends"],0);
            for i in 0..n {
                let rec=tt.s["air_base"];
                tt.c.state.reg.set24(Reg16::IY,rec);
                tt.m.poke(rec+tt.s["sprite_id"],(12+i) as u8);
                tt.m.poke(rec+tt.s["sprite_frame"],((i+frame)%2) as u8);
                tt.m._poke24(rec+tt.s["sprite_x"],((i*16+3)*256+0x80) as u32);
                tt.m._poke24(rec+tt.s["sprite_y"],((frame%240)*256+0x40) as u32);
                tt.call(if cost {"pc_publish"} else {"pose_publish"});
                if arm!="direct" && i+1<sends { tt.call("pose_submit"); }
            }
            tt.call("pose_submit");
            assert_eq!(tt.m.peek(tt.s["pose_frame_n"]),row[4]);
            assert_eq!(tt.m.peek(tt.s["pose_frame_sends"]),row[5]);
        }
        assert_eq!(t.packets,plain.packets,"wire frame {}",frame+1);
        t.packets.clear(); plain.packets.clear();
        if frame+1>=17 && (frame+1-17)%61==0 {
            let pass=(frame+1-17)/61%3;
            events+=2+2*match pass {0=>n,1 if arm=="direct"=>0,_=>sends};
        }
    }
    let at = t.s["pc_events"] as usize;
    let used = t.m._peek24(t.s["pc_ptr"]) as usize-at;
    assert_eq!(used,10*events);
    assert!(t.s["pc_end"] as usize >= at+used);
    assert_eq!(t.m.peek(t.s["pc_bad"]),0);
    let mut out=b"EPT1".to_vec();
    out.extend(&eps[4..33]);
    out.push(t.m.peek(t.s["pc_arm"])); out.push(0);
    out.extend(&(used as u32).to_le_bytes()[..3]);
    out.extend(&t.m.0[at..t.s["pc_end"] as usize]);
    out.extend(&eps[33..]);
    fs::write(dest,out).unwrap(); // test-only output, never feed to the physical results table
    println!("{arm}: {events} linked stamps; 4544 frames' wire identical; synthetic clock");
}
fn main() {
    let args:Vec<_>=env::args().collect();
    if args[1]=="--music-shape" {
        assert!((5..=6).contains(&args.len()));
        music_queue::profile(Path::new(&args[2]),args[3].parse().unwrap(),args[4].parse().unwrap(),
                             args.get(5).map(Path::new));
        return;
    }
    if args[1]=="--cost" {
        assert_eq!(args.len(),7);
        cost_check(Path::new(&args[2]),Path::new(&args[3]),&args[4],&fs::read(&args[5]).unwrap(),Path::new(&args[6]));
        return;
    }
    let root = &args[1];
    for arm in ["direct","sync","async","golem","native-direct","native"] {
        Test::load(&Path::new(&root).join(arm)).check(arm);
        println!("{arm}: linked pose bytes, source lifetime and counts passed");
    }
}
