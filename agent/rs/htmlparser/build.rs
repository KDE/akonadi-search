// SPDX-FileCopyrightText: 2021 Carl Schwan <carlschwan@kde.org>
//
// SPDX-License-Identifier: LGPL-2.0-or-later

extern crate cxx_build;

fn main() {
    cxx_build::bridge("src/lib.rs").compile("htmlparser")
}

