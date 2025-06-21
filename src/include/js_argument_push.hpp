#pragma once
#include "duckdb.hpp"
#include "duckdb/common/types/time.hpp"
#include "duckdb/common/types/timestamp.hpp"
extern "C" {
#include "duktape.h"
}

namespace duckdb {

inline void PushValueToDuktape(duk_context *ctx, Vector &vec, idx_t row) {
    switch (vec.GetType().id()) {
        case LogicalTypeId::TINYINT: {
            auto data = FlatVector::GetData<int8_t>(vec);
            duk_push_int(ctx, static_cast<duk_int_t>(data[row]));
            break;
        }
        case LogicalTypeId::UTINYINT: {
            auto data = FlatVector::GetData<uint8_t>(vec);
            duk_push_int(ctx, static_cast<duk_int_t>(data[row]));
            break;
        }
        case LogicalTypeId::SMALLINT: {
            auto data = FlatVector::GetData<int16_t>(vec);
            duk_push_int(ctx, static_cast<duk_int_t>(data[row]));
            break;
        }
        case LogicalTypeId::USMALLINT: {
            auto data = FlatVector::GetData<uint16_t>(vec);
            duk_push_int(ctx, static_cast<duk_int_t>(data[row]));
            break;
        }
        case LogicalTypeId::INTEGER: {
            auto data = FlatVector::GetData<int32_t>(vec);
            duk_push_int(ctx, static_cast<duk_int_t>(data[row]));
            break;
        }
        case LogicalTypeId::UINTEGER: {
            auto data = FlatVector::GetData<uint32_t>(vec);
            duk_push_number(ctx, static_cast<double>(data[row]));
            break;
        }
        case LogicalTypeId::BIGINT: {
            auto data = FlatVector::GetData<int64_t>(vec);
            duk_push_number(ctx, static_cast<double>(data[row]));
            break;
        }
        case LogicalTypeId::UBIGINT: {
            auto data = FlatVector::GetData<uint64_t>(vec);
            duk_push_number(ctx, static_cast<double>(data[row]));
            break;
        }
        case LogicalTypeId::HUGEINT: {
            auto data = FlatVector::GetData<hugeint_t>(vec);
            std::string hugeint_str = data[row].ToString();
            duk_push_string(ctx, hugeint_str.c_str());
            break;
        }
        case LogicalTypeId::UHUGEINT: {
            auto data = FlatVector::GetData<hugeint_t>(vec);
            std::string uhugeint_str = data[row].ToString();
            duk_push_string(ctx, uhugeint_str.c_str());
            break;
        }
        case LogicalTypeId::FLOAT: {
            auto data = FlatVector::GetData<float>(vec);
            duk_push_number(ctx, data[row]);
            break;
        }
        case LogicalTypeId::DOUBLE: {
            auto data = FlatVector::GetData<double>(vec);
            duk_push_number(ctx, data[row]);
            break;
        }
        case LogicalTypeId::DECIMAL: {
            auto data = FlatVector::GetData<hugeint_t>(vec);
            std::string decimal_str = data[row].ToString();
            duk_push_string(ctx, decimal_str.c_str());
            break;
        }
        case LogicalTypeId::DATE: {
            auto data = FlatVector::GetData<date_t>(vec);
            std::string date_str = Date::ToString(data[row]);
            duk_push_string(ctx, date_str.c_str());
            break;
        }
        case LogicalTypeId::TIMESTAMP: {
            auto data = FlatVector::GetData<timestamp_t>(vec);
            std::string ts_str = Timestamp::ToString(data[row]);
            duk_push_string(ctx, ts_str.c_str());
            break;
        }
        case LogicalTypeId::TIME: {
            auto data = FlatVector::GetData<dtime_t>(vec);
            std::string time_str = Time::ToString(data[row]);
            duk_push_string(ctx, time_str.c_str());
            break;
        }
        case LogicalTypeId::LIST:
        case LogicalTypeId::ARRAY: {
            // For simplicity, convert to string
            auto val = vec.GetValue(row);
            std::string list_str = val.ToString();
            duk_push_string(ctx, list_str.c_str());
            break;
        }
        case LogicalTypeId::SQLNULL: {
            duk_push_null(ctx);
            break;
        }
        case LogicalTypeId::VARCHAR: {
            auto data = FlatVector::GetData<string_t>(vec);
            duk_push_string(ctx, data[row].GetString().c_str());
            break;
        }
        default: {
            auto val = vec.GetValue(row);
            std::string def_str = val.ToString();
            duk_push_string(ctx, def_str.c_str());
            break;
        }
    }
}

} // namespace duckdb