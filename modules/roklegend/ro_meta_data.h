/*************************************************************************/
/*  ro_meta_data.h                                                       */
/*************************************************************************/
/*                       This file is part of:                           */
/*                  GODOT ENGINE (RokLegend Version)                     */
/*                       https://roklegend.org                           */
/*************************************************************************/
/* Copyright (c) 2017 RokLegend contributors (cf. AUTHORS.md)            */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/
#pragma once
#ifndef RO_META_DATA_H
#define RO_META_DATA_H

#include "reference.h"
#include "ustring.h"
#include "hash_map.h"
#include "set.h"

/**
  @author Vite Falcon <vitefalcon@gmail.com>
*/

class SQLite;

class RoMetaData : public Reference {
    GDCLASS(RoMetaData, Reference);

private:
    SQLite* db;
    HashMap<String, String> cached_values;

    String get_value(String prefix, String key);

    String get_value(String absolute_key);

    String get_value(String absolute_key, Set<String>& placeholders_being_resolved);

protected:
    static void _bind_methods();

public:
    RoMetaData();
    ~RoMetaData();

    String get_string(String key);

    String get_directory(String key);

    String get_filter(String key);

    String get_format(String key);
};

#endif
