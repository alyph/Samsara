#pragma once

#include "string.h"
#include "reflection.h"
#include "array.h"

enum class SerializeOpType
{
	read,
	write,
};

enum class Delimiter: const char
{
	section_open 	= '[',
	section_close 	= ']',
	array_open 		= '[',
	array_close 	= ']',
	value_open 		= '(',
	value_close 	= ')',
	value_separator	= ',',
	object_tag 		= '#',
	custom1			= ';',
	custom2			= ':',
};

enum class TokenType
{
	none,
	unknown,
	delimiter,
	number,
	string,
	eof,
};

template<class TTokenStream, class TContext>
class WriteOp
{
public:
	TTokenStream token_stream;
	TContext context;
	int depth{};
	bool heading{};

	inline void section(const String& key)
	{
		delim(Delimiter::section_open);
		label(key);
		delim(Delimiter::section_close);
		newline();
		newline(); // empty line as separation
		depth = 0;
		heading = false;
	}

	template<typename TColl>
	inline void collection(const TColl& collection, const String& entry_key) // TODO: variatic arguments?
	{
		::serialization::template CollectionSerializer<TColl>::write(*this, collection, entry_key);
	}

	inline void new_object()
	{
		delim(Delimiter::object_tag);
		depth = 0;
		heading = true;
	}

	inline void end_heading()
	{
		depth = 0;
		heading = false;
		newline();
	}

	template<typename TProp>
	inline void prop(const String& key, const TProp& value)
	{
		begin_prop(key);
		::serialization::template TypeSerializer<TProp>::write(*this, value);
		end_prop();
	}

	// template<typename TProp>
	// inline void prop_array(const String& key, const TProp& value)
	// {
	// 	begin_prop(key);
	// 	serialization::write_array(*this, value);
	// 	end_prop();
	// }

	template<class TS = void, typename TVal, typename ...TArgs>
	inline void value(const TVal& value, TArgs&&... args)
	{
		asserts(depth > 0 || heading);
		// serialization::TypeSerializer<TVal>::write(*this, value);
		defined_or_default_t<TS, ::serialization::template TypeSerializer<TVal>>::write(*this, value, std::forward<TArgs>(args)...);
	}

	// template<typename TVal>
	// inline void value_string_id(const TVal& value, const String& collection_name)
	// {
	// 	asserts(depth > 0);
	// 	// IMPL
	// 	asserts(false); // not implemented
	// }

	// template<typename TVal>
	// inline void array_value(const TVal& value)
	// {
	// 	asserts(depth > 0);
	// 	serialization::write_array(*this, value);
	// }

	// template<typename TArray, typename TMemPtr>
	// inline void prop_struct_array(const String& key, const TArray& array, TMemPtr ptr)
	// {
	// 	emit_key(key); 
	// 	serialization::serialize_struct_array(*this, array, ptr);
	// }

	template<class TS, typename TBlob, typename ...TArgs>
	inline void blob(const TBlob& blob, TArgs&&... args)
	{
		TS::write(*this, blob, std::forward<TArgs>(args)...);
	}

	// // TODO: rename
	// inline void emit_key(const String& key)
	// {
	// 	// IMPL
	// 	label(key);
	// }

	inline void begin_prop(const String& key)
	{
		asserts(depth == 0 && !heading); // TODO: need handle sub object
		label(key);
		depth++;
	}

	inline void end_prop()
	{
		newline();
		depth--;
		asserts(depth >= 0);
	}

	inline void delim(Delimiter del)
	{
		token_stream.emit_delim(del);
	}

	template<typename T>
	inline void number(T num)
	{
		token_stream.emit_number(num);
	}

	inline void string(const String& str)
	{
		token_stream.emit_string(str);
	}

	inline void label(const String& label)
	{
		token_stream.emit_string(label); // TODO: we don't wan this to form a quoted string, so maybe emit_string need take some argument to indicate what kind of strings is emitted
	}

	// inline void character(char ch)
	// {
	// 	// IMPL
	// }

	inline void blob_char(char ch)
	{
		token_stream.emit_char(ch);		
	}

	inline void blob_next_line()
	{
		newline();
	}

	void newline()
	{
		token_stream.emit_newline();
	}
};

template<class TTokenStream, class TContext>
class ReadOp
{
public:
	struct PropertySearchEntry
	{
		uint64_t key_hash;
		String key_name;
		size_t pos_after_key;
	};

	TTokenStream token_stream;
	TContext context;
	int depth{};
	bool heading{};
	Array<PropertySearchEntry> found_props{0, 128, Allocator::temp};

	inline void section(const String& key)
	{
		delim(Delimiter::section_open);
		label(key); // TODO: should we search if not matched
		delim(Delimiter::section_close);
		newline();
		// no need for separation, we auto skip white spaces by default
		depth = 0;
		heading = false;
	}

