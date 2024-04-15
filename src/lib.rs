use std::ops::Range;

use anyhow::{bail, Result};

/// The size of a game save in bytes.
const SAVE_SIZE: usize = 131072;
/// The size of a save block in bytes.
const BLOCK_SIZE: usize = 57344;
/// The size of a block section in bytes.
const SECTION_SIZE: usize = 4096;

const SECTION_ID_RANGE: Range<usize> = 0x0FF4..(0x0FF4 + 2);
const SAVE_INDEX_RANGE: Range<usize> = 0x0FFC..(0x0FFC + 4);

const TRAINER_INFO_SECTION_ID: u16 = 0;
// const TEAM_SECTION_ID: u16 = 1;
// const GAME_STATE_SECTION_ID: u16 = 2;
// const MISC_SECTION_ID: u16 = 3;
// const RIVAL_INFO_SECTION_ID: u16 = 4;
// const PC_BUFFER_A_SECTION_ID: u16 = 5;
// const PC_BUFFER_B_SECTION_ID: u16 = 6;
// const PC_BUFFER_C_SECTION_ID: u16 = 7;
// const PC_BUFFER_D_SECTION_ID: u16 = 8;
// const PC_BUFFER_E_SECTION_ID: u16 = 9;
// const PC_BUFFER_F_SECTION_ID: u16 = 10;
// const PC_BUFFER_G_SECTION_ID: u16 = 11;
// const PC_BUFFER_H_SECTION_ID: u16 = 12;
// const PC_BUFFER_I_SECTION_ID: u16 = 13;

/// The offset of the player gender value in the trainer info section.
const PLAYER_GENDER_OFFSET: usize = 0x0008;

/// The player gender.
#[derive(Debug)]
pub enum Gender {
    Boy,
    Girl,
}

/// A wrapper for a save file.
pub struct Save {
    buf: Box<[u8]>,
    block_offset: usize,
    section_offsets: [usize; 14],
}

impl Save {
    pub fn new(buf: Vec<u8>) -> Result<Save> {
        if buf.len() < SAVE_SIZE {
            bail!("the game save is too small");
        }

        let save_blocks = &buf[..(BLOCK_SIZE * 2)];
        let (block_a, block_b) = save_blocks.split_at(BLOCK_SIZE as usize);

        let save_index_a = read_u32(&block_a[SAVE_INDEX_RANGE]);
        let save_index_b = read_u32(&block_b[SAVE_INDEX_RANGE]);

        // TODO: Try the other block if the most recent block is invalid.
        let (block_offset, section_offsets) = if save_index_a > save_index_b {
            (0, validate_sections(block_a)?)
        } else {
            (BLOCK_SIZE, validate_sections(block_b)?)
        };

        Ok(Save { buf: buf.into_boxed_slice(), block_offset, section_offsets })
    }

    pub fn player_gender(&self) -> Gender {
        let section = self.section(TRAINER_INFO_SECTION_ID);
        match section[PLAYER_GENDER_OFFSET] {
            0x00 => Gender::Boy,
            0x01 => Gender::Girl,
            _ => panic!("invalid player gender value"),
        }
    }

    fn section(&self, id: u16) -> &[u8] {
        let start = self.block_offset;
        let end = start + BLOCK_SIZE;
        let block = &self.buf[start..end];

        let section_start = self.section_offsets[id as usize];
        let section_end = section_start + SECTION_SIZE;
        &block[section_start..section_end]
    }
}

fn validate_sections(block: &[u8]) -> Result<[usize; 14]> {
    debug_assert!(block.len() == BLOCK_SIZE);

    let mut seen_section_ids = [false; 14];
    let mut section_offsets = [0; 14];

    let save_index = read_u32(&block[SAVE_INDEX_RANGE]);

    for (i, section) in block.chunks_exact(SECTION_SIZE).enumerate() {
        if read_u32(&block[SAVE_INDEX_RANGE]) != save_index {
            bail!("not all sections have the same save index");
        }

        let section_id = read_u16(&section[SECTION_ID_RANGE]);
        if !(0..=13).contains(&section_id) {
            bail!("invalid section ID");
        }

        if seen_section_ids[section_id as usize] {
            bail!("repeat section ID");
        } else {
            seen_section_ids[section_id as usize] = true;
        }

        section_offsets[section_id as usize] = i * SECTION_SIZE;
    }

    // I think this condition is always true after the loop, but I'll keep it
    // in anyways.
    if !seen_section_ids.iter().all(|b| *b) {
        bail!("every section didn't appear once");
    }

    Ok(section_offsets)
}

/// Reads an unsigned 16 bit integer from `buf`.
fn read_u16(buf: &[u8]) -> u16 {
    u16::from_le_bytes(buf[..2].try_into().unwrap())
}

/// Reads an unsigned 32 bit integer from `buf`.
fn read_u32(buf: &[u8]) -> u32 {
    u32::from_le_bytes(buf[..4].try_into().unwrap())
}
