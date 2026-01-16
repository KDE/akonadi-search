// SPDX-FileCopyrightText: 2020 Carl Schwan <carlschwan@kde.org>
//
// SPDX-License-Identifier: LGPL-2.0-or-later

extern crate html2text;

use html2text::from_read;

pub fn convert_to_text(html: String) -> String {
    match from_read(html.as_bytes(), html.len()) {
        Ok(str) => return str,
        Err(error) => {
            eprintln!("Error parsing html: {} ", error);
            return String::new();
        }
    };
}

#[cxx::bridge]
mod ffi {
    extern "Rust" {
        fn convert_to_text(html: String) -> String;
    }
}
