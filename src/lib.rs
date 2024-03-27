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

/// The offset of the player gender value in the trainer info section.
const PLAYER_GENDER_OFFSET: usize = 0x0008;

/// The player gender.
#[derive(Debug)]
pub enum Gender {
    Boy,
    Girl,
}

/// A save block.
enum Block {
    A,
    B,
}

/// A wrapper for a save file.
pub struct Save {
    buf: Box<[u8]>,
    most_recent_block: Block,
}

impl Save {
    pub fn new(buf: Vec<u8>) -> Result<Save> {
        if buf.len() < SAVE_SIZE {
            bail!("the game save is too small");
        }

        let save_blocks = &buf[..(BLOCK_SIZE * 2)];
        let (block_a, block_b) = save_blocks.split_at(BLOCK_SIZE as usize);

        let save_index_a = save_index(block_a);
        let save_index_b = save_index(block_b);
        let most_recent_block =
            if save_index_a > save_index_b { Block::A } else { Block::B };

        Ok(Save { buf: buf.into_boxed_slice(), most_recent_block })
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
        let start = match self.most_recent_block {
            Block::A => 0,
            Block::B => BLOCK_SIZE,
        };
        let end = start + BLOCK_SIZE;
        let block = &self.buf[start..end];
        // TODO: Cache the offsets of sections?
        block
            .chunks_exact(SECTION_SIZE)
            .find(|section| read_u16(&section[SECTION_ID_RANGE]) == id)
            .unwrap()
    }
}

/// Reads the save index from `block`.
fn save_index(block: &[u8]) -> u32 {
    debug_assert!(block.len() == BLOCK_SIZE);
    // TODO: Verify that all sections have the same save index.
    let last_section = block.chunks_exact(SECTION_SIZE).last().unwrap();
    read_u32(&last_section[SAVE_INDEX_RANGE])
}

/// Reads an unsigned 16 bit integer from `buf`.
fn read_u16(buf: &[u8]) -> u16 {
    u16::from_le_bytes(buf[..2].try_into().unwrap())
}

/// Reads an unsigned 32 bit integer from `buf`.
fn read_u32(buf: &[u8]) -> u32 {
    u32::from_le_bytes(buf[..4].try_into().unwrap())
}
