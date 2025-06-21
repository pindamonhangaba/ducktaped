#define DUCKDB_EXTENSION_MAIN

#include "ducktaped_extension.hpp"
#include "duckdb.hpp"
#include "duckdb/common/exception.hpp"
#include "duckdb/common/string_util.hpp"
#include "duckdb/function/scalar_function.hpp"
#include "duckdb/main/extension_util.hpp"
#include <duckdb/parser/parsed_data/create_scalar_function_info.hpp>
#include "js_argument_push.hpp"

extern "C" {
#include "duktape.h"
}

namespace duckdb {

inline void JsEvalScalar(DataChunk &args, ExpressionState &state, Vector &result) {
    idx_t input_count = args.ColumnCount();
    result.SetVectorType(VectorType::FLAT_VECTOR);
    auto count = args.size();
    auto result_data = FlatVector::GetData<string_t>(result);
    auto &js_code_vector = args.data[0];

    for (idx_t row = 0; row < count; ++row) {
        try {
            if (input_count < 1) {
                result_data[row] = StringVector::AddString(result, "js_eval error: requires at least one argument (the JS function)");
                continue;
            }

            auto js_code = FlatVector::GetData<string_t>(js_code_vector)[row];
            duk_context *ctx = duk_create_heap_default();
            if (!ctx) {
                result_data[row] = StringVector::AddString(result, "js_eval error: Failed to create Duktape context");
                continue;
            }

            string wrapped_func = "("+ js_code.GetString()+")";
            if (duk_peval_string(ctx, wrapped_func.c_str()) != 0) {
                string err = duk_safe_to_string(ctx, -1);
                string stack;
                if (duk_get_prop_string(ctx, -1, "stack")) {
                    stack = duk_safe_to_string(ctx, -1);
                    duk_pop(ctx);
                }
                duk_destroy_heap(ctx);
                string error_message = "JS compile error: " + err +
                    (stack.empty() ? "" : ("\nJS stack:\n" + stack)) +
                    "\nWhile compiling JS code: " + js_code.GetString();
                result_data[row] = StringVector::AddString(result, error_message);
                continue;
            }

            duk_require_function(ctx, -1);
            int js_argc = 0;
            for (idx_t i = 1; i < input_count; ++i) {
                PushValueToDuktape(ctx, args.data[i], row);
                js_argc++;
            }
            if (duk_pcall(ctx, js_argc) != 0) {
                string err = duk_safe_to_string(ctx, -1);
                string stack;
                if (duk_get_prop_string(ctx, -1, "stack")) {
                    stack = duk_safe_to_string(ctx, -1);
                    duk_pop(ctx);
                }
                duk_destroy_heap(ctx);
                string error_message = "JS runtime error: " + err +
                    (stack.empty() ? "" : ("\nJS stack:\n" + stack));
                result_data[row] = StringVector::AddString(result, error_message);
                continue;
            }

            string result_str = duk_safe_to_string(ctx, -1);
            duk_destroy_heap(ctx);
            result_data[row] = StringVector::AddString(result, result_str);
        } catch (const std::exception &ex) {
            result_data[row] = StringVector::AddString(result, string("js_eval internal error: ") + ex.what());
        } catch (...) {
            result_data[row] = StringVector::AddString(result, "js_eval unknown internal error");
        }
    }
}

static void LoadInternal(DatabaseInstance &instance) {
    auto js_scalar_function = ScalarFunction("js_eval", {}, LogicalType::VARCHAR, JsEvalScalar);
	js_scalar_function.varargs = LogicalType::ANY;
    ExtensionUtil::RegisterFunction(instance, js_scalar_function);
}

void DucktapedExtension::Load(DuckDB &db) {
	LoadInternal(*db.instance);
}
std::string DucktapedExtension::Name() {
	return "ducktaped";
}

std::string DucktapedExtension::Version() const {
#ifdef EXT_VERSION_QUACK
	return EXT_VERSION_QUACK;
#else
	return "";
#endif
}

} // namespace duckdb

extern "C" {

DUCKDB_EXTENSION_API void ducktaped_init(duckdb::DatabaseInstance &db) {
	duckdb::DuckDB db_wrapper(db);
	db_wrapper.LoadExtension<duckdb::DucktapedExtension>();
}

DUCKDB_EXTENSION_API const char *ducktaped_version() {
	return duckdb::DuckDB::LibraryVersion();
}
}

#ifndef DUCKDB_EXTENSION_MAIN
#error DUCKDB_EXTENSION_MAIN not defined
#endif
