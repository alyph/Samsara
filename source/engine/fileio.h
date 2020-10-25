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
};


class FileTokenWriteStream
{
public:
	inline void open(const String& path);
	inline size_t emit_delim(Delimiter del);
	template<typename T>
	inline size_t emit_number(T num);
	inline size_t emit_string(const String& str);
	inline size_t emit_newline();
	inline size_t emit_char(char ch);
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
	inline void open(const String& path);
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
	inline operator bool() const { return file.fp != nullptr; }

private:
	struct NextTokenInfo
	{
		TokenType type{};
		char first{};
	};

	File file;
	size_t current{};
	NextTokenInfo next_token;
	Array<char> read_buffer{0, 1024, Allocator::temp};

	inline void rewind();
	inline Delimiter consume_delimiter();
	inline const char* consume_number();
	inline const char* consume_string(size_t& len);
	inline void do_skip_spaces(bool stop_at_eol);
	inline void reposition();
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
inline typename FileOpSelector<T, TContext>::type make_file_op(const String& path, const TContext& context)
{
	typename FileOpSelector<T, TContext>::type op{};
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


inline void FileTokenWriteStream::open(const String& path)
{
	*this = {};
	file.open(path, FileMode::write);
}

inline size_t FileTokenWriteStream::emit_delim(Delimiter del)
{
	const auto ch = static_cast<char>(del);
	prepare_next_token(TokenType::delimiter, ch);
	file.put(ch);
	return file.tell();
}

template<typename T>
inline size_t FileTokenWriteStream::emit_number(T num)
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
	return file.tell();
}

inline size_t FileTokenWriteStream::emit_string(const String& str)
{
	prepare_next_token(TokenType::string, 0);
	// TODO: space in string etc.
	const auto begin = str.data();
	for (auto i = 0; i < str.size(); i++)
	{
		file.put(*(begin + i));
	}
	return file.tell();
}

inline size_t FileTokenWriteStream::emit_newline()
{
	file.put(newline_char);
	last_token_type = TokenType::none;
	last_token_char = 0;
	return file.tell();
}

inline size_t FileTokenWriteStream::emit_char(char ch)
{
	file.put(ch);
	last_token_type = TokenType::none;
	last_token_char = 0;
	return file.tell();
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


inline void FileTokenReadStream::open(const String& path)
{
	*this = {};
	file.open(path, FileMode::read);
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
		auto num_str = consume_number();
		char* end;
		if constexpr (std::is_integral_v<T>)
		{
			num = static_cast<T>(std::strtoll(num_str, &end, 10));
		}
		else if constexpr (std::is_floating_point_v<T>)
		{
			if constexpr (sizeof(T) <= 4)
			{
				num = static_cast<T>(std::strtof(num_str, &end));
			}
			else
			{
				num = static_cast<T>(std::strtod(num_str, &end));
			}
		}
		else
		{
			static_assert(false, "unsupported number type");
		}
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
			seek(prev_pos);
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
	rewind();
	if (file.get(ch))
	{
		if (ch != newline_char || !stop_at_eol)
		{
			current = file.tell();
			return true;
		}
		file.unget(ch);
	}
	return false;
}

inline size_t FileTokenReadStream::emit_newline()
{
	rewind();
	do_skip_spaces(true);
	char ch;
	if (file.get(ch))
	{
		if (ch != newline_char)
		{
			file.unget(ch);
			asserts(false); // report error
		}
	}
	// silently stop here if at eof
	return (current = file.tell());
}

inline TokenType FileTokenReadStream::peek()
{
	if (next_token.type == TokenType::none)
	{
		do_skip_spaces(false);
		next_token.type = TokenType::eof;
		next_token.first = '\0';
		char ch{};
		while (file.get(ch))
		{
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
				if (file.peek(next_ch) && std::isdigit(next_ch))
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
				if (file.peek(next_ch) && (next_ch == '.' || std::isdigit(next_ch)))
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
	rewind();
	char ch;
	if (file.get(ch))
	{
		file.unget(ch);
		return (ch == newline_char);
	}
	return true;
}

inline bool FileTokenReadStream::seek_next_line()
{
	rewind();
	char ch;
	while (file.get(ch))
	{
		if (ch == newline_char)
		{
			current = file.tell();
			return true;
		}
	}
	return false;
}

inline size_t FileTokenReadStream::skip_spaces(bool stop_at_eol)
{
	rewind();
	do_skip_spaces(stop_at_eol);
	return (current = file.tell());
}

inline size_t FileTokenReadStream::cursor() const
{
	return current;
}

inline void FileTokenReadStream::seek(size_t pos)
{
	file.seek(pos);
	reposition();
}

inline void FileTokenReadStream::rewind()
{
	// Only peek functions can move file pos ahead of the current pos
	// and those will always mark the next token type, if none marked,
	// that means the file pos hasn't been moved, and this should be a noop
	if (next_token.type != TokenType::none)
	{
		file.seek(current);
		reposition();
	}
}

inline Delimiter FileTokenReadStream::consume_delimiter()
{
	const auto delim = static_cast<Delimiter>(next_token.first);
	reposition();
	return delim;
}

inline const char* FileTokenReadStream::consume_number()
{
	read_buffer.clear();
	read_buffer.push_back(next_token.first);

	char ch{};
	bool expect_decimal = (next_token.first != '.');
	bool expect_Exp = true;
	while (file.get(ch))
	{
		if (expect_decimal && ch == '.')
		{
			expect_decimal = false;
			read_buffer.push_back(ch);
		}
		else if (expect_Exp && (ch == 'e' || ch == 'E'))
		{
			expect_Exp = false;
			expect_decimal = false;
			read_buffer.push_back(ch);
			if (file.get(ch) && (ch == '+' || ch == '-'))
			{
				read_buffer.push_back(ch);
			}
			else
			{
				file.unget(ch);
			}
		}
		else if (std::isdigit(ch))
		{
			read_buffer.push_back(ch);
		}
		// TODO: maybe allow underscore?
		else
		{
			file.unget(ch);
			break;
		}
	}
	read_buffer.push_back('\0');
	reposition();
	return read_buffer.data();
}

inline const char* FileTokenReadStream::consume_string(size_t& len)
{
	read_buffer.clear();
	read_buffer.push_back(next_token.first);
	char ch{};
	while (file.get(ch))
	{
		if (std::isalpha(ch) || ch == '_')
		{
			read_buffer.push_back(ch);
		}
		else
		{
			file.unget(ch);
			break;
		}
	}
	read_buffer.push_back('\0');
	len = (read_buffer.size() - 1);
	reposition();
	return read_buffer.data();
}

inline void FileTokenReadStream::do_skip_spaces(bool stop_at_eol)
{
	char ch;
	while (file.get(ch))
	{
		if (!std::isspace(ch) || (ch == newline_char && stop_at_eol))
		{
			file.unget(ch);
			break;
		}
	}
}

inline void FileTokenReadStream::reposition()
{
	current = file.tell();
	next_token = NextTokenInfo{};
}


