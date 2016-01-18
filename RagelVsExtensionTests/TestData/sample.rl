%%{
	machine sample;

	main := |*
		'{' { yield return Emit(TokenType.LCurly); };
		'}' { yield return Emit(TokenType.RCurly); };
		space;
		*|;
}%%

// ReSharper disable All

using System.Collections.Generic;

namespace Sample
{
	public partial class SampleLexer
	{

		%% write data;

		partial void Init()
		{
			%% write init;
		}

		private IEnumerable<Token> GetTokens()
		{
			%% write exec;
		}
	}
}

// ReSharper restore All