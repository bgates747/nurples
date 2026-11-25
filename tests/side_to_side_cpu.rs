// Run the assembled fixture against an independent raster-space oracle.
use ez80::{Cpu, Machine, PlainMachine};
fn put(m: &mut PlainMachine, addr:u32, x:i32) {
    for i in 0..3 { m.poke(addr+i,(x>>(8*i)) as u8); }
}
fn get(m:&PlainMachine, addr:u32)->i32 {
    ((m._peek24(addr)<<8) as i32)>>8
}
fn run(m:&mut PlainMachine, cpu:&mut Cpu, op:u8)->u8 {
    m.poke(0x2102,op); m.poke(0x2103,0);
    cpu.state.set_pc(0);
    for _ in 0..300 {
        cpu.fast_execute_instruction(m);
        let r=m.peek(0x2103); if r!=0 {return r-1;}
    }
    panic!("guest did not return");
}
fn main() {
    let mut m=PlainMachine::new(); let mut cpu=Cpu::new_ez80();
    cpu.state.reg.adl=true;
    for (i,v) in std::fs::read(std::env::args().nth(1).unwrap()).unwrap().into_iter().enumerate(){
        m.poke(i as u32,v);
    }
    let mut ticks=0;
    for (op,w,h) in [(1,16,16),(2,32,16),(3,28,28),(4,17,15)] {
        let xmax=(256-w)*256;
        for x0 in [-256,0,1,xmax/2-1,xmax/2,xmax/2+1,xmax,xmax+256] {
          for vx0 in [-128i32,-24,0,1,96,127] {
           for (y0,vy0) in [(0,-1),(100*256,0),((336-h)*256,8),(1-h*256,-1)] {
            put(&mut m,0x2001,x0); put(&mut m,0x2004,y0);
            m.poke(0x201e,w as u8); m.poke(0x201f,h as u8);
            m.poke(0x2100,vx0 as u8); m.poke(0x2101,vy0 as u8);
            let mut x=x0.clamp(0,xmax); let mut y=y0;
            let mut vx=vx0*16; let vy=vy0*16;
            if op!=4 { vx=if x*2<xmax {vx.abs()} else {-vx.abs()}; }
            else if x==0 { vx=vx.abs(); } else if x==xmax {vx=-vx.abs();}
            let mut status=run(&mut m,&mut cpu,op);
            for _ in 0..600 {
                let row=y>>8;
                let expected=if row+h<=0 || row>=336 {2} else if row<0 {1} else {0};
                assert_eq!((get(&m,0x2001),get(&m,0x2004),get(&m,0x2007),get(&m,0x200a),status),
                           (x,y,vx,vy,expected));
                ticks+=1; if status==2 {break;}
                x+=vx; y+=vy;
                if x<=0 {x=0;vx=vx.abs();} else if x>=xmax {x=xmax;vx=-vx.abs();}
                status=run(&mut m,&mut cpu,0);
            }
           }
          }
        }
    }
    println!("{ticks} actor snapshots passed");
}
