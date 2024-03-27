use std::fs;

use firered_save_editor::Save;

fn main() {
    let mut args = std::env::args();
    args.next().unwrap();
    let save_file = args.next().unwrap();
    let buf = fs::read(save_file).unwrap();

    let save = Save::new(buf);
    println!("{:?}", save.player_gender());
}
