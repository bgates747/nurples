// Linked diagnostic; synthetic clock only. No emulated game timing scores.
use super::{Test, Machine, Reg16, Path};
pub const CLOCK: u32 = 0x15400;
pub const STEP: u32 = CLOCK + 3;

fn leaf_and_parent_tags(dir: &Path) {
    for (target, phase) in [("tiles_plot",3),("vdu_scroll_down",4),("agnb_read_exact",5),
        ("vdu_buff_select",7),("vdu_plot_bmp",7),("do_player",8),("do_enemies",8),
        ("move_explosions",8),("move_tiles",8)] {
        let mut t=Test::load(dir);
        t.m._poke24(t.s["pace_read_clock"]+1,CLOCK);
        t.m._poke24(t.s["game_state"],t.s["game_playing"]);
        t.call("pace_classify");
        // Simulate an edge strictly inside the callee and return Z=1, C=0
        // (in particular the SD reader's NZ-on-error contract must survive).
        let at=t.s[target] as usize;
        t.m.0[at..at+8].copy_from_slice(&[0x3e,2,0x32,0,0,0,0xaf,0xc9]);
        t.m._poke24(at as u32+3,CLOCK);
        t.call(&format!("dl_{target}"));
        assert_eq!(t.m._peek24(t.s["dl_tags"]),phase,"{target}");
        assert_eq!(t.m.peek(t.s["dl_parent"]),1,"{target} leaked its scope");
        assert_eq!(t.c.state.reg.get16(Reg16::AF)&0x41,0x40,"{target} lost SD/status flags");
    }
}

pub fn check(dir: &Path) {
    leaf_and_parent_tags(dir);
    let mut t = Test::load(dir);
    t.m._poke24(t.s["pace_read_clock"]+1, CLOCK);
    t.m._poke24(t.s["pace_prev"], 0xfffffc);
    t.m._poke24(CLOCK, 0xfffffe);
    t.m._poke24(t.s["game_state"], t.s["game_playing"]);
    t.call("pace_classify");
    assert_eq!(t.m._peek24(t.s["dl_tags"]), 2); // first edge before classification
    t.m.poke(t.s["dl_parent"], 8);
    t.m._poke24(CLOCK, 0); // second edge in actor work, across a 24-bit wrap
    t.m.poke(STEP,2);     // third edge inside sprite-select's MOS send
    t.call("vdu_sprite_select");
    assert_eq!(t.packets.len(),1);
    assert_eq!(&t.packets[0][..3], &[23,27,4]);
    assert_eq!(t.m._peek24(t.s["dl_tags"]),0x090802);
    t.m.poke(STEP,0);
    t.m._poke24(t.s["claim_epoch"],4); // fourth edge: still counted by pace
    t.m.poke(t.s["claim_pending"],1);
    t.m._poke24(CLOCK,6); // after claim: not a fifth edge of this frame
    t.call("pace_sample");
    assert_eq!(t.m._peek24(t.s["pace_raw"]),8);
    let row=t.s["trace_rows"] as usize;
    assert_eq!(&t.m.0[row..row+10], &[0,0,4,0,0,0,2,8,9,1]);
    assert_eq!(t.m._peek24(t.s["trace_cursor"]) as usize,row+10);
    t.call("pace_classify");
    assert_eq!(t.m._peek24(t.s["dl_tags"]),2);
    // Upload with known data: preserve all incoming registers and MOS flags
    // at the wrapper boundary, except for changes made by the real service
    // (our MOS stub leaves them alone).
    t.m._poke24(CLOCK,6);
    t.m.poke(STEP,2);
    t.c.state.reg.set24(Reg16::HL,0x15700);
    t.c.state.reg.set24(Reg16::BC,2);
    t.c.state.reg.set24(Reg16::DE,0x123456);
    t.c.state.reg.set16(Reg16::AF,0xa795);
    t.call("dl_row_send");
    assert_eq!(t.c.state.reg.get24(Reg16::HL),0x15700);
    assert_eq!(t.c.state.reg.get24(Reg16::DE),0x123456);
    assert_eq!(t.c.state.reg.get24(Reg16::BC),2);
    assert_eq!(t.c.state.reg.get16(Reg16::AF),0xa795);
    assert_eq!(t.m._peek24(t.s["dl_tags"]),0x0602);
    t.m._poke24(CLOCK,9); // impossible half-VBlank: no false attribution
    t.call("dl_row_send");
    assert_eq!(t.m.peek(t.s["dl_flags"]),2);
    // A fresh on-time wait claims the only edge; that edge is NOT a skip.
    t.m._poke24(t.s["pace_prev"],100);
    t.m._poke24(CLOCK,100);
    t.call("pace_classify");
    t.m._poke24(t.s["claim_epoch"],102);
    t.m.poke(t.s["claim_pending"],0);
    t.call("pace_sample");
    assert_eq!(&t.m.0[row+10..row+20], &[0,0,1,0,0,0,10,0,0,0]);
    // Same direct wire payload and call counts for 0..32 poses, probes ON.
    t.m.poke(t.s["dl_active"],1);
    t.m.poke(STEP,2);
    t.packets.clear();
    t.check("native-direct");
    println!("EDL1 linked: carry-in, wrap, nested pose, upload, pending/wait, overflow and wire parity passed");
}
