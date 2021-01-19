#pragma once

#include "serialization.h"
#include <cstdio>
#include <cstdlib>
#include <cctype>
#include <type_traits>

static constexpr const char newline_char = '\n';

enum FileMode
{
	read,
	write,
};

class File
{
public:
	FILE* fp{};

	File() = default;
	File(const File& other) = delete;
	inline File(File&& other);
	inline ~File();

	File& operator=(const File& other) = delete;
	inline File& operator=(File&& other);
	
	inline bool open(const String& path, FileMode mode);
	inline void close();
	inline void put(char ch);
	inline bool get(char& out_ch);
	inline void unget(char ch);
	inline bool peek(char& out_ch);
	inline size_t tell();
	inline void seek(size_t pos);
	inline size_t read(uint8_t* data, size_t size);
};


class FileTokenWriteStream
{
public:
	inline void open(const String& path);
	inline void emit_delim(Delimiter del);
	template<typename T>
	inline void emit_number(T num);
	inline void emit_string(const String& str);
	inline void emit_newline();
	inline void emit_char(char ch);
	inline operator bool() const { return file.fp != nullptr; }

private:
	File file;
	TokenType last_token_type{};
	char last_token_char{};
	inline void prepare_next_token(TokenType next_type, char next_char);
};

class FileTokenReadStream
{
public:
	void open(const String& path);
	inline size_t emit_delim(Delimiter del);
	template<typename T>
	inline size_t emit_number(T& num);
	inline size_t emit_string(String& str);
	inline size_t emit_matched_string(const String& str);
	inline bool emit_char(char& ch, bool stop_at_eol);
	inline size_t emit_newline();
	inline TokenType peek();
	inline bool peek_delim(Delimiter& out_delim);
	inline bool peek_eol();
	inline bool seek_next_line();
	inline size_t skip_spaces(bool stop_at_eol);
	inline size_t cursor() const;
	inline void seek(size_t pos);
	inline operator bool() const { return buffer.data != nullptr; }

private:
	struct NextTokenInfo
	{
		TokenType type{};
		char first{}; // TODO: maybe we don't need first anymore
		size_t begin{};
	};

	Buffer buffer;
	size_t current{};
	NextTokenInfo next_token;

	inline bool eof(size_t pos) { return pos >= buffer.size(); }
	inline char char_at(size_t pos) { return *buffer.get(pos); }
	inline bool test_char_at(size_t pos, char& out_ch);
	inline Delimiter consume_delimiter();
	inline const char* consume_number(size_t& end_pos); // end_pos is one beyond the last char of the number
	inline const char* consume_string(size_t& len);
	inline size_t next_non_space(bool stop_at_eol);
};

template<SerializeOpType T, class TContext>
struct FileOpSelector {};

template<class TContext>
struct FileOpSelector<SerializeOpType::read, TContext>
{
	using type = ReadOp<FileTokenReadStream, TContext>;
};

template<class TContext>
struct FileOpSelector<SerializeOpType::write, TContext>
{
	using type = WriteOp<FileTokenWriteStream, TContext>;
};

template<SerializeOpType T, class TContext>
using FileOpType = typename FileOpSelector<T, TContext>::type;

template<SerializeOpType T, class TContext>
inline FileOpType<T, TContext> make_file_op(const String& path, const TContext& context)
{
	FileOpType<T, TContext> op{};
	op.context = context;
	op.token_stream.open(path);
	asserts(op.token_stream); // TODO: have a way to indicate error occured
	return op;
}

inline File::File(File&& other)
{
	fp = other.fp;
	other.fp = nullptr;
}

inline File::~File()
{
	close();
}

inline File& File::operator=(File&& other)
{
	close();
	fp = other.fp;
	other.fp = nullptr;
	return *this;
}

inline bool File::open(const String& path, FileMode mode)
{
	close();
	if (mode == FileMode::read)
	{
		fp = std::fopen(path.c_str(), "r");
	}
	else if (mode == FileMode::write)
	{
		fp = std::fopen(path.c_str(), "w");
	}
	else
	{
		asserts(false);
	}
	return (fp != nullptr);
}

