// Exercise the linked game routines, not a second implementation of allocation.
// Arguments: game binary and its matching ez80asm symbols. Only MOS VDU I/O is stubbed.
use ez80::{Cpu, Machine, Reg16};
use std::{collections::HashMap, env, fs};

struct Ram(Vec<u8>);
impl Machine for Ram {
    fn peek(&self, a:u32)->u8 { self.0[a as usize] }
    fn poke(&mut self,a:u32,v:u8) { self.0[a as usize]=v; }
    fn use_cycles(&self,_:i32) {}
    fn port_in(&mut self,_:u16)->u8 { panic!("unexpected IN"); }
    fn port_out(&mut self,_:u16,_:u8) { panic!("unexpected OUT"); }
}
fn call(m:&mut Ram,c:&mut Cpu,addr:u32) {
    c.state.reg.set24(Reg16::SP,0x80000);
    m._poke24(0x80000,0x012345);
    c.state.set_pc(addr);
    for _ in 0..100000 {
        c.fast_execute_instruction(m);
        if c.state.pc()==0x012345 { return; }
    }
    panic!("no return from {addr:x}");
}
fn main() {
    let args:Vec<_>=env::args().collect();
    let bin=fs::read(&args[1]).unwrap();
    let syms=fs::read_to_string(&args[2]).unwrap();
    let s:HashMap<_,_>=syms.lines().filter_map(|l|{
        let (k,v)=l.split_once(" $")?;
        Some((k,u32::from_str_radix(v.trim(),16).ok()?))
    }).collect();
    let mut m=Ram(vec![0;1<<20]);
    m.0[0x40000..0x40000+bin.len()].copy_from_slice(&bin);
    for name in ["vdu_sprite_select", "vdu_sprite_hide"] {
        m.poke(s[name],0xc9); // no display attached
    }
    let mut c=Cpu::new_ez80(); c.state.reg.adl=true;
    let base=s["formation_base"]; let live=s["formation_live"];
    let size=s["table_record_size"];
    assert_eq!(s["formation_end"]-base,40*size);
    assert_eq!(s["PRESENTATION_SPRITE_TITLE"],68);
    call(&mut m,&mut c,s["table_init"]);
    m.poke(s["tiles_row"],m.peek(s["earth_rank_programs"]));
    for n in 0..40 {
        let launch=["earth_launch_small","earth_launch_medium","earth_launch_large"][n%3];
        call(&mut m,&mut c,s[launch]);
        assert_eq!(c.state.reg.get24(Reg16::IY),base+(39-n as u32)*size);
        assert_eq!(m.peek(live),1+n as u8);
    }
    let snapshot=m.0[base as usize..s["table_end"] as usize].to_vec();
    call(&mut m,&mut c,s["earth_launch_small"]);
    assert_eq!(m.peek(live),40);
    assert_eq!(snapshot,m.0[base as usize..s["table_end"] as usize]);
    // Kill half through the ordinary pending-death dispatcher and scroll the
    // others beyond the last playfield row through their real move program.
    for slot in 0..40 {
        let rec=base+slot*size;
        let t=rec+s["sprite_type"];
        m.poke(t,m.peek(t)&0x7f); // first publication has happened
        if slot%2==0 {
            let flags=rec+s["sprite_collisions"];
            m.poke(flags,m.peek(flags)|0x80);
        } else {
            m._poke24(rec+s["sprite_y"],(s["field_bottom"]+1)*256);
        }
    }
    call(&mut m,&mut c,s["enemies_alive"]);
    assert_eq!(m.peek(live),0);
    assert_eq!(m.peek(s["sprite_fault_flags"]),0);
    for slot in 0..40 {
        c.state.reg.set24(Reg16::IY,base+slot*size);
        call(&mut m,&mut c,s["table_deactivate_sprite"]);
        assert_eq!(m.peek(live),0); // repeated teardown cannot underflow
    }
    call(&mut m,&mut c,s["activate_pad_medium"]);
    assert_eq!(m.peek(live),0);
    assert_eq!(c.state.reg.get24(Reg16::IY),s["air_base"]);
    call(&mut m,&mut c,s["earth_launch_large"]);
    assert_eq!(c.state.reg.get24(Reg16::IY),base+39*size);
    assert_eq!(m.peek(live),1);
    call(&mut m,&mut c,s["table_init"]);
    assert_eq!(m.peek(live),0);

    // Actual row admission cadence and assembled movement, no renderer.
    // One representative at its original X per rank is sufficient to catch
    // phase slip; exercise both edges using the widest ranks.
    for (start, ranks, stride) in [
        (233u8, vec![(0,6,2),(16,15,2),(32,7,2),(48,14,2),(64,8,2)],96),
        (186u8, vec![(0,4,2),(0,13,3),(32,6,4)],64)
    ] {
        call(&mut m,&mut c,s["table_init"]);
        let release=ranks.last().unwrap().0;
        m.poke(s["terrain_step_pixels"],1);
        let mut actors=vec![];
        for tick in 0usize..=release+stride*2+1 {
            for &(birth,col,gid) in &ranks {
                if tick!=birth {continue;}
                m.poke(s["tiles_row"],start-(birth/16) as u8);
                m._poke16(s["tiles_x_plot"],col*16);
                call(&mut m,&mut c,s[["earth_launch_small","earth_launch_medium","earth_launch_large"][gid-2]]);
                let rec=c.state.reg.get24(Reg16::IY);
                actors.push((rec,birth,m._peek24(rec+s["sprite_x"]),m._peek24(rec+s["sprite_y"])));
            }
            for &(rec,birth,x,y) in &actors {
                if tick!=birth {
                    c.state.reg.set24(Reg16::IY,rec);
                    call(&mut m,&mut c,s["earth_march_tick"]);
                }
                let age=tick.saturating_sub(release);
                let phase=age%(2*stride);
                let offset=phase.min(2*stride-phase) as u32;
                assert_eq!(m._peek24(rec+s["sprite_x"]),x-offset*256);
                assert_eq!(m._peek24(rec+s["sprite_y"]),
                    (y+((tick.min(release)-birth)*256+age*128) as u32)&0xffffff);
            }
        }
    }
}
