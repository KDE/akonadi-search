// SPDX-FileCopyrightText: 2020 Carl Schwan <carlschwan@kde.org>
//
// SPDX-License-Identifier: LGPL-2.0-or-later

extern crate html2text;

use html2text::from_read;

pub fn convert_to_text(html: String) -> String {
    from_read(html.as_bytes(), html.len())
}

#[cxx::bridge]
mod ffi {
    extern "Rust" {
        fn convert_to_text(html: String) -> String;
    }
}
