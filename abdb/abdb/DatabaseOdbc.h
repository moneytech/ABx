/**
 * Copyright 2017-2020 Stefan Ascher
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#pragma once

#ifdef USE_ODBC

#include "Database.h"
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define _WINSOCKAPI_
#include <windows.h>
#else
#include <sqltypes.h>
#endif

#include <sql.h>
#include <sqlext.h>

namespace DB {

/// ODBC Database driver. Should only be used with MS SQL Server.
class DatabaseOdbc final : public Database
{
private:
    /// Number of active transactions.
    unsigned transactions_;
protected:
    SQLHDBC handle_;
    SQLHENV env_;
    bool BeginTransaction() override;
    bool Rollback() override;
    bool Commit() override;
    bool InternalQuery(const std::string& query) override;
    std::shared_ptr<DBResult> InternalSelectQuery(const std::string& query) override;
    std::string Parse(const std::string& s);
public:
    DatabaseOdbc();
    ~DatabaseOdbc() override;

    bool GetParam(DBParam param) override;
    uint64_t GetLastInsertId() override;
    std::string EscapeString(const std::string& s) override;
    std::string EscapeBlob(const char* s, size_t length) override;
    void FreeResult(DBResult* res) override;
};

class OdbcResult : public DBResult
{
    friend class DatabaseOdbc;
protected:
    OdbcResult(SQLHSTMT stmt);

    typedef std::map<const std::string, uint32_t> ListNames;
    ListNames listNames_;
    bool rowAvailable_;

    SQLHSTMT handle_;
public:
    ~OdbcResult() override;
    int32_t GetInt(const std::string& col) override;
    uint32_t GetUInt(const std::string& col) override;
    int64_t GetLong(const std::string& col) override;
    uint64_t GetULong(const std::string& col) override;
    time_t GetTime(const std::string& col) override;
    std::string GetString(const std::string& col) override;
    std::string GetStream(const std::string& col) override;
    bool IsNull(const std::string& col) override;

    bool Empty() const override { return !rowAvailable_; }
    std::shared_ptr<DBResult> Next() override;
};

}

#endif