	template<typename TColl>
	inline void collection(TColl& collection, const String& entry_key) // TODO: variatic arguments?
	{
		static_assert(!std::is_const_v<TColl>);
		::serialization::template CollectionSerializer<TColl>::read(*this, collection, entry_key);
	}

	inline bool next_object()
	{
		while (!at_delim(Delimiter::object_tag))
		{
			if (at_delim(Delimiter::section_open) ||
				!token_stream.seek_next_line())
			{
				return false;
			}
		}
		delim(Delimiter::object_tag);
		depth = 0;
		heading = true;
		return true;
	}

	inline void end_heading()
	{
		depth = 0;
		heading = false;
		newline();
	}

	template<typename TProp>
	inline void prop(const String& key, TProp& value)
	{
		static_assert(!std::is_const_v<TProp>);
		if (begin_prop(key))
		{
			::serialization::template TypeSerializer<TProp>::read(*this, value);
			end_prop();
		}
	}

	// template<typename TProp>
	// inline void prop_array(const String& key, TProp& value)
	// {
	// 	static_assert(!std::is_const_v<TProp>);
	// 	if (begin_prop(key))
	// 	{
	// 		serialization::read_array(*this, value);
	// 		end_prop();
	// 	}
	// }
	
	template<class TS = void, typename TVal, typename ...TArgs>
	inline void value(TVal& value, TArgs&&... args)
	{
		static_assert(!std::is_const_v<TVal>);
		asserts(depth > 0 || heading);
		// serialization::TypeSerializer<TVal>::read(*this, value);
		defined_or_default_t<TS, ::serialization::template TypeSerializer<TVal>>::read(*this, value, std::forward<TArgs>(args)...);
	}

	// template<typename TVal>
	// inline void value_string_id(TVal& value, const String& collection_name)
	// {
	// 	static_assert(!std::is_const_v<TVal>);
	// 	asserts(depth > 0);
	// 	asserts(false); // not implemented
	// }

	// template<typename TVal>
	// inline void array_value(TVal& value)
	// {
	// 	static_assert(!std::is_const_v<TVal>);
	// 	asserts(depth > 0);
	// 	serialization::read_array(*this, value);
	// }


	// template<typename TArray, typename TMemPtr>
	// inline void prop_struct_array(const String& key, TArray& array, TMemPtr ptr)
	// {
	// 	static_assert(!std::is_const_v<TArray>);
	// 	search_and_emit_key(key);
	// 	serialization::serialize_struct_array(*this, array, ptr);
	// }

	template<class TS, typename TBlob, typename ...TArgs>
	inline void blob(TBlob& blob, TArgs&&... args)
	{
		static_assert(!std::is_const_v<TBlob>);
		TS::read(*this, blob, std::forward<TArgs>(args)...);
	}

	inline bool search_and_emit_key(const String& key)
	{
		String parsed_key;
		if (token_stream.peek() == TokenType::string)
		{
			token_stream.emit_string(parsed_key);
			if (parsed_key == key)
			{
				return true;
			}
		}

		if (!found_props.empty())
		{
			token_stream.seek(found_props.back().pos_after_key);
			token_stream.seek_next_line();
		}

		while (token_stream.peek() == TokenType::string)
		{
			token_stream.emit_string(parsed_key);
			found_props.push_back({parsed_key.hash(), parsed_key, token_stream.cursor()});
			if (parsed_key == key)
			{
				return true;
			}
		}

		const auto key_hash = key.hash();
		for (const auto& prop : found_props)
		{
			if (prop.key_hash == key_hash)
			{
				asserts(prop.key_name == key); // TODO: replace it with error report
				token_stream.seek(prop.pos_after_key);
				return true;
			}
		}
		return false;
	}

	inline bool begin_prop(const String& key)
	{
		asserts(depth == 0 && !heading); // TODO: need handle sub object
		if (search_and_emit_key(key))
		{
			depth++;
			return true;
		}
		return false;
	}

	inline void end_prop()
	{
		token_stream.emit_newline();
		depth--;
		asserts(depth >= 0);
	}

	inline void delim(Delimiter del)
	{
		token_stream.emit_delim(del);
	}

	inline bool opt_delim(Delimiter del)
	{
		if (at_delim(del))
		{
			delim(del);
			return true;
		}
		return false;
	}

	template<typename T>
	inline void number(T& num)
	{
		static_assert(!std::is_const_v<T>);
		token_stream.emit_number(num);
	}

	template<typename T>
	inline bool opt_number(T& num)
	{
		static_assert(!std::is_const_v<T>);
		if (token_stream.peek() == TokenType::number)
		{
			number(num);
			return true;
		}
		return false;
	}

	inline void string(String& str)
	{
		token_stream.emit_string(str);
	}