inline void File::close()
{
	if (fp)
	{
		std::fclose(fp);
		fp = nullptr;
	}
}

inline void File::put(char ch)
{
	std::putc(ch, fp);
}

inline bool File::get(char& out_ch)
{
	const auto c = std::getc(fp);
	if (c == EOF)
	{
		out_ch = 0;
		return false;
	}
	else
	{
		out_ch = static_cast<char>(c);
		return true;
	}
}

inline void File::unget(char ch)
{
	std::ungetc(ch, fp);
}

inline bool File::peek(char& out_ch)
{
	int c = std::getc(fp);
	if (c == EOF)
	{
		out_ch = 0;
		return false;
	}
	else
	{
		out_ch = static_cast<char>(c);
		std::ungetc(c, fp);
		return true;
	}
}

inline size_t File::tell()
{
	std::fpos_t fpos;
	const auto ret = std::fgetpos(fp, &fpos);
	asserts(ret == 0);
	return static_cast<size_t>(fpos);
}

inline void File::seek(size_t pos)
{
	std::fpos_t fpos = static_cast<std::fpos_t>(pos);
	const auto ret = std::fsetpos(fp, &fpos);
	asserts(ret == 0);
}

inline size_t File::read(uint8_t* data, size_t size)
{
	return std::fread(data, sizeof(uint8_t), size, fp);
}


inline void FileTokenWriteStream::open(const String& path)
{
	*this = {};
	file.open(path, FileMode::write);
}

inline void FileTokenWriteStream::emit_delim(Delimiter del)
{
	const auto ch = static_cast<char>(del);
	prepare_next_token(TokenType::delimiter, ch);
	file.put(ch);
}

template<typename T>
inline void FileTokenWriteStream::emit_number(T num)
{
	prepare_next_token(TokenType::number, 0);
	if constexpr (std::is_integral_v<T>)
	{
		if constexpr (std::is_signed_v<T>)
		{
			long long d = num;
			std::fprintf(file.fp, "%lld", d);
		}
		else
		{
			unsigned long long u = num;
			std::fprintf(file.fp, "%llu", u);
		}
	}
	else if constexpr (std::is_floating_point_v<T>)
	{
		std::fprintf(file.fp, "%g", num);
	}
	else
	{
		static_assert(false, "unsupported number type");
	}
}

inline void FileTokenWriteStream::emit_string(const String& str)
{
	prepare_next_token(TokenType::string, 0);
	// TODO: space in string etc.
	const auto begin = str.data();
	for (auto i = 0; i < str.size(); i++)
	{
		file.put(*(begin + i));
	}
}

inline void FileTokenWriteStream::emit_newline()
{
	file.put(newline_char);
	last_token_type = TokenType::none;
	last_token_char = 0;
}

inline void FileTokenWriteStream::emit_char(char ch)
{
	file.put(ch);
	last_token_type = TokenType::none;
	last_token_char = 0;
}

inline void FileTokenWriteStream::prepare_next_token(TokenType next_type, char next_char)
{
	bool need_space = true;
	if (last_token_type != TokenType::none)
	{
		if (last_token_type == TokenType::delimiter)
		{
			const Delimiter del = static_cast<Delimiter>(last_token_char);
			if (del == Delimiter::section_open ||
				del == Delimiter::array_open ||
				del == Delimiter::value_open)
			{
				need_space = false;
			}
		}
		if (next_type == TokenType::delimiter)
		{
			const Delimiter del = static_cast<Delimiter>(next_char);
			if (del != Delimiter::section_open &&
				del != Delimiter::array_open &&
				del != Delimiter::value_open &&
				del != Delimiter::object_tag)
			{
				need_space = false;
			}
		}
	}
	else
	{
		need_space = false;
	}

	if (need_space)
	{
		file.put(' ');
	}
	last_token_type = next_type;
	last_token_char = next_char;
}



inline size_t FileTokenReadStream::emit_delim(Delimiter del)
{
	Delimiter parsed_delim{};
	if (peek_delim(parsed_delim) && parsed_delim == del)
	{
		consume_delimiter();
	}
	else
	{
		asserts(false); // report error
	}
	return current;
}

