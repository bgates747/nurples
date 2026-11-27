// Offline feasibility only. Exercise the shipping decoder and renderer, not
// an invented note stream. No tune bytes, notes, audio or event dump escapes.
use super::{Test, Machine};
use std::{fs::File,io::{BufRead,BufReader},path::Path};

type Event = (u8,u8,u16);

fn observed(log:&Path)->Vec<Event> {
    let mut result=Vec::new();
    let mut reader=BufReader::new(File::open(log).unwrap());
    let mut buf=Vec::new();
    while {buf.clear();reader.read_until(b'\n',&mut buf).unwrap()!=0} {
        let line=String::from_utf8_lossy(&buf);
        let Some((_,tail))=line.split_once("AudioChannel: set") else {continue};
        let mut fields=tail.split_ascii_whitespace();
        let op=match fields.next(){Some("Frequency")=>3,Some("Volume")=>2,_=>continue};
        let Some(value)=fields.next().and_then(|v|v.parse::<u16>().ok()) else {continue};
        if fields.next()!=Some("on") || fields.next()!=Some("channel"){continue}
        let Some(ch)=fields.next().and_then(|v| v.parse::<u8>().ok()) else {continue};
        if ch < 5 {result.push((ch,op,value))}
    }
    result
}

pub(super) fn profile(dir:&Path, idx:u8, ticks:usize, log:Option<&Path>) {
    assert!(idx<5 && (50..=10_000).contains(&ticks));
    let mut t=Test::load(dir);
    // Match mus_audio_open's music setup (omit its unrelated voices and SFX).
    t.call("ay_init");
    t.m._poke24(t.s["ay_amps"],t.s["ay_amptab2"]);
    t.m._poke24(t.s["lp_selected_track"],t.s[&format!("mus_track_{idx}")]);
    t.packets.clear();
    t.call("lp_prime_track"); // already prepares, does not send, tick 1
    assert!(t.packets.is_empty());
    let mut lens=Vec::new();
    let mut ops=[0;16];
    let mut chan=[0;5];
    let mut events=Vec::new();
    let mut simultaneous=0;
    for i in 0..ticks {
        if i!=0 {t.call("lp_prepare");}
        let n=t.m._peek24(t.s["lp_size"]) as usize;
        assert!(n<=512 && t.m.peek(t.s["lp_status"])&4==0 && t.packets.is_empty());
        t.call("lp_send");
        lens.push(n);
        if n==0 {assert!(t.packets.is_empty());continue;}
        assert_eq!(t.packets.len(),1);
        let wire=t.packets.pop().unwrap();
        assert_eq!(wire.len(),n);
        let mut at=0;
        let mut mask=0u8;
        while at<n {
            assert_eq!(&wire[at..at+3], &[23,0,0x85]);
            let ch=wire[at+3]; let op=wire[at+4]; assert!(ch<5);
            let width=match op {2|4=>1,3|13=>2,_=>panic!("unclassified music op {op}")};
            let val=if width==1 {u16::from(wire[at+5])}
                    else {u16::from_le_bytes([wire[at+5],wire[at+6]])};
            ops[op as usize]+=1;chan[ch as usize]+=1;mask|=1<<ch;
            if op==2 || op==3 {events.push((ch,op,val));}
            at+=5+width;
        }
        assert_eq!(at,n);
        simultaneous+=usize::from(mask.count_ones()>1);
    }
    assert!(ops[2]>0 && ops[3]>0);
    let mut sorted=lens.clone();sorted.sort_unstable();
    let peak=|w:usize|lens.windows(w).map(|row|row.iter().sum::<usize>()).max().unwrap();
    let total=lens.iter().sum::<usize>();
    println!("track={idx} ticks={ticks} empty={} multi_voice={simultaneous} bytes={total} bytes/s={:.1} p95={} max={} max4={} max8={} vol={} freq={} wave={} rate={} per_channel={chan:?}",
             sorted.iter().filter(|&&n|n==0).count(), total as f64*50.0/ticks as f64,
             sorted[(95*ticks).div_ceil(100)-1],peak(1),peak(4),peak(8),ops[2],ops[3],ops[4],ops[13]);
    if let Some(log)=log {
        // Compare a consecutive sequence of *values and channel identities*
        // with independent stock VDP actions, not with our own byte counter.
        // A single matching sequence is corroboration, not a whole-run proof.
        let seen=observed(log);
        assert!(!seen.is_empty(),"no stock music setter records in this log; cannot cross-check");
        let start=events.len().min(200);
        let expected=&events[start..];
        assert!(expected.len()>=64);
        let matched=seen.windows(64).enumerate().filter(|(_,seq)|*seq==&expected[..64])
            .map(|(at,_)|seen[at..].iter().zip(expected).take_while(|(x,y)|x==y).count()).max().unwrap_or(0);
        println!("stock log: longest exact volume/frequency sequence from event {start}: {matched}/{} events",expected.len());
        assert!(matched>=64,"no corroborating PT3 sequence in the stock log");
    }
}
