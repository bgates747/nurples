// Execute the assembled production clamp and collision scan in fence order.
use ez80::{Cpu, Machine, PlainMachine};
fn main() {
    let code=std::fs::read(std::env::args().nth(1).unwrap()).unwrap();
    let mut m=PlainMachine::new();
    for (i,v) in code.into_iter().enumerate(){m.poke(i as u32,v);}
    let mut cpu=Cpu::new_ez80();
    cpu.state.reg.adl=true;
    let mut n=0;
    for muzzle in [-256,5*256,11*256,12*256,235*256,236*256,240*256,245*256,255*256] {
        for y in [86*256,87*256+255,88*256,100*256,115*256+255,116*256] {
            for pair in [[0,16],[224,240],[240,240]] {
                let x=muzzle.clamp(0,251*256);
                for (i,(px,py,w,h)) in [(muzzle,y,5,13),(pair[0]*256,100*256,16,16),
                                          (pair[1]*256,100*256,16,16)].into_iter().enumerate(){
                    let r=0x2000+i as u32*64;
                    m._poke24(r+1,px as u32);m._poke24(r+4,py as u32);
                    m.poke(r+30,w);m.poke(r+31,h);m.poke(r+29,64);
                }
                m.poke(0x2102,0);cpu.state.set_pc(0);
                for _ in 0..500 {cpu.fast_execute_instruction(&mut m);if m.peek(0x2102)!=0 {break;}}
                assert_eq!(m.peek(0x2102),1);
                assert_eq!(m._peek24(0x2001),x as u32);
                let overlap=|t| (x>>8)<t+16 && (x>>8)+5>t && (y>>8)<116 && (y>>8)+13>100;
                let first=overlap(pair[0]);
                assert_eq!((m.peek(0x2100)==0,m.peek(0x2101)==0),
                           (first,!first && overlap(pair[1])),"x={muzzle} y={y} {pair:?}");
                n+=1;
            }
        }
    }
    println!("{n} laser/fence cases passed");
}