template<typename T>
inline size_t FileTokenReadStream::emit_number(T& num)
{
	if (peek() == TokenType::number)
	{
		size_t end_pos{};
		auto num_str = consume_number(end_pos);
		auto end_ptr = buffer.get(end_pos);
		const auto old_end_ch = *end_ptr;
		*end_ptr = 0; // null terminate, so the c functions can work
		char* dummy_end;
		if constexpr (std::is_integral_v<T>)
		{
			num = static_cast<T>(std::strtoll(num_str, &dummy_end, 10));
		}
		else if constexpr (std::is_floating_point_v<T>)
		{
			if constexpr (sizeof(T) <= 4)
			{
				num = static_cast<T>(std::strtof(num_str, &dummy_end));
			}
			else
			{
				num = static_cast<T>(std::strtod(num_str, &dummy_end));
			}
		}
		else
		{
			static_assert(false, "unsupported number type");
		}
		*end_ptr = old_end_ch; // set it back
	}
	else
	{
		asserts(false); // report error
	}
	return current;
}

inline size_t FileTokenReadStream::emit_string(String& str)
{
	if (peek() == TokenType::string)
	{
		size_t len{};
		auto start = consume_string(len);
		str = String{start, len};
	}
	else
	{
		asserts(false); // report error
	}
	return current;
}

inline size_t FileTokenReadStream::emit_matched_string(const String& str)
{
	if (peek() == TokenType::string)
	{
		const auto prev_pos = current;
		size_t len{};
		auto start = consume_string(len);
		if (str != make_string_view(start, len))
		{
			asserts(false); // report error
			current = prev_pos; // NOTE: consume_string() is not destructive but merely move the current, so it's safe to simply move the current back
		}
	}
	else
	{
		asserts(false); // report error
	}
	return current;
}

inline bool FileTokenReadStream::emit_char(char& ch, bool stop_at_eol)
{
	// NOTE: no need rewind or clear next token, since if we progress past the next token position it will be auto invalidated
	if (!eof(current))
	{
		const auto next_ch = char_at(current);
		if (next_ch != newline_char || !stop_at_eol)
		{
			ch = next_ch;
			current++;
			return true;
		}
	}
	return false;
}

inline size_t FileTokenReadStream::emit_newline()
{
	// NOTE: since we only skip white spaces (including new liens), it shouldn't affect the next token info
	const auto pos = next_non_space(true);
	if (!eof(pos))
	{
		const auto next_ch = char_at(pos);
		if (next_ch == newline_char)
		{
			current = (pos + 1);
		}
		else
		{
			asserts(false); // report error
		}
	}
	else
	{
		// silently stop here if at eof
		current = pos;
	}
	return current;
}

inline TokenType FileTokenReadStream::peek()
{
	if (next_token.type == TokenType::none || current > next_token.begin)
	{
		auto pos = next_non_space(false);
		next_token.type = TokenType::eof;
		next_token.first = '\0';
		next_token.begin = pos;
		while (!eof(pos))
		{
			const auto ch = char_at(pos);
			if (std::isdigit(ch))
			{
				next_token.type = TokenType::number;
			}
			else if (std::isalpha(ch) || ch == '_')
			{
				next_token.type = TokenType::string;
			}
			else if (ch == '.')
			{
				char next_ch{};
				if (test_char_at(pos+1, next_ch) && std::isdigit(next_ch))
				{
					next_token.type = TokenType::number;
				}
				else
				{
					next_token.type = TokenType::delimiter;
				}
			}
			else if (ch == '+' || ch == '-')
			{
				char next_ch{};
				if (test_char_at(pos+1, next_ch) && (next_ch == '.' || std::isdigit(next_ch))) // TODO: if it's "+.???" test the next next char
				{
					next_token.type = TokenType::number;
				}
				else
				{
					next_token.type = TokenType::delimiter;
				}
			}
			else if (std::ispunct(ch))
			{
				next_token.type = TokenType::delimiter;
			}
			else
			{
				continue;
			}
			next_token.first = ch;
			next_token.begin = pos;
			break;
		}

	}
	return next_token.type;
}

