#include <coroutine>
#include <optional>
#include <iostream>
#include <fstream>
#include <string>
#include <string_view>
#include <vector>

template <typename T>
struct Enumerable 
{
	struct promise_type 
	{
		T current_value;

		std::suspend_always yield_value(T value)
		{
			current_value = value;
			return {};
		}

		std::suspend_always initial_suspend() { return {}; }
		std::suspend_always final_suspend() noexcept { return {}; }
		void return_void() {}
		void unhandled_exception() { throw; }
		Enumerable get_return_object() { return Enumerable{ std::coroutine_handle<promise_type>::from_promise(*this) }; }
	};

	struct iterator 
	{
		std::coroutine_handle<promise_type> handle;

		bool operator==(std::default_sentinel_t) const { return handle.done(); }
		bool operator!=(std::default_sentinel_t) const { return !handle.done(); }
		iterator& operator++() { handle.resume(); return *this; }
		T operator*() const { return handle.promise().current_value; }
	};

	iterator begin() 
	{
		handle.resume();
		return iterator{ handle };
	}

	std::default_sentinel_t end() { return {}; }
	std::coroutine_handle<promise_type> handle;
};

enum class TokenID
{
	Root,
	Eos,
	Whitespace,
	Label,
	Number,
	Plus,
	Increment,
	PlusEqual,
	Minus,
	Decrement,
	MinusEqual,
	And,
	AndAnd,
	AndEqual,
	Or,
	OrOr,
	OrEqual,
	Xor,
	XorEqual,
	Not,
	NotEqual,
	Assign,
	Equal,
	Less,
	LessEqual,
	Greater,
	GreaterEqual,
	PBrace,
	NBrace,
	PScope,
	NScope,
	PIndex,
	NIndex,
	Dot,
	Comma,
	End
};

class LexStage
{
public:
	const char* const chars;
	const TokenID tokenID;
	const std::vector<std::reference_wrapper<const LexStage>> following;

	LexStage(const char* const chars, 
		const TokenID tokenID,
		std::initializer_list<std::reference_wrapper<const LexStage>> following)
		: chars{ chars }
		, tokenID{ tokenID }
		, following{ following }
	{ }
};

extern const LexStage sRoot;
extern const LexStage sWhitespace;
extern const LexStage sAlpha;
extern const LexStage sAlphaNum;
extern const LexStage sNumerical;
extern const LexStage sPlus;
extern const LexStage sIncrement;
extern const LexStage sPlusEqual;
extern const LexStage sMinus;
extern const LexStage sDecrement;
extern const LexStage sMinusEqual;
extern const LexStage sAnd;
extern const LexStage sAndAnd;
extern const LexStage sAndEqual;
extern const LexStage sOr;
extern const LexStage sOrOr;
extern const LexStage sOrEqual;
extern const LexStage sXor;
extern const LexStage sXorEqual;
extern const LexStage sNot;
extern const LexStage sNotEqual;
extern const LexStage sAssign;
extern const LexStage sEqual;
extern const LexStage sLess;
extern const LexStage sLessEqual;
extern const LexStage sGreater;
extern const LexStage sGreaterEqual;
extern const LexStage sPBrace;
extern const LexStage sNBrace;
extern const LexStage sPScope;
extern const LexStage sNScope;
extern const LexStage sPIndex;
extern const LexStage sNIndex;
extern const LexStage sDot;
extern const LexStage sComma;
extern const LexStage sEnd;

const LexStage sRoot = { "", TokenID::Root, { sWhitespace, sAlpha, sNumerical, sPlus, sMinus, sAnd, sOr, sXor, sNot, sAssign, sLess, sGreater, sPBrace, sNBrace, sPScope, sNScope, sPIndex, sNIndex, sDot, sComma, sEnd } };
const LexStage sWhitespace = { " \t\r\n", TokenID::Whitespace, { sWhitespace } };
const LexStage sAlpha = { "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_", TokenID::Label, { sAlphaNum, sAlpha } };
const LexStage sAlphaNum = { "0123456789", TokenID::Label, { sAlphaNum, sAlpha } };
const LexStage sNumerical = { "0123456789", TokenID::Number, { sNumerical, sAlpha } };
const LexStage sPlus = { "+", TokenID::Plus, { sIncrement } };
const LexStage sIncrement = { "+", TokenID::Increment, {} };
const LexStage sPlusEqual = { "=", TokenID::PlusEqual, {} };
const LexStage sMinus = { "-", TokenID::Minus, { sDecrement } };
const LexStage sDecrement = { "-", TokenID::Decrement, {} };
const LexStage sMinusEqual = { "=", TokenID::MinusEqual, {} };
const LexStage sAnd = { "&", TokenID::And, { sAndAnd, sAndEqual } };
const LexStage sAndAnd = { "&", TokenID::AndAnd, {} };
const LexStage sAndEqual = { "=", TokenID::AndEqual, {} };
const LexStage sOr = { "|", TokenID::Or, { sOrOr, sOrEqual } };
const LexStage sOrOr = { "|", TokenID::OrOr, {} };
const LexStage sOrEqual = { "=", TokenID::OrEqual, {} };
const LexStage sXor = { "^", TokenID::Xor, { sXorEqual } };
const LexStage sXorEqual = { "=", TokenID::XorEqual, {} };
const LexStage sNot = { "!", TokenID::Not, { sNotEqual } };
const LexStage sNotEqual = { "=", TokenID::NotEqual, {} };
const LexStage sAssign = { "=", TokenID::Assign, { sEqual } };
const LexStage sEqual = { "=", TokenID::Equal, {} };
const LexStage sLess = { "<", TokenID::Less, { sLessEqual } };
const LexStage sLessEqual = { "=", TokenID::LessEqual, {} };
const LexStage sGreater = { ">", TokenID::Greater, { sGreaterEqual } };
const LexStage sGreaterEqual = { "=", TokenID::GreaterEqual, {} };
const LexStage sPBrace = { "(", TokenID::PBrace, {} };
const LexStage sNBrace = { ")", TokenID::NBrace, {} };
const LexStage sPScope = { "{", TokenID::PScope, {} };
const LexStage sNScope = { "}", TokenID::NScope, {} };
const LexStage sPIndex = { "[", TokenID::PIndex, {} };
const LexStage sNIndex = { "]", TokenID::NIndex, {} };
const LexStage sDot = { ".", TokenID::Dot, {} };
const LexStage sComma = { ",", TokenID::Comma, {} };
const LexStage sEnd = { ";", TokenID::End, {} };

