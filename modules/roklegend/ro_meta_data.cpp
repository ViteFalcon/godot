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
#include "ro_meta_data.h"
#include "../gdsqlite/gdsqlite.h"

#include "error_macros.h"
#include "set.h"

#define RO_META_DATA_DB "res://data/RokLegend.db"

#define RO_META_DATA_OK 0
#define RO_META_DATA_MISSING_DB 1
#define RO_META_DATA_DB_LOADING_ISSUE 2


class DummySQLite : public SQLite {
    GDCLASS(DummySQLite, SQLite);
public:
    inline int open(String path) {
        return SQLITE_OK;
    }

    inline void prepare(String query) {
        return;
    }

    inline int step() {
        return 0;
    }

    inline int step_assoc() {
        return 0;
    }

    inline Array fetch_assoc() {
        return Array();
    }

    inline Array fetch_one() {
        return Array();
    }

    inline Array fetch_array(String query) {
        return Array();
    }

    inline int query(String query) {
        return 0;
    }

    inline int get_data_count() {
        return 0;
    }

    inline int get_column_count() {
        return 0;
    }

    inline int get_column_int(int col) {
        return 0;
    }

    inline double get_column_double(int col) {
        return 0.0;
    }

    inline String get_column_text(int col) {
        return String();
    }

    inline int get_column_int_assoc(String col) {
        return 0;
    }

    inline double get_column_double_assoc(String col) {
        return 0;
    }

    inline String get_column_text_assoc(String col) {
        return String();
    }

    inline void finalize() {
        return;
    }

    inline String get_errormsg() {
        return String();
    }

    inline void close() {
        return;
    }
};


String RoMetaData::get_value(String prefix, String key) {
    static const String full_key_format = "{0}:{1}";
    String absolute_key = full_key_format.format(varray(prefix, key));
    return get_value(absolute_key);
}

String RoMetaData::get_value(String absolute_key) {
    Set<String> placeholders_being_resolved;
    return get_value(absolute_key, placeholders_being_resolved);
}

String get_unresolved_token_value(String key) {
    static const String PREFIX = "#{";
    static const String SUFFIX = "}";
    return PREFIX + key + SUFFIX;
}

String RoMetaData::get_value(String absolute_key, Set<String> &placeholders_being_resolved) {
    if (cached_values.has(absolute_key)) {
        return cached_values.get(absolute_key);
    }

    static const String CYCLIC_DEPENDENCY_ERROR_MSG_FORMAT = "Cyclic dependency found when resolving key '{0}'";
    static const String PLACEHOLDER_START("{");
    static const String PLACEHOLDER_END("}");
    // NOTE: A '#' gets prefixed to the placeholder format to indicate that it wasn't resolved.
    if (placeholders_being_resolved.has(absolute_key)) {
        ERR_PRINTS(CYCLIC_DEPENDENCY_ERROR_MSG_FORMAT.format(varray(absolute_key)));
        return get_unresolved_token_value(absolute_key);
    }

    placeholders_being_resolved.insert(absolute_key);

    // CREATE TABLE StringData (Id integer primary key autoincrement, Name TEXT not null unique, Value TEXT)
    static const String SELECT_SQL_FORMAT("SELECT Value FROM StringData WHERE Name='{0}'");
    String sql = SELECT_SQL_FORMAT.format(varray(absolute_key));
    Array result = db->fetch_array(sql);
    if (result.size() < 1) {
        static const String ERROR_MESSAGE_FORMAT("Failed to find value for key '{0}'");
        ERR_PRINTS(ERROR_MESSAGE_FORMAT.format(varray(absolute_key)));
        return get_unresolved_token_value(absolute_key);
    }
    static const String VALUE_COLUMN("Value");
    String value = result.get(0).get_named(VALUE_COLUMN);

    int search_from_index = 0;
    for (int placeholder_start_index = value.find(PLACEHOLDER_START);
         placeholder_start_index > -1;
         placeholder_start_index = value.findn(PLACEHOLDER_START, search_from_index)) {

        int placeholder_end_index = value.findn(PLACEHOLDER_END, placeholder_start_index);
        if (placeholder_end_index == -1) {
            break;
        }
        search_from_index = placeholder_end_index;
        String placeholder_name = value.substr(placeholder_start_index + 1, placeholder_end_index - placeholder_start_index - 1);
        String placeholder_value = get_value(placeholder_name, placeholders_being_resolved);
        if (placeholder_value.empty()) {
            continue;
        }
        String new_value = value.substr(0, placeholder_start_index)
                + placeholder_value
                + value.substr(placeholder_end_index + 1, value.length() - placeholder_end_index - 1);
        search_from_index = placeholder_start_index + placeholder_value.length();
        value = new_value;
    }

    cached_values.set(absolute_key, value);
    return value;
}

void RoMetaData::_bind_methods() {
    ClassDB::bind_method("get_string", &RoMetaData::get_string);
    ClassDB::bind_method("get_directory", &RoMetaData::get_directory);
    ClassDB::bind_method("get_filter", &RoMetaData::get_filter);
    ClassDB::bind_method("get_format", &RoMetaData::get_format);
}

RoMetaData::RoMetaData()
    : db(memnew(SQLite))
{
    int result = db->open_v2(RO_META_DATA_DB, true);
    if (result != SQLITE_OK) {
        ERR_PRINT("Failed to load " RO_META_DATA_DB ". Using dummy data instead.");
        db->close();
        memdelete(db);
        db = memnew(DummySQLite);
    }
}

RoMetaData::~RoMetaData() {
    memdelete(db);
}

String RoMetaData::get_string(String key) {
    return get_value("str", key);
}

String RoMetaData::get_directory(String key) {
    return get_value("dir", key);
}

String RoMetaData::get_filter(String key) {
    return get_value("filter", key);
}

String RoMetaData::get_format(String key) {
    return get_value("format", key);
}