	inline bool opt_string(String& str)
	{
		if (token_stream.peek() == TokenType::string)
		{
			string(str);
			return true;
		}
		return false;
	}

	inline void label(const String& label)
	{
		token_stream.emit_matched_string(label);
	}

	inline void newline()
	{
		token_stream.emit_newline();
	}

	inline bool blob_char(char& ch)
	{
		return token_stream.emit_char(ch, true);
	}

	inline bool blob_next_line()
	{
		while (token_stream.seek_next_line())
		{
			const auto line_begin = token_stream.cursor();
			if (!at_eol())
			{
				token_stream.seek(line_begin);
				return true;
			}
		}
		return false;
	}

	inline bool at_delim(Delimiter del)
	{
		Delimiter next_del{};
		return (token_stream.peek_delim(next_del) && next_del == del);
	}

	inline bool at_eol()
	{
		token_stream.skip_spaces(true);
		return token_stream.peek_eol();
	}


	// bool match(TokenMatch type)
	// {
	// 	// IMPL
	// 	return false;
	// }
};

// template<typename T>
// void serialize(const T& obj, const String& path)
// {
// 	SerializeOp<TokenWriteStream> op;
// 	serialization::schema(op, obj);
// }

// template<typename T>
// void deserialize(T& obj, const String& path)
// {
// 	DeserializeOp<TokenReadStream> op;
// 	serialization::schema(op, obj);
	
// }


// template<class TContext>
// inline WriteOp<FileTokenWriteStream, TContext> make_file_op<SerializeOpType::write, TContext>(const String& path, const TContext& context)
// {
// 	auto op = WriteOp<FileTokenWriteStream, TContext>{};
// 	op.context = context;
// }

// template<class TContext>
// inline ReadOp<FileTokenReadStream, TContext> make_file_op<SerializeOpType::read, TContext>(const String& path, const TContext& context)
// {
// 	auto op = ReadOp<FileTokenReadStream, TContext>{};
// 	op.context = context;
// }

namespace serialization
{
	template<typename TS, typename TO>
	struct OpObjectRef {};

	template<class TT, class TC, typename TO>
	struct OpObjectRef<WriteOp<TT, TC>, TO>
	{
		using type = const TO&;
	};

	template<class TT, class TC, typename TO>
	struct OpObjectRef<ReadOp<TT, TC>, TO>
	{
		using type = TO&;
	};

	template<typename TS, typename TO>
	using RefType = typename OpObjectRef<TS, TO>::type;


	template<SerializeOpType EOT, typename TO>
	struct RefSelectorByOpType {};

	template<typename TO>
	struct RefSelectorByOpType<SerializeOpType::write, TO>
	{
		using type = const TO&;
	};

	template<typename TO>
	struct RefSelectorByOpType<SerializeOpType::read, TO>
	{
		using type = TO&;
	};

	template<SerializeOpType EOT, typename TO>
	using RefTypeE = typename RefSelectorByOpType<EOT, TO>::type;

	template<class T>
	struct TypeSerializer
	{
		template<class TOp>	static inline void write(TOp& op, const T& v) { serialize(op, v); }
		template<class TOp>	static inline void read(TOp& op, T& v) { serialize(op, v); }
	};

	template<class T>
	struct CollectionSerializer {};


	template<class T>
	struct CollectionSerializer<Array<T>>
	{
		template<class TOp>
		static inline void write(TOp& op, const Array<T>& array, const String& entry_key)
		{
			static_assert(false);
		}

		template<class TOp>
		static inline void read(TOp& op, Array<T>& array, const String& entry_key)
		{
			static_assert(false);
		}
	};
	// template<class TOp, class T>
	// inline std::enable_if_t<is_template_v<std::remove_const_t<T>, Array>>
	// serialize(TOp& op, T& v)
	// {
	// 	op.array_value(v);
	// }

	template<class T>
	struct TypeSerializer<Array<T>>
	{
		template<class TOp>
		static inline void write(TOp& op, const Array<T>& array)
		{
			op.delim(Delimiter::array_open);
			size_t idx = 0;
			size_t size = array.size();
			for (const T& elem : array)
			{
				op.value(elem);
				if (idx++ < (size - 1))
				{
					op.delim(Delimiter::value_separator);
				}
			}
			op.delim(Delimiter::array_close);
		}

		template<class TOp>
		static inline void read(TOp& op, Array<T>& array)
		{
			array.clear();
			op.delim(Delimiter::array_open);
			while (!op.opt_delim(Delimiter::array_close))
			{
				T elem;
				op.value(elem);
				array.push_back(elem);
				op.opt_delim(Delimiter::value_separator);
			}
		}
	};

	// Common serialization functions

	template<class TOp, class T>
	inline std::enable_if_t<std::is_arithmetic_v<T>>
	serialize(TOp& op, T& v)
	{
		op.number(v);
	}
}