struct SourceChar
{
	char c;
	int column;
	int line;
	const char* ptr;
};

Enumerable<SourceChar> EnumerateSource(const std::string& source)
{
	int column = 1;
	int line = 1;
	for (const auto& c : source)
	{
		co_yield{ c, column, line, &c };
		switch (c)
		{
		case '\n': column = 1; ++line; break;
		case '\t': column += 3; break;
		default: ++column;
		}
	}
}

Enumerable<SourceChar> EnumerateRemLine(Enumerable<SourceChar> stream)
{
	const auto end = stream.end();
	for (auto itr = stream.begin(); itr != end; ++itr)
	{
		const auto& i = *itr;
		if (i.c == '/')
		{
			++itr;
			if (itr == end)
			{
				co_yield i;
				co_return;
			}
			const auto& j = *itr;
			if (j.c == '/')
			{
				while (true)
				{
					if ((*itr).c == '\n') break;
					++itr;
					if (itr == end) co_return;
				}
				continue;
			}
			else
			{
				co_yield i;
				co_yield j;
				continue;
			}
		}
		co_yield i;
	}
}

Enumerable<SourceChar> EnumerateRemBlock(Enumerable<SourceChar> stream)
{
	enum class State
	{
		None,
		PrimedExit,
		PrimedEntry
	};

	const auto end = stream.end();
	for (auto itr = stream.begin(); itr != end; ++itr)
	{
		const auto& i = *itr;
		if (i.c == '/')
		{
			++itr;
			if (itr == end)
			{
				co_yield i;
				co_return;
			}
			const auto& j = *itr;
			if (j.c == '*')
			{
				State state{ State::None };
				int depth = 1;

				while (true)
				{
					++itr;
					if (itr == end) co_return; // todo : warning, stream ended during comment block

					const auto& j = *itr;
					switch (state)
					{
					case State::None:
						switch (j.c)
						{
						case '/': state = State::PrimedEntry; break;
						case '*': state = State::PrimedExit; break;
						}
						break;

					case State::PrimedExit:
						switch (j.c)
						{
						case '/':
						{
							--depth;
							if (depth == 0) goto end_block;
							state = State::None;
							break;
						}
						case '*': break;
						default: state = State::None; break;
						}
						break;

					case State::PrimedEntry:
						switch (j.c)
						{
						case '*':
						{
							++depth;
							state = State::None;
							break;
						}
						case '/': break;
						default: state = State::None; break;
						}
						break;
					}
				}
				end_block:
				co_yield { ' ', i.column, i.line, i.ptr };
				continue;
			}
		}
		co_yield i;
	}
}

struct SourceToken
{
	TokenID token;
	std::string_view value;
	int column;
	int line;
};

Enumerable<SourceToken> EnumerateToken(Enumerable<SourceChar> stream)
{
	const auto end = stream.end();
	for (auto itr = stream.begin(); itr != end;)
	{
		const auto& i = *itr;
		std::reference_wrapper<const LexStage> stage = sRoot;

		while (itr != end)
		{
			const auto& j = *itr;
			bool found{ false };
			for (auto& next : stage.get().following)
			{
				if (std::strchr(next.get().chars, j.c))
				{
					++itr;
					stage = next;
					found = true;
					break;
				}
			}
			if (found == false) break;
		}
		
		const auto token = stage.get().tokenID;
		if (token == TokenID::Root) throw; // todo : unhandled character
		if (token == TokenID::Whitespace) continue;
		co_yield{ stage.get().tokenID, { i.ptr, std::size_t((*itr).ptr - i.ptr) }, i.column, i.line};
	}
	co_yield{ TokenID::Eos, {}, 0, 0 };
}

template <typename E>
auto EnumChain(E e)
{
	return e;
}

template <typename E, typename ...ENUMS>
auto EnumChain(E e, ENUMS... enums)
{
	return e(EnumChain(enums...));
}

int main()
{
	std::ifstream file("source.txt", std::ios::binary | std::ios::ate);
	if (!file) return 1;
	std::streamsize size = file.tellg();
	file.seekg(0, std::ios::beg);
	std::string source(size, '\0');
	if (!file.read(&source[0], size)) return 1;

	for (const auto& i : EnumChain(EnumerateToken, EnumerateRemBlock, EnumerateRemLine, EnumerateSource(source)))
	{
		std::cout << (int)i.token << ": '" << i.value << "'" << std::endl;
	}
}