inline bool FileTokenReadStream::peek_delim(Delimiter& out_delim)
{
	if (peek() == TokenType::delimiter)
	{
		out_delim = static_cast<Delimiter>(next_token.first);
		return true;
	}
	return false;
}

inline bool FileTokenReadStream::peek_eol()
{
	char ch;
	if (test_char_at(current, ch))
	{
		return (ch == newline_char);
	}
	return true; // eof treated as eol
}

inline bool FileTokenReadStream::seek_next_line()
{
	// NOTE: no need rewind or clear next token, since if we progress past the next token position it will be auto invalidated
	char ch;
	while (test_char_at(current, ch))
	{
		current++;
		if (ch == newline_char)
		{
			return true;
		}
	}
	return false;
}

inline size_t FileTokenReadStream::skip_spaces(bool stop_at_eol)
{
	// NOTE: since token always at next non-space, no need to clear peeked tokens
	return (current = next_non_space(stop_at_eol));
}

inline size_t FileTokenReadStream::cursor() const
{
	return current;
}

inline void FileTokenReadStream::seek(size_t pos)
{
	asserts(pos <= buffer.size()); // allow to position to the eof (== buffer.size())
	// TODO: we should only clear if we seek to the previous location, if we seek to future location then the token info can self validate
	next_token = NextTokenInfo{}; // random repositioning, so the next token must be invalidated
	current = pos;
}

inline bool FileTokenReadStream::test_char_at(size_t pos, char& out_ch)
{
	if (!eof(pos))
	{
		out_ch = char_at(pos);
		return true;
	}
	return false;
}

inline Delimiter FileTokenReadStream::consume_delimiter()
{
	const auto delim = static_cast<Delimiter>(next_token.first);
	current = (next_token.begin + 1); // NOTE: we are skipping past the token position, so the previous token will be auto invalidated
	return delim;
}

inline const char* FileTokenReadStream::consume_number(size_t& end_pos)
{
	auto num_str = reinterpret_cast<const char*>(buffer.get(next_token.begin));
	
	bool expect_decimal = (next_token.first != '.');
	bool expect_Exp = true;
	auto pos = (next_token.begin + 1);
	while (!eof(pos))
	{
		const auto ch = char_at(pos);
		if (expect_decimal && ch == '.')
		{
			expect_decimal = false;
		}
		else if (expect_Exp && (ch == 'e' || ch == 'E'))
		{
			expect_Exp = false;
			expect_decimal = false;
			char next_ch;
			char next_next_ch;
			if (test_char_at(pos+1, next_ch))
			{
				if (std::isdigit(next_ch))
				{
					pos++; // further skip the digit
				}
				else if ((next_ch == '+' || next_ch == '-') && test_char_at(pos+2, next_next_ch) && std::isdigit(next_next_ch))
				{
					pos += 2; // skip the sign and a digit
				}
				else
				{
					// dangling e/E, terminate
					break;
				}
			}
		}
		// TODO: maybe allow underscore?
		else if (!std::isdigit(ch))
		{
			break;
		}
		pos++;
	}
	current = end_pos = pos; // NOTE: we are skipping past the token position, so the previous token will be auto invalidated
	return num_str;
}

inline const char* FileTokenReadStream::consume_string(size_t& len)
{
	auto str = reinterpret_cast<const char*>(buffer.get(next_token.begin));
	auto pos = (next_token.begin + 1);
	while (!eof(pos))
	{
		const auto ch = char_at(pos);
		if (std::isalpha(ch) || ch == '_')
		{
			pos++;
		}
		else
		{
			break;
		}
	}
	len = (pos - next_token.begin);
	current = pos; // NOTE: we are skipping past the token position, so the previous token will be auto invalidated
	return str;
}

inline size_t FileTokenReadStream::next_non_space(bool stop_at_eol)
{
	auto pos = current;
	while (!eof(pos))
	{
		const auto ch = char_at(pos);
		if (!std::isspace(ch) || (ch == newline_char && stop_at_eol))
		{
			break;
		}
		pos++;
	}
	return pos;
}